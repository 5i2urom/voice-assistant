#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define SERVER_IP_KEY "server_ip"

// Инициализация Wi-Fi
void init_wifi(void);

// Подключение к Wi-Fi
esp_err_t connect_to_wifi(); 

// Получение текущих данных для Wi-Fi (SSID и пароль)
esp_err_t get_wifi_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len);

// Установка новых данных для Wi-Fi (SSID и пароль)
void set_wifi_credentials(const char *ssid, const char *password);

// Обновление данных для Wi-Fi, если они изменились
void update_wifi_credentials(const char *new_ssid, const char *new_password);

// Получение IP-адреса сервера из NVS
esp_err_t get_server_ip(char *ip_address, size_t len);

// Установка IP-адреса сервера в NVS
void set_server_ip(const char *ip_address);

// Обновление IP-адреса сервера в NVS, если он изменился
esp_err_t update_server_ip(const char *new_ip_address);
