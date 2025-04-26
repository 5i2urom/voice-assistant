#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include "server-connect.h"

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
