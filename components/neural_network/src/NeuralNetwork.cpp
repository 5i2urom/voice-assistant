#include "NeuralNetwork.h"
#include "model.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

uint8_t NeuralNetwork::s_tensor_arena[NeuralNetwork::kArenaSize] __attribute__((aligned(16)));

NeuralNetwork::NeuralNetwork()
{
    m_error_reporter = new tflite::MicroErrorReporter();

    m_tensor_arena = s_tensor_arena;

    TF_LITE_REPORT_ERROR(m_error_reporter, "Loading model");

    m_model = tflite::GetModel(converted_model_tflite);
    if (m_model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(m_error_reporter, "Model provided is schema version %d not equal to supported version %d.",
                             m_model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    m_resolver.AddConv2D();
    m_resolver.AddMaxPool2D();
    m_resolver.AddFullyConnected();
    m_resolver.AddMul();
    m_resolver.AddAdd();
    m_resolver.AddLogistic();
    m_resolver.AddReshape();
    m_resolver.AddQuantize();
    m_resolver.AddDequantize();

    m_interpreter = new tflite::MicroInterpreter(
        m_model, m_resolver, m_tensor_arena, kArenaSize, m_error_reporter);

    TfLiteStatus allocate_status = m_interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(m_error_reporter, "AllocateTensors() failed");
        return;
    }

    size_t used_bytes = m_interpreter->arena_used_bytes();
    TF_LITE_REPORT_ERROR(m_error_reporter, "Used bytes %d\n", used_bytes);

    input = m_interpreter->input(0);
    output = m_interpreter->output(0);
}

NeuralNetwork::~NeuralNetwork()
{
    delete m_interpreter;
    delete m_error_reporter;
}

float *NeuralNetwork::getInputBuffer()
{
    return input->data.f;
}

float NeuralNetwork::predict()
{
    m_interpreter->Invoke();
    return output->data.f[0];
}
