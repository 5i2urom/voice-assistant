#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "dht.h"
#include "i2c-lcd.h"
#include "ble-connect.h"
#include <esp_wifi.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "wifi-connect.h"    
#include "network-monitor.h"
#include "wake_word_detector.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_22     
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      
#define I2C_MASTER_NUM              0                    
#define I2C_MASTER_FREQ_HZ          400000                    
#define I2C_MASTER_TX_BUF_DISABLE   0                         
#define I2C_MASTER_RX_BUF_DISABLE   0                          
#define I2C_MASTER_TIMEOUT_MS       1000
#define SENSOR_TYPE                 DHT_TYPE_DHT11
#define DHT_GPIO                    GPIO_NUM_4

int16_t temperature = 0;
int16_t humidity = 0;

extern char ip_str[16];
static char empty_str[13];

static esp_err_t i2c_master_init(void) {
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static SemaphoreHandle_t lcd_mutex = NULL;

void put_led_string(char line, char row, char* buffer) {
    if (xSemaphoreTake(lcd_mutex, portMAX_DELAY)) { 
        lcd_set_cursor(line, row);
        lcd_send_string(empty_str);
        lcd_set_cursor(line, row);
        lcd_send_string(buffer);
        xSemaphoreGive(lcd_mutex);
    }
}

void dht_task(void *pvParameters) {
    char buffer[13];

    gpio_set_direction(DHT_GPIO, GPIO_MODE_OUTPUT_OD); // открытый сток (open drain)
    gpio_set_level(DHT_GPIO, 1);

    do {
        vTaskDelay(pdMS_TO_TICKS(1000)); // задержка 1с
        if (dht_read_data(SENSOR_TYPE, DHT_GPIO, &humidity, &temperature) == ESP_OK) {
            temperature /= 10;
            humidity /= 10;

            sprintf(buffer, "%d C", temperature);
            put_led_string(0, 8, buffer);

            sprintf(buffer, "%d %%", humidity);
            put_led_string(1, 8, buffer);
        }        
    } while (1);
}

void app_main(void) {    
    nvs_flash_init();
    // lcd_mutex = xSemaphoreCreateMutex();

    // i2c_master_init();
    // lcd_init();
    // lcd_clear();

    // char buffer[21];
    // memset(empty_str, ' ', 12);
    // empty_str[12] = '\0';

    // sprintf(buffer, "Temp  :");
    // put_led_string(0, 0, buffer);

    // sprintf(buffer, "Humi  :");
    // put_led_string(1, 0, buffer);

    // xTaskCreate(dht_task, "DHT Task", 4096, NULL, 3, NULL);

    init_ble();
    init_wifi();

    connect_to_wifi();

    start_network_monitor();    

    start_wake_word_task();

}
