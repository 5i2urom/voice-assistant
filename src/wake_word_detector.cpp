#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include "DetectWakeWordState.h"
#include "I2SMicSampler.h"
#include "wake_word_detector.h"
#include "config.h"

static DetectWakeWordState *wake_word_state = nullptr;
static I2SMicSampler *i2s_sampler = nullptr;

static i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

static i2s_pin_config_t i2s_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA
};

static void wake_word_task(void *param)
{
    while (true)
    {
        if (wake_word_state->run())
        {
            printf("Wake word detected!\n");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void start_wake_word_task()
{
    i2s_sampler = new I2SMicSampler(i2s_pins, false);
    i2s_sampler->start(I2S_NUM_0, i2s_config, nullptr);

    wake_word_state = new DetectWakeWordState(i2s_sampler);
    wake_word_state->enterState();

    xTaskCreate(wake_word_task, "wake_word_task", 8192, nullptr, 5, nullptr);
}
