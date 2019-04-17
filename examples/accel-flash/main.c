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

#include <internal/nonvolatile_storage.h>
#include "ninedof.h"

static bool timer_interval;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    timer_interval = true;
}

const double g = -9.8;

// Step counter app
// TODO get sqrt working
int main(void) {
  printf("Step counter init\n");
  unsigned num_measurements = 100;
  double accel_mags[num_measurements];

  uint8_t writebuf[512];
  size_t size = 512;
  int ret = nonvolatile_storage_internal_write_buffer(writebuf, size);
  int offset = 0;
  int len = 0;

  tock_timer_t timer;
  timer_every(2000, timer_cb, NULL, &timer);

  while(1) {
    yield_for(&timer_interval);
    timer_interval = false;

    // take accelerometer measurements
    for (unsigned ii = 0; ii < num_measurements; ii++) {
      unsigned accel_mag = ninedof_read_accel_mag();
      printf("accel square = %u\n", accel_mag);
      printf("********************\n");
      accel_mags[ii] = accel_mag + g;
      delay_ms(2000);
    }

    // windowing/thresholding
    unsigned steps = 0;
    for (unsigned ii = 0; ii < num_measurements - 1; ii++) {
      if (accel_mags[ii] < 0 && accel_mags[ii + 1] > 0) {
        // step occurred
        steps++;
      }
    }

    printf("%u steps occurred.\n", steps);

    // write to flash
    ret  = nonvolatile_storage_internal_write(offset, len);
    if (ret != 0) {
      printf("\tERROR calling write\n");
      return ret;
    }
  }
}
