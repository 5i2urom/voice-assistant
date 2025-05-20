#ifndef __NeuralNetwork__
#define __NeuralNetwork__

#include <stdint.h>

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

namespace tflite
{
    template <unsigned int tOpCount>
    class MicroMutableOpResolver;
    class ErrorReporter;
    class Model;
    class MicroInterpreter;
} // namespace tflite

struct TfLiteTensor;

class NeuralNetwork
{
private:
    static const int kArenaSize = 25000;

    tflite::MicroMutableOpResolver<10> m_resolver;
    tflite::ErrorReporter *m_error_reporter;
    const tflite::Model *m_model;
    tflite::MicroInterpreter *m_interpreter;
    TfLiteTensor *input;
    TfLiteTensor *output;

    alignas(16) static uint8_t s_tensor_arena[kArenaSize];
    uint8_t *m_tensor_arena;

public:
    NeuralNetwork();
    ~NeuralNetwork();
    float *getInputBuffer();
    float predict();
};

#endif
