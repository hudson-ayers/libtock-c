#include <stdio.h>

#include <button.h>
#include <led.h>
#include <clock.h>

#define DEBUG 1

// Callback for button presses.
//   btn_num: The index of the button associated with the callback
//   val: 1 if pressed, 0 if depressed
static void button_callback(__attribute__ ((unused)) int btn_num,
                            int val,
                            __attribute__ ((unused)) int arg2,
                            __attribute__ ((unused)) void *ud) {
  if (val == 1) {
    if (DEBUG) {
      printf("Button Press!\n");
    }
    led_toggle(0);
  }
}

int main(void) {
  //set_clock(DFLL);
  button_subscribe(button_callback, NULL);

  // Enable interrupts on each button.
  int count = button_count();
  if (count < 0) {
    printf("Error detecting buttons: %i\n", count);
    return -1;
  } else if (count < 1) {
    printf("No buttons on this board!\n");
    return -2;
  }

  // Enable the button interrupt
  button_enable_interrupt(0);

  return 0;
}
