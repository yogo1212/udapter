#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#include "config.h"

uint16_t to_uint(const char *str)
{
	errno = 0;
	uint16_t u = strtoul(str, NULL, 0);
	if (errno != 0) {
		log_error("strtoul port(%s): %s\n", str, strerror(errno));
	}

	return u;
}

int config_timeout(void)
{
	const char *str = getenv("TIMEOUT");
	if (!str) {
		goto def;
	}
	errno = 0;
	int res = strtol(str, NULL, 0);
	if (errno != 0) {
		log_error("timeout conversion: %s", strerror(errno));
		goto def;
	}
	return res;
def:
	return 300;
}

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)

#define _meow(n, N) \
uint16_t config_port_##n(void)\
{\
	const char *str = getenv(N "_PORT");\
	if (!str) {\
		log_info("no " N "_PORT - assuming 0");\
		return 0;\
	}\
	return to_uint(str);\
}\
\
const char *config_host_##n(void)\
{\
	return getenv(N "_HOST");\
}\
\
bool config_no_ssl_##n(void)\
{\
	return !!getenv(N "_NO_SSL");\
}\
\
const char *config_ssl_key_##n(void)\
{\
	return getenv(N "_SSL_KEY");\
}\
\
const char *config_ssl_cert_##n(void)\
{\
	return getenv(N "_SSL_CERT");\
}\
\
const char *config_ssl_ca_file_##n(void)\
{\
	return getenv(N "_SSL_CA_FILE");\
}\
\
const char *config_ssl_ca_dir_##n(void)\
{\
	return getenv(N "_SSL_CA_DIR");\
}\
\
uint8_t config_ssl_verify_depth_##n(void)\
{\
	const char *str = getenv(N "_SSL_VERIFY_DEPTH");\
	if (!str) {\
		log_error("assuming " N "_SSL_VERIFY_DEPTH = 2");\
		return 2;\
	}\
	return to_uint(str);\
}

#define meow(n,N) _meow(n, STRINGIFY(N))

meow(a,A)
meow(b,B)
