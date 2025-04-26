#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include "wifi_server_config.h"

char ip_str[16] = {0};
static bool reconnect_required = false;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (reconnect_required) {
            reconnect_required = false;
            connect_to_wifi();
        } else {
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void init_wifi() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

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

    err = nvs_get_str(nvs_handle, WIFI_SSID_KEY, ssid, &ssid_len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_get_str(nvs_handle, WIFI_PASS_KEY, password, &password_len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
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

    if (err != ESP_OK) {
        return err;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    return esp_wifi_connect();
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
        return;
    }

    set_wifi_credentials(new_ssid, new_password);
    reconnect_required = true;
    esp_wifi_disconnect();
}

esp_err_t get_server_ip(char *ip_address, size_t len) {
    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READONLY, &nvs_handle);
    size_t ip_len = len;

    esp_err_t err = nvs_get_str(nvs_handle, SERVER_IP_KEY, ip_address, &ip_len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

void set_server_ip(const char *ip_address) {
    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READWRITE, &nvs_handle);
    nvs_set_str(nvs_handle, SERVER_IP_KEY, ip_address);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

esp_err_t update_server_ip(const char *new_ip_address) {
    char current_ip[16] = {0};
    size_t ip_len = 16;

    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READONLY, &nvs_handle);
    nvs_get_str(nvs_handle, SERVER_IP_KEY, current_ip, &ip_len);
    nvs_close(nvs_handle);

    
    if (strcmp(new_ip_address, current_ip) == 0) {
        return;
    }

    set_server_ip(new_ip_address);
    return ESP_OK;
}
