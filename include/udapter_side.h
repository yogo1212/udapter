#ifndef UDAPTER_SIDE_H
#define UDAPTER_SIDE_H

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <evtssl.h>

typedef void (*updapter_ready_cb_t)(void *);
typedef struct {
	struct bufferevent *bev;
	evt_ssl_t *essl;

	const char *host;
	uint16_t port;
	bool no_ssl;
	const char *ssl_key;
	const char *ssl_cert;
	const char *ssl_ca_file;
	const char *ssl_ca_dir;
	uint8_t ssl_verify_depth;
	updapter_ready_cb_t ready_cb;
	void *ctx;
} udapter_side_t;

bool udapter_side_start(struct event_base *base, udapter_side_t *side);
void udapter_side_stop(udapter_side_t *side);

#endif