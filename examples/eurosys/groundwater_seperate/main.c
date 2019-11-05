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

const double g = -9.8;

static bool timer_called;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    timer_called = true;
}

int main(void) {
  gpio_enable_output(1);

  tock_timer_t timer;
  timer_every(1900, timer_cb, NULL, &timer);

  /* Begin accel / flash enable code */
  unsigned num_measurements = 5;
  double accel_vals[num_measurements*3];

  int mag_size = sizeof(double);
  int len = mag_size*num_measurements;
  uint8_t writebuf[len];
  int ret = nonvolatile_storage_internal_write_buffer(writebuf, len);
  int offset = 0;
  /* End accel / flash enable code */

  //clock_set(DFLL);
  while(1) {
    //clock_set(RCSYS);
    yield_for(&timer_called);
    timer_called = false;

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
