#pragma once

#include "tock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_BUZZER 0x90000

// Pass a callback to the buzzer capsule.
int buzzer_set_callback(subscribe_cb callback, void* callback_args);

// Call the buzzer buzz command.
int buzzer_buzz(uint32_t frequency_hz, uint32_t duration_ms);

// Call the buzzer buzz command and wait for it to finish.
int buzzer_buzz_sync(uint16_t frequency_hz, uint16_t duration_ms);

#ifdef __cplusplus
}
#endif
