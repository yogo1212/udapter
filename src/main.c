#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

#include <evtssl.h>

#include "config.h"
#include "log.h"
#include "udapter_side.h"

static void loopbreak(evutil_socket_t fd, short what, void *ctx)
{
	(void) fd;

	if (what & EV_TIMEOUT)
		log_error("timeout");

	struct event_base *base = ctx;

	event_base_loopbreak(base);
}

typedef struct {
	udapter_side_t a;
	udapter_side_t b;

	ssize_t timeout;
	struct event *timeout_evt;
	struct event *signal_evt;
} udapter_t;

static void refresh_timeout(udapter_t *u)
{
	struct timeval tv = { u->timeout + 1, 0 };
	event_add(u->timeout_evt, &tv);
}

static void bev_event(struct bufferevent *bev, short what, void *ctx)
{
	udapter_t *u = ctx;
	evt_ssl_t *essl = u->a.essl;
	if (u->b.bev == bev) {
		essl = u->b.essl;
	}
	log_error("fw bev %x %s:%u", what, evt_ssl_get_hostname(essl), evt_ssl_get_port(essl));
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) {
		bufferevent_disable(u->a.bev, EV_READ);
		bufferevent_disable(u->b.bev, EV_READ);
		// TODO if the source was closed, forward the rest to dest
		struct timeval tv = { 1, 0 };
		event_add(u->timeout_evt, &tv);
	}
}

static void shovel(struct bufferevent *from, struct bufferevent *to)
{
	if (bufferevent_write_buffer(to, bufferevent_get_input(from)) == -1) {
		log_error("tcp fw write error");
	}
}

static void read_a(struct bufferevent *bev, void *ctx)
{
	udapter_t *u = ctx;
	shovel(bev, u->b.bev);
}

static void read_b(struct bufferevent *bev, void *ctx)
{
	udapter_t *u = ctx;
	shovel(bev, u->a.bev);
}

static void connect_both(udapter_t *u)
{
	refresh_timeout(u);

  bufferevent_setcb(u->a.bev, read_a, NULL, bev_event, u);
  bufferevent_enable(u->a.bev, EV_READ);

  bufferevent_setcb(u->b.bev, read_b, NULL, bev_event, u);
  bufferevent_enable(u->b.bev, EV_READ);
}

static void a_ready(void *ctx)
{
	udapter_t *u = ctx;

	if (!u->b.bev)
		return;

	connect_both(u);
}

static void b_ready(void *ctx)
{
	udapter_t *u = ctx;

	if (!u->a.bev)
		return;

	connect_both(u);
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	struct event_base *base = event_base_new();

	udapter_t u;
	memset(&u, 0, sizeof(u));
	u.timeout = config_timeout();

#define init_side(n) \
	u.n.host = config_host_##n();\
	u.n.port = config_port_##n();\
	u.n.no_ssl = config_no_ssl_##n();\
	u.n.ssl_key = config_ssl_key_##n();\
	u.n.ssl_cert = config_ssl_cert_##n();\
	u.n.ssl_ca_file = config_ssl_ca_file_##n();\
	u.n.ssl_ca_dir = config_ssl_ca_dir_##n();\
	u.n.ssl_verify_depth = config_ssl_verify_depth_##n();\
	u.n.ready_cb = n##_ready;\
	u.n.ctx = &u;

	init_side(a)
	init_side(b)

	if (!udapter_side_start(base, &u.a))
		goto cleanup_base;
	if (!udapter_side_start(base, &u.b))
		goto cleanup_side_a;

	u.signal_evt = evsignal_new(base, SIGINT, loopbreak, base);
	event_add(u.signal_evt, NULL);

	u.timeout_evt = event_new(base, -1, EV_TIMEOUT, loopbreak, base);
	refresh_timeout(&u);

	event_base_dispatch(base);

	event_free(u.timeout_evt);
	event_free(u.signal_evt);

	udapter_side_stop(&u.b);

cleanup_side_a:
	udapter_side_stop(&u.a);

cleanup_base:
	event_base_free(base);

	return 0;
}
