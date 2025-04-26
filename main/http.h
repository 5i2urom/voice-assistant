#pragma once

#include <stdint.h>                   
#include "esp_err.h"                

extern int16_t temperature;
extern int16_t humidity;
extern int16_t noise_level;
extern int16_t threshold_noise_level;

void start_http_server(void);
