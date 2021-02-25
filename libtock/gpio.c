#include "gpio.h"

int gpio_count(void) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 0, 0, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS_U32) {
    return rval.data[0];
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_enable_output(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 1, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_set(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 2, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_clear(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 3, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_toggle(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 4, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_enable_input(GPIO_Pin_t pin, GPIO_InputMode_t pin_config) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 5, pin, pin_config);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_read(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 6, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS_U32) {
    return rval.data[0];
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_enable_interrupt(GPIO_Pin_t pin, GPIO_InterruptMode_t irq_config) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 7, pin, irq_config);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_disable_interrupt(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 8, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_disable(GPIO_Pin_t pin) {
  syscall_return_t rval = command2(GPIO_DRIVER_NUM, 9, pin, 0);
  if (rval.type == TOCK_SYSCALL_SUCCESS) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(rval.data[0]);
  }
}

int gpio_interrupt_callback(subscribe_upcall callback, void* callback_args) {
  subscribe_return_t sval = subscribe2(GPIO_DRIVER_NUM, 0, callback, callback_args);
  if (sval.success) {
    return TOCK_SUCCESS;
  } else {
    return tock_error_to_rcode(sval.error);
  }
}

