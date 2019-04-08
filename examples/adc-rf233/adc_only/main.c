#include <stdbool.h>
#include <stdio.h>

#include <adc.h>
#include <timer.h>

static bool adc_sample;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    adc_sample = true;
}

#define ADC_SAMPLES 1000
static uint16_t adc_buffer[ADC_SAMPLES];
int main(void) {
  tock_timer_t timer;
  timer_every(5000, timer_cb, NULL, &timer);

  uint16_t length = ADC_SAMPLES;
  while (1) {
    yield_for(&adc_sample);
    adc_sample = false;
    //printf("About to sample...\n");
    int err = adc_sample_buffer_sync(0, 7000, adc_buffer, length);
    if (err < 0) {
        //printf("Error sampling ADC: %d\n", err);
    } else {
      //printf("first sample: %d\n", adc_buffer[0]);
    }
  }
}
