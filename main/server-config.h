#pragma once

#include "esp_err.h"
#include "esp_netif.h"
#include <stdbool.h>

#define SERVER_IP_KEY "server_ip"

esp_err_t get_server_ip(char *ip_str, size_t len);
esp_err_t set_server_ip(const char *ip_str);
esp_err_t update_server_ip(const char *new_ip_str);
bool is_server_ip_set(void);
esp_err_t get_server_ip_addr(esp_ip4_addr_t *addr);
