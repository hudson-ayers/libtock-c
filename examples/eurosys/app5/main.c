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
#include <button.h>
#include <led.h>
#include <clock.h>
#include <timer.h>

#define DEBUG 1

static bool button_interrupt;
const double g = -9.8;

// Callback for button presses.
//   btn_num: The index of the button associated with the callback
//   val: 1 if pressed, 0 if depressed
static void button_callback(__attribute__ ((unused)) int btn_num,
                            int val,
                            __attribute__ ((unused)) int arg2,
                            __attribute__ ((unused)) void *ud) {
  if (val == 1) {
    button_interrupt = true;
    button_disable_interrupt(0);
    if (DEBUG) {
      printf("Button Press app4!\n");
    }
  }
}

int main(void) {
  /* Begin button enable code */
  button_subscribe(button_callback, NULL);
  int count = button_count();
  if (count < 0) {
    printf("Error detecting buttons: %i\n", count);
    return -1;
  } else if (count < 1) {
    printf("No buttons on this board!\n");
    return -2;
  }
  button_enable_interrupt(0);
  /* End button enable code */


  while(1) {
    button_interrupt = false;
    button_enable_interrupt(0);
    yield_for(&button_interrupt);
    led_toggle(0);
  }

  return 0;
}
