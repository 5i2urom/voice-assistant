#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "network-monitor.h"
#include "server-config.h"
#include "wifi-connect.h"
#include "esp_log.h"
#include "ping/ping_sock.h"
#include "esp_netif.h"
#include <string.h>
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

static const char *TAG = "network-monitor";
static EventGroupHandle_t network_event_group;
#define SERVER_REACHABLE_BIT BIT0

static TimerHandle_t ping_timer = NULL;

static inline void set_server_reachable() {
    xEventGroupSetBits(network_event_group, SERVER_REACHABLE_BIT);
}

static inline void set_server_unreachable() {
    xEventGroupClearBits(network_event_group, SERVER_REACHABLE_BIT);
}

bool is_server_reachable() {
    return xEventGroupGetBits(network_event_group) & SERVER_REACHABLE_BIT;
}

static void ping_callback(esp_ping_handle_t hdl, void *args) {
    uint32_t transmitted = 0, received = 0;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));

    if (received > 0) {
        set_server_reachable();
        ESP_LOGI(TAG, "Ping successful");
    } else {
        set_server_unreachable();
        ESP_LOGW(TAG, "Ping failed");   
    }

    esp_ping_stop(hdl);
    esp_ping_delete_session(hdl);
}

static void start_ping() {
    if (!is_wifi_connected() || !is_server_ip_set()) {
        set_server_unreachable();
        return;
    }

    esp_ip4_addr_t ip4;
    if (get_server_ip_addr(&ip4) != ESP_OK) {
        set_server_unreachable();
        return;
    }

    ip_addr_t target_addr;
    target_addr.type = IPADDR_TYPE_V4;
    target_addr.u_addr.ip4.addr = ip4.addr;

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;
    ping_config.count = 3;

    esp_ping_callbacks_t cbs = {
        .on_ping_end = ping_callback,
        .cb_args = NULL
    };

    esp_ping_handle_t ping;
    esp_err_t err = esp_ping_new_session(&ping_config, &cbs, &ping);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ping session: %s", esp_err_to_name(err));
        set_server_unreachable();
        return;
    }

    err = esp_ping_start(ping);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start ping: %s", esp_err_to_name(err));
        set_server_unreachable();
        esp_ping_delete_session(ping);
    }
}

static void ping_timer_callback(TimerHandle_t xTimer) {
    start_ping_timer();
}

void trigger_ping_now() {
    stop_ping_timer();
    start_ping_timer();
}

void start_network_monitor() {
    xEventGroupWaitBits(get_wifi_event_group(), WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    network_event_group = xEventGroupCreate();
    ping_timer = xTimerCreate("ping_timer", pdMS_TO_TICKS(30000), pdFALSE, NULL, ping_timer_callback);
    if (ping_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create timer");
        return;
    }

    start_ping_timer();
}

void start_ping_timer() {
    if (ping_timer != NULL) {
        start_ping();
        xTimerStart(ping_timer, 0);
    }
}

void stop_ping_timer() {
    if (ping_timer != NULL) {
        xTimerStop(ping_timer, 0);
    }
}
