#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include <stdio.h>

#define SAMPLE_RATE     16000
#define I2S_BCK_IO      26
#define I2S_WS_IO       22
#define I2S_DI_IO       21

void app_main(void)
{
    i2s_chan_handle_t rx_handle;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t err = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    if (err != ESP_OK) {
        printf("Failed to create I2S channel: %s\n", esp_err_to_name(err));
        return;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    err = i2s_channel_init_std_mode(rx_handle, &std_cfg);
    if (err != ESP_OK) {
        printf("Failed to init I2S STD mode: %s\n", esp_err_to_name(err));
        return;
    }

    err = i2s_channel_enable(rx_handle);
    if (err != ESP_OK) {
        printf("Failed to enable I2S channel: %s\n", esp_err_to_name(err));
        return;
    }

    int32_t *i2s_data = heap_caps_malloc(1024 * sizeof(int32_t), MALLOC_CAP_DMA);
    if (!i2s_data) {
        printf("Failed to allocate DMA buffer\n");
        return;
    }

    size_t bytes_read;

    while (1) {
        err = i2s_channel_read(rx_handle, i2s_data, 1024 * sizeof(int32_t), &bytes_read, portMAX_DELAY);
        if (err != ESP_OK) {
            printf("Read error: %s\n", esp_err_to_name(err));
            continue;
        }

        for (int i = 0; i < bytes_read / sizeof(int32_t); i++) {
            printf("%ld ", i2s_data[i]);
        }
        printf("\n");

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// #include "driver/i2s.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include <stdio.h>

// #define SAMPLE_RATE     16000
// #define I2S_BCK_IO      26
// #define I2S_WS_IO       22
// #define I2S_DI_IO       21

// void app_main(void)
// {
//     i2s_config_t i2s_config = {
//         .mode = I2S_MODE_MASTER | I2S_MODE_RX,
//         .sample_rate = SAMPLE_RATE,
//         .bits_per_sample = 32,
//         .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
//         .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
//         .intr_alloc_flags = 0,
//         .dma_buf_count = 8,
//         .dma_buf_len = 1024,
//         .use_apll = false,
//         .tx_desc_auto_clear = false,
//         .fixed_mclk = 0
//     };

//     i2s_pin_config_t pin_config = {
//         .bck_io_num = I2S_BCK_IO,
//         .ws_io_num = I2S_WS_IO,
//         .data_out_num = I2S_PIN_NO_CHANGE,
//         .data_in_num = I2S_DI_IO
//     };

//     esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//     if (err != ESP_OK) {
//         printf("Failed to install i2s driver: %s\n", esp_err_to_name(err));
//         return;
//     }

//     err = i2s_set_pin(I2S_NUM_0, &pin_config);
//     if (err != ESP_OK) {
//         printf("Failed to set i2s pins: %s\n", esp_err_to_name(err));
//         return;
//     }

//     int32_t *i2s_data = malloc(1024 * sizeof(int32_t));
//     if (!i2s_data) {
//         printf("Failed to allocate buffer\n");
//         return;
//     }

//     size_t bytes_read;

//     while (1) {
//         err = i2s_read(I2S_NUM_0, (void*)i2s_data, 1024 * sizeof(int32_t), &bytes_read, portMAX_DELAY);
//         if (err != ESP_OK) {
//             printf("Read error: %s\n", esp_err_to_name(err));
//             continue;
//         }

//         int samples = bytes_read / sizeof(int32_t);
//         for (int i = 0; i < samples; i++) {
//             printf("DATA: %ld\n", i2s_data[i]);
//         }

//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }



