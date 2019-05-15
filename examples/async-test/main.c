#include <stdio.h>
#include <timer.h>
#include <internal/nonvolatile_storage.h>
#include <adc.h>
#include <clock.h>
#include "ninedof.h"
#include "gpio.h"

typedef struct {
  bool fired;
  uint8_t channel;
  uint16_t sample;
  uint32_t length;
  uint16_t* buffer;
  int error;
} adc_data_t;

static void adc_cb(int callback_type,
                    int arg1,
                    int arg2,
                    void* callback_args) {
    adc_data_t* result = (adc_data_t*)callback_args;

        gpio_toggle(0);
  switch (callback_type) {
    case SingleSample:
      result->error   = TOCK_SUCCESS;
      result->channel = arg1;
      result->sample  = arg2;
      break;

    case ContinuousSample:
      result->error   = TOCK_SUCCESS;
      result->channel = arg1;
      result->sample  = arg2;
      break;

    case SingleBuffer:
      result->error   = TOCK_SUCCESS;
      result->channel = (arg1 & 0xFF);
      result->length  = ((arg1 >> 8) & 0xFFFFFF);
      result->buffer  = (uint16_t*)arg2;
      break;

    case ContinuousBuffer:
      result->error   = TOCK_SUCCESS;
      result->channel = (arg1 & 0xFF);
      result->length  = ((arg1 >> 8) & 0xFFFFFF);
      result->buffer  = (uint16_t*)arg2;
      break;

    default:
      result->error = TOCK_FAIL;
      break;
  }

  result->fired = true;
}

struct ninedof_data {
  int x;
  int y;
  int z;
  bool fired;
};
static struct ninedof_data res = { .fired = false };
static void ninedof_cb(int x, int y, int z, void* ud) {
        gpio_toggle(1);
  struct ninedof_data* result = (struct ninedof_data*) ud;
  result->x     = x;
  result->y     = y;
  result->z     = z;
  result->fired = true;
}

uint8_t readbuf[512];
uint8_t writebuf[512];
bool read_done = false;
bool write_done = false;
static void read_done_cb(int length,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* ud) {
    read_done = true;
}

static void write_done_cb(int length,
                       __attribute__ ((unused)) int arg1,
                       __attribute__ ((unused)) int arg2,
                       __attribute__ ((unused)) void* ud) {
    write_done = true;
}

// callback for timers
bool timer_done = false;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    timer_done = true;
}


int main(void) {

    gpio_enable_output(0);
    gpio_enable_output(1);
    gpio_enable_output(2);
    printf("Begin test\n");
    
    // Setup ADC
    uint8_t channel = 0;
    uint32_t freq = 31500;
    uint32_t length = 100;
    uint16_t buf[length];
    adc_data_t result = {0};
    adc_set_callback(adc_cb, (void*) &result);
    adc_set_buffer(buf, length);

    // Setup I2C
    ninedof_subscribe(ninedof_cb, (void*) &res);

    // Setup flash
    int ret = nonvolatile_storage_internal_read_buffer(readbuf, 512);
    if (ret != 0) printf("ERROR setting read buffer\n");
    ret = nonvolatile_storage_internal_read_done_subscribe(read_done_cb, NULL);
    if (ret != 0) printf("ERROR setting read done callback\n");
    ret = nonvolatile_storage_internal_write_buffer(writebuf, 512);
    if (ret != 0) printf("ERROR setting write buffer\n");
    ret = nonvolatile_storage_internal_write_done_subscribe(write_done_cb, NULL);
    if (ret != 0) printf("ERROR setting write done callback\n");

    // Timer 
    tock_timer_t timer;
    timer_every(500, timer_cb, NULL, &timer);

    //clock_set(DFLL);
    while(1){
        gpio_toggle(0);
        result.fired = false;
        adc_buffered_sample(channel, freq);
        gpio_toggle(1);
        res.fired = false;
        ninedof_start_accel_reading();

        yield_for(&result.fired);
        yield_for(&res.fired);
        adc_stop_sampling();

        // Output adc results
        //printf("Sample taken\n");
        gpio_toggle(2);
        for (uint32_t i = 0; i < length; i++) {
            // convert to millivolts
            writebuf[i] = (buf[i] * 3300) / 4095;
        }
        writebuf[length] = res.x + res.y + res.z;
        
        gpio_toggle(0);
        // Write to flash
        read_done = false;
        ret = nonvolatile_storage_internal_read(0, 512);
        if (ret != 0) printf("ERROR calling read\n");
        yield_for(&read_done);
        gpio_toggle(2);

        write_done = false;
        ret  = nonvolatile_storage_internal_write(0, 512);
        if (ret != 0) printf("ERROR calling write\n");
        yield_for(&write_done);
        gpio_toggle(2);

        yield_for(&timer_done);
        timer_done = false;
    }

}

