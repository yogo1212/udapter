#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

int config_timeout(void);

#define config_functions(n) \
uint16_t config_port_##n(void);\
const char *config_host_##n(void);\
bool config_no_ssl_##n(void);\
const char *config_ssl_key_##n(void);\
const char *config_ssl_cert_##n(void);\
const char *config_ssl_ca_file_##n(void);\
const char *config_ssl_ca_dir_##n(void);\
uint8_t config_ssl_verify_depth_##n(void);

config_functions(a)
config_functions(b)

#endif
