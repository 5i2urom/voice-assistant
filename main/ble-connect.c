#include "ble-connect.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"

uint8_t ble_addr_type;
void ble_app_advertise(void);

extern char ip_str[16];

static const char *TAG = "wifi-ble";

static int device_write(uint16_t conn_handle, uint16_t attr_handle, 
                      struct ble_gatt_access_ctxt *ctxt, void *arg) {
    // Максимальные длины согласно вашей реализации
    #define MAX_SSID_LEN 32
    #define MAX_PASS_LEN 64
    
    struct os_mbuf *om = ctxt->om;
    char buf[MAX_SSID_LEN + MAX_PASS_LEN + 1]; // +1 для нуль-терминатора
    char *ssid = NULL;
    char *password = NULL;

    // 1. Проверяем длину данных
    if (om->om_len < 5 || om->om_len > sizeof(buf)-1) {
        return -1;
    }

    // 2. Копируем данные с нуль-терминацией
    memcpy(buf, om->om_data, om->om_len);
    buf[om->om_len] = '\0';

    // 3. Проверяем префикс
    if (strncmp(buf, "wifi ", 5) != 0) {
        return -1;
    }

    // 4. Парсим данные
    char *ptr = buf + 5;
    char *colon = strchr(ptr, ':');
    
    // 5. Проверяем наличие разделителя
    if (!colon) {
        ESP_LOGE(TAG, "No ':' delimiter found");
        return -1;
    }

    // 6. Проверяем длины
    size_t ssid_len = colon - ptr;
    size_t pass_len = strlen(colon + 1);
    
    if (ssid_len >= MAX_SSID_LEN || pass_len >= MAX_PASS_LEN) {
        return -1;
    }

    // 7. Разделяем строку
    *colon = '\0';
    ssid = ptr;
    password = colon + 1;

    update_wifi_credentials(ssid, password);
    return 0;
}

static int device_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (strlen(ip_str) > 0) {
        os_mbuf_append(ctxt->om, ip_str, strlen(ip_str));
    } else {
        os_mbuf_append(ctxt->om, "No IP", 5);
    }
    return 0;
}

const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status != 0) ble_app_advertise();
            break;
        case (BLE_GAP_EVENT_DISCONNECT || BLE_GAP_EVENT_ADV_COMPLETE):
            ble_app_advertise();
            break;
        default:
            break;
    }
    return 0;
}

void ble_app_advertise(void) {
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen((char *)fields.name);
    fields.name_is_complete = 1; // имя устройства передаётся полностью
    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // undirected, готово принимать соединения от любых клиентов BLE
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // General Discoverable Mode, доступно всеми для обнаружения 
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL); // публичный или случайный адрес
    // ble_gap_adv_start(own_addr_type, direct_addr, duration, adv_params, cb, cb_arg);
}

void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, &ble_addr_type); // 0 - публичный или случайный BLE-адрес, в Bluetooth Low Energy Host Stack
    ble_app_advertise();
}

void host_task(void *param) {
    nimble_port_run(); // запуск BLE стека
}
