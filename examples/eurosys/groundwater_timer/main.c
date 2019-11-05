#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <console.h>
#include <tock.h>
#include <internal/nonvolatile_storage.h>
#include "ninedof.h"
#include <led.h>
#include <clock.h>
#include <timer.h>
#include <gpio.h>

#define DEBUG 0

static bool gpio_interrupt;
const double g = -9.8;

static void gpio_cb (__attribute__ ((unused)) int pin_num,
                     __attribute__ ((unused)) int pin_val,
                     __attribute__ ((unused)) int unused,
                     __attribute__ ((unused)) void* userdata) {

  gpio_disable_interrupt(0);
  if (DEBUG) {
    printf("Button pressed1. \n");
  }
  gpio_interrupt = true;
}

int main(void) {
  //clock_set(RCFAST4M);
  /* Begin button enable code */
  /*
  button_subscribe(button_callback, NULL);
  int count = button_count();
  if (count < 0) {
    printf("Error detecting buttons: %i\n", count);
    return -1;
  } else if (count < 1) {
    printf("No buttons on this board!\n");
    return -2;
  }
  button_enable_interrupt(0); */
  /* End button enable code */
  /* Begin gpio enable code instead of button */
  gpio_interrupt_callback(gpio_cb, NULL);
  gpio_enable_input(0, PullDown);
  gpio_enable_interrupt(0, Change);
  gpio_enable_output(1);
  /* end gpio enable code */




  /* Begin accel / flash enable code */
  unsigned num_measurements = 5;
  double accel_vals[num_measurements*3];

  int mag_size = sizeof(double);
  int len = mag_size*num_measurements;
  uint8_t writebuf[len];
  int ret = nonvolatile_storage_internal_write_buffer(writebuf, len);
  int offset = 0;
  /* End accel / flash enable code */

  /* enable interrupt here to run *once*, emulating the second app being on a timer but firing at same time as first app */
  gpio_enable_interrupt(0, Change);
  //clock_set(DFLL);
  while(1) {
    gpio_interrupt = false;
    //clock_set(RCSYS);
    yield_for(&gpio_interrupt);

    // take accelerometer measurements
    //clock_set(RCFAST4M);
    unsigned int ii;
    int x, y, z;
    if (DEBUG) {
      printf("About to read accel.\n");
    }
    for (ii = 0; ii < num_measurements; ii++) {
      int err = ninedof_read_acceleration_sync(&x, &y, &z);
      if (err < 0) {
        printf("Ninedof error\n");
      }
      accel_vals[ii*3] = x;
      accel_vals[ii*3+1] = y;
      accel_vals[ii*3+2] = z;
    }
    //clock_set(RC80M);

    // calculate acceleration magnitudes
    for (ii = 0; ii < num_measurements; ii++) {
        x = accel_vals[ii*3];
        y = accel_vals[ii*3+1];
        z = accel_vals[ii*3+2];
        double mag = sqrt(x*x + y*y + z*z);
        memcpy(writebuf+mag_size*ii, &mag, mag_size);
    }

    if (DEBUG) {
      printf("About to write flash.\n");
    }
    // write to flash
    //clock_set(RC1M);
    ret  = nonvolatile_storage_internal_write(offset, len);
    if (ret != 0) {
      printf("\tERROR calling write\n");
      return ret;
    }
    if (DEBUG) {
      printf("Done writing to flash.\n");
    }
  }

  return 0;
}
