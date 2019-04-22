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

#include <internal/nonvolatile_storage.h>
#include "ninedof.h"

static bool gpio_interrupt;

static void gpio_cb (int pin_num,
                     int pin_val,
                     __attribute__ ((unused)) int unused,
                     __attribute__ ((unused)) void* userdata) {

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

  unsigned num_measurements = 5;
  double accel_mags[num_measurements];

  uint8_t writebuf[4];
  size_t size = 512;
  int ret = nonvolatile_storage_internal_write_buffer(writebuf, size);
  int offset = 0;
  int len = sizeof(double)*num_measurements;

  while(1) {
    //Wait for interrupt
    yield_for(&gpio_interrupt);
    gpio_interrupt = false;

    // take accelerometer measurements
    unsigned int ii;
    for (ii = 0; ii < num_measurements; ii++) {
      unsigned accel_mag = ninedof_read_accel_mag();
      printf("accel square = %x\n", accel_mag);
      //printf("********************\n");
      accel_mags[ii] = accel_mag + g;
      //delay_ms(100);
    }

    // windowing/thresholding
    //unsigned steps = 0;
    //for (unsigned ii = 0; ii < num_measurements - 1; ii++) {
    //  if (accel_mags[ii] < 0 && accel_mags[ii + 1] > 0) {
    //    // step occurred
    //    steps++;
    //  }
    //}
    //printf("%u steps occurred.\n", steps);

    memcpy(writebuf, accel_mags, len);
    // write to flash
    ret  = nonvolatile_storage_internal_write(offset, len);
    if (ret != 0) {
      printf("\tERROR calling write\n");
      return ret;
    }
    offset += len;
    gpio_enable_interrupt(0, Change);

  }
}
