#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "freertos/event_groups.h"
#include <stdbool.h>

#define WIFI_CONNECTED_BIT BIT0

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

EventGroupHandle_t get_wifi_event_group(void);

bool is_wifi_connected(void);