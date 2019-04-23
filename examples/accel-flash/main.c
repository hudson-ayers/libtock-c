#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <console.h>
#include <timer.h>
#include <tock.h>
#include "gpio.h"
#include "clock.h"
#include "math.h"

#include <internal/nonvolatile_storage.h>
#include "ninedof.h"

static bool gpio_interrupt;

static void gpio_cb (int pin_num,
                     int pin_val,
                     __attribute__ ((unused)) int unused,
                     __attribute__ ((unused)) void* userdata) {

  //gpio_toggle(1);
  gpio_disable_interrupt(0);
  gpio_interrupt = true;
}

const double g = -9.8;

int main(void) {
  printf("Movement tracker\n");

  // GPIO input from proximity sensor or door open/close sensor
  gpio_interrupt_callback(gpio_cb, NULL);
  gpio_enable_input(0, PullDown);
  gpio_enable_interrupt(0, Change);
  //gpio_enable_output(1);
  //gpio_enable_output(2);

  unsigned num_measurements = 5;
  double accel_vals[num_measurements*3];

  int mag_size = sizeof(double); 
  int len = mag_size*num_measurements;
  uint8_t writebuf[len];
  int ret = nonvolatile_storage_internal_write_buffer(writebuf, len);
  int offset = 0;

  while(1) {
    //Wait for interrupt
    yield_for(&gpio_interrupt);
    gpio_interrupt = false;

    // take accelerometer measurements
    unsigned int ii;
    int x, y, z;
    clock_set(RCFAST4M);
    for (ii = 0; ii < num_measurements; ii++) {
      //gpio_toggle(2);
      int err = ninedof_read_acceleration_sync(&x, &y, &z);
      if (err < 0) {
        printf("Ninedof error\n");
      }
      accel_vals[ii*3] = x;
      accel_vals[ii*3+1] = y;
      accel_vals[ii*3+2] = z;
      //delay_ms(100);
    }
    clock_set(RC80M);

    //gpio_toggle(1);
    // calculate acceleration magnitudes
    for (ii = 0; ii < num_measurements; ii++) {
        x = accel_vals[ii*3];
        y = accel_vals[ii*3+1];
        z = accel_vals[ii*3+2];
        double mag = sqrt(x*x + y*y + z*z);
        memcpy(writebuf+mag_size*ii, &mag, mag_size);
    }
    
    //gpio_toggle(2);
    // write to flash
    ret  = nonvolatile_storage_internal_write(offset, len);
    if (ret != 0) {
      printf("\tERROR calling write\n");
      return ret;
    }
    //gpio_toggle(1);
    //offset += len;
    gpio_enable_interrupt(0, Change);
    //gpio_toggle(2);

  }
}
