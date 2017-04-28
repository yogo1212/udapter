#ifndef UDAPTER_H
#define UDAPTER_H

#include <event2/event.h>

#include <evtssl.h>

struct udapter;
typedef struct udapter udapter_t;

udaptor_t *udaptor_new(struct event_base *base, uint16_t port, evt_ssl_ssl_ctx_config_cb_t ssl_config);
void udaptor_free(udaptor_t *u);

#endif
