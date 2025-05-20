#include "I2SSampler.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"
#include "RingBuffer.h"
#include "DetectWakeWordState.h"
#include "esp_log.h"
#include "esp_timer.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

static const char *TAG = "DetectWakeWord";

DetectWakeWordState::DetectWakeWordState(I2SSampler *sample_provider)
{
    m_sample_provider = sample_provider;
    m_average_detect_time = 0;
    m_number_of_runs = 0;
}

void DetectWakeWordState::enterState()
{
    m_nn = new NeuralNetwork();
    ESP_LOGI(TAG, "Created Neural Network");
    m_audio_processor = new AudioProcessor(AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);
    ESP_LOGI(TAG, "Created Audio Processor");
    m_number_of_detections = 0;
}

bool DetectWakeWordState::run()
{
    int64_t start = esp_timer_get_time();
    RingBufferAccessor *reader = m_sample_provider->getRingBufferReader();
    reader->rewind(16000);

    float *input_buffer = m_nn->getInputBuffer();
    
    m_audio_processor->get_spectrogram(reader, input_buffer);

    // for (int i = 0; i < 10; i++) {
    //     ESP_LOGI(TAG, "Input buffer[%d]: %.6f", i, input_buffer[i]);
    // }

    delete reader;

    float output = m_nn->predict();
    ESP_LOGI(TAG, "Output: %.4f", output);

    int64_t end = esp_timer_get_time();

    float detect_time_ms = (end - start) / 1000.0f;
    m_average_detect_time = detect_time_ms * 0.1f + m_average_detect_time * 0.9f;
    m_number_of_runs++;

    if (m_number_of_runs == 100)
    {
        m_number_of_runs = 0;
        ESP_LOGI(TAG, "Average detection time %.2f ms", m_average_detect_time);
    }

    if (output > 0.95f)
    {
        m_number_of_detections++;
        if (m_number_of_detections > 1)
        {
            m_number_of_detections = 0;
            ESP_LOGI(TAG, "P(%.2f): Wake word detected", output);
            return true;
        }
    }

    return false;
}


void DetectWakeWordState::exitState()
{
    delete m_nn;
    m_nn = nullptr;
    delete m_audio_processor;
    m_audio_processor = nullptr;

    uint32_t free_ram = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Free RAM after cleanup: %lu bytes", free_ram);
}
