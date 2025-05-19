#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include "wifi-connect.h"
#include "esp_log.h"
#include "network-monitor.h"

#define MAX_RETRY_COUNT 5
#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define SERVER_IP_KEY "server_ip"

static int retry_count = 0;
static bool wifi_updating = false;
static SemaphoreHandle_t wifi_connect_mutex = NULL;
static TimerHandle_t reconnect_timer = NULL;
static EventGroupHandle_t wifi_event_group = NULL;

static const char *TAG = "wifi-connect";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "Disconnected, reason: %d", event->reason);

        if (wifi_updating) {
            ESP_LOGI(TAG, "Update in progress, ignoring disconnection");
            return;
        }

        stop_ping_timer();

        xSemaphoreTake(wifi_connect_mutex, portMAX_DELAY);

        if (retry_count < MAX_RETRY_COUNT) {
            ESP_LOGI(TAG, "Attempting to reconnect (%d/%d)...", retry_count + 1, MAX_RETRY_COUNT);
            esp_wifi_connect();
            ++retry_count;
        } else {
            xTimerStart(reconnect_timer, 0);
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGW(TAG, "Max retry count reached. Reconnecting in 60 seconds...");
        }

        xSemaphoreGive(wifi_connect_mutex);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

        start_ping_timer();
    }
}

static void reconnect_callback(TimerHandle_t xTimer) {
    xSemaphoreTake(wifi_connect_mutex, portMAX_DELAY);
    retry_count = 0;
    esp_wifi_connect();
    xSemaphoreGive(wifi_connect_mutex);
}

void init_wifi() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_event_group = xEventGroupCreate();
    wifi_connect_mutex = xSemaphoreCreateMutex();
    reconnect_timer = xTimerCreate(
        "wifi_connect_timer",
        pdMS_TO_TICKS(30000),
        pdFALSE,
        NULL,
        reconnect_callback
    );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);
}

esp_err_t get_wifi_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return err;

    err = nvs_get_str(nvs_handle, WIFI_SSID_KEY, ssid, &ssid_len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_get_str(nvs_handle, WIFI_PASS_KEY, password, &password_len);
    nvs_close(nvs_handle);
    return err;
}

void set_wifi_credentials(const char *ssid, const char *password) {
    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READWRITE, &nvs_handle);
    nvs_set_str(nvs_handle, WIFI_SSID_KEY, ssid);
    nvs_set_str(nvs_handle, WIFI_PASS_KEY, password);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

esp_err_t connect_to_wifi() {
    char ssid[32] = {0};
    char password[64] = {0};
    size_t ssid_len = sizeof(ssid);
    size_t password_len = sizeof(password);

    esp_err_t err = get_wifi_credentials(ssid, ssid_len, password, password_len);
    if (err != ESP_OK) return err;

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    xSemaphoreTake(wifi_connect_mutex, portMAX_DELAY);

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
    err = esp_wifi_connect();
    retry_count = 0;

    xSemaphoreGive(wifi_connect_mutex);
    return err;
}

void update_wifi_credentials(const char *new_ssid, const char *new_password) {
    char current_ssid[32] = {0};
    char current_password[64] = {0};
    size_t ssid_len = sizeof(current_ssid);
    size_t password_len = sizeof(current_password);

    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READONLY, &nvs_handle);
    nvs_get_str(nvs_handle, WIFI_SSID_KEY, current_ssid, &ssid_len);
    nvs_get_str(nvs_handle, WIFI_PASS_KEY, current_password, &password_len);
    nvs_close(nvs_handle);

    if (strcmp(new_ssid, current_ssid) == 0 && strcmp(new_password, current_password) == 0) {
        ESP_LOGI(TAG, "New credentials are the same as current, no update needed");
        return;
    }

    set_wifi_credentials(new_ssid, new_password);

    xSemaphoreTake(wifi_connect_mutex, portMAX_DELAY);
    xTimerStop(reconnect_timer, 0);
    retry_count = 0;
    wifi_updating = true;
    xSemaphoreGive(wifi_connect_mutex);

    esp_wifi_disconnect();

    vTaskDelay(pdMS_TO_TICKS(500));

    connect_to_wifi();

    xSemaphoreTake(wifi_connect_mutex, portMAX_DELAY);
    wifi_updating = false;
    xSemaphoreGive(wifi_connect_mutex);
}

EventGroupHandle_t get_wifi_event_group(void) {
    return wifi_event_group;
}

bool is_wifi_connected(void) {
    EventBits_t bits = xEventGroupGetBits(wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}