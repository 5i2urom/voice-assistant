#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <string.h>
#include "wifi-connect.h"   

void ble_app_advertise(void);
void ble_app_on_sync(void);
void host_task(void *param);

extern const struct ble_gatt_svc_def gatt_svcs[];