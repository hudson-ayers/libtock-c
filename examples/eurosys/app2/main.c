#include <timer.h>
#include <tock.h>
#include "gpio.h"
#include "clock.h"

static bool gpio_interrupt;

static void gpio_cb (int pin_num,
                     int pin_val,
                     __attribute__ ((unused)) int unused,
                     __attribute__ ((unused)) void* userdata) {

  gpio_disable_interrupt(0);
  gpio_interrupt = true;
}

int main(void) {
  clock_set(DFLL);
  // GPIO input from proximity sensor or door open/close sensor
  gpio_interrupt_callback(gpio_cb, NULL);
  gpio_enable_input(0, PullDown);
  gpio_enable_interrupt(0, Change);
  gpio_enable_output(1);
  //gpio_enable_output(2);

  while(1) {
    //Wait for interrupt
    yield_for(&gpio_interrupt);
    gpio_interrupt = false;
    delay_ms(1000);

    gpio_toggle(1);
    gpio_enable_interrupt(0, Change);

  }
}
