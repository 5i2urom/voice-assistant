#include <stdio.h>
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_err.h"

// Конфигурация пинов I2S
#define I2S_SD  34
#define I2S_WS  25
#define I2S_SCK 26

// Настройки I2S
#define I2S_PORT          I2S_NUM_0
#define SAMPLE_RATE       16000
#define SAMPLE_BITS       32
#define BUFFER_SIZE       512
#define DMA_BUF_COUNT     6
#define DMA_BUF_LEN       512

static const char *TAG = "MIC_TEST";

void app_main(void)
{
    // Инициализация последовательного порта
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting microphone test...");

    // Конфигурация I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = DMA_BUF_COUNT,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD
    };

    // Установка I2S драйвера
    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S driver install failed: %s", esp_err_to_name(err));
        return;
    }

    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S set pin failed: %s", esp_err_to_name(err));
        i2s_driver_uninstall(I2S_PORT);
        return;
    }

    ESP_LOGI(TAG, "I2S initialized successfully");

    int32_t *pcm_data = malloc(BUFFER_SIZE * sizeof(int32_t));
    if (pcm_data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        return;
    }

    while (1) {
        size_t bytes_read = 0;
        err = i2s_read(I2S_PORT, pcm_data, BUFFER_SIZE * sizeof(int32_t), &bytes_read, portMAX_DELAY);
        
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "I2S read error: %s", esp_err_to_name(err));
            continue;
        }

        int samples_read = bytes_read / sizeof(int32_t);
        int32_t min = INT32_MAX;
        int32_t max = INT32_MIN;
        int64_t sum = 0;

        for (int i = 0; i < samples_read; i++) {
            int32_t sample = pcm_data[i] >> 16; // Конвертация 32bit -> 16bit
            if (sample < min) min = sample;
            if (sample > max) max = sample;
            sum += sample;
        }

        int avg = (int)(sum / samples_read);
        int amplitude = max - min;

        // Вывод статистики
        printf("Samples: %d | Min: %6ld | Max: %6ld | Avg: %6d | Amp: %6d | ", 
            samples_read, min, max, avg, amplitude);       

        // Визуализация амплитуды
        int bars = amplitude / 100;
        for (int i = 0; i < bars && i < 50; i++) {
            printf("#");
        }
        printf("\n");

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    free(pcm_data);
    i2s_driver_uninstall(I2S_PORT);
}