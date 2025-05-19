#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include <string.h>
#include "server-config.h"
#include "network-monitor.h"
#include "esp_log.h"

static const char *TAG = "server-config";

static char cached_ip[16] = {0}; 
static bool ip_cached = false;  

esp_err_t get_server_ip(char *ip_str, size_t len) {
    if (ip_cached) {
        strncpy(ip_str, cached_ip, len);
        return ESP_OK;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_str(nvs_handle, SERVER_IP_KEY, ip_str, &len);
    if (err == ESP_OK) {
        strncpy(cached_ip, ip_str, sizeof(cached_ip));
        ip_cached = true;
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t set_server_ip(const char *ip_str) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_str(nvs_handle, SERVER_IP_KEY, ip_str);
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
        if (err == ESP_OK) {
            strncpy(cached_ip, ip_str, sizeof(cached_ip));
            ip_cached = true;
        }
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t update_server_ip(const char *new_ip_str) {
    char current_ip[16] = {0};
    size_t len = sizeof(current_ip);
    esp_err_t err = get_server_ip(current_ip, len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        return err;
    }

    if (strcmp(current_ip, new_ip_str) == 0) {
        ESP_LOGI(TAG, "IP address is unchanged");
        return ESP_OK;
    }

    err = set_server_ip(new_ip_str);
    if (err != ESP_OK) {
        return err;
    }

    trigger_ping_now();

    return ESP_OK;
}

bool is_server_ip_set(void) {
    if (ip_cached) {
        return true;
    }

    char ip[16] = {0};
    if (get_server_ip(ip, sizeof(ip)) == ESP_OK) {
        return true;
    } else {
        return false;
    }
}

esp_err_t get_server_ip_addr(esp_ip4_addr_t *addr) {
    char ip_str[16] = {0};
    esp_err_t err = get_server_ip(ip_str, sizeof(ip_str));
    if (err != ESP_OK) {
        return err;
    }

    return esp_netif_str_to_ip4(ip_str, addr);
}
