#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <event2/buffer.h>
#include <event2/util.h>

#include <evtssl.h>

#include "log.h"

#include "udapter_side.h"

// TODO remove
#include <arpa/inet.h>

static void accept_cb(evt_ssl_t *essl, struct bufferevent *bev, struct sockaddr *addr, int addrlen)
{
	(void) addr;
	(void) addrlen;

	// TODO remove
	struct sockaddr_in6 *sin = (struct sockaddr_in6 *) addr;
	char str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &(sin->sin6_addr), str, INET6_ADDRSTRLEN);
	log_info("accepted connection (%d) from %s\n", sin->sin6_family, str);

	udapter_side_t *s = evt_ssl_get_ctx(essl);

	s->bev = bev;
	s->ready_cb(s->ctx);
}

static const char *udapter_side_ssl_config(evt_ssl_t *essl, SSL_CTX *ssl_ctx)
{
	udapter_side_t *side = evt_ssl_get_ctx(essl);

	if (side->ssl_ca_file || side->ssl_ca_dir) {
		if (SSL_CTX_load_verify_locations(ssl_ctx, side->ssl_ca_file, side->ssl_ca_dir) != 1)
			return "couldn't set CA";

		SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
		SSL_CTX_set_verify_depth(ssl_ctx, side->ssl_verify_depth);
	}

	if (SSL_CTX_use_certificate_file(ssl_ctx, side->ssl_cert, SSL_FILETYPE_PEM) != 1)
		return "couldn't set certificate!";

	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, side->ssl_key, SSL_FILETYPE_PEM) != 1)
		return "couldn't set private key!";

	if (SSL_CTX_check_private_key(ssl_ctx) != 1)
		return "invalid private key!";

	return NULL;
}

bool udapter_side_start(struct event_base *base, udapter_side_t *side)
{
	// TODO errorcb
	side->essl = evt_ssl_create(base, side->host, side->port, side, side->no_ssl ? NULL : udapter_side_ssl_config, NULL);
	if (!side->essl) {
		log_error("!evt_ssl_create");
		goto leave;
	}

	if (side->no_ssl)
		evt_ssl_dont_really_ssl(side->essl);

	if (side->host) {
		side->bev = evt_ssl_connect(side->essl);
		if (!side->bev) {
			log_error("!bev");
			goto cleanup_essl;
		}

		side->ready_cb(side->ctx);
	}
	else {
		if(evt_ssl_listen(side->essl, accept_cb) == -1) {
			log_error("_listen == -1");
			goto cleanup_essl;
		}
	}

	return true;

cleanup_essl:
	evt_ssl_free(side->essl);

leave:
	log_error("ouch %s %u", side->host, side->port);

	return false;
}

void udapter_side_stop(udapter_side_t *side)
{
	bufferevent_free(side->bev);
	evt_ssl_free(side->essl);
}
