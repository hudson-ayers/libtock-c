#include "i2c_master_slave.h"
#include "tock.h"

struct i2c_master_slave_data {
  bool fired;
  int callback_type;
  int length;
};

static struct i2c_master_slave_data result = { .fired = false };

// Internal callback for faking synchronous reads
static void i2c_master_slave_cb(int callback_type,
                                int length,
                                __attribute__ ((unused)) int unused,
                                void* ud) {
  struct i2c_master_slave_data* data = (struct i2c_master_slave_data*) ud;
  data->callback_type = callback_type;
  data->length        = length;
  data->fired         = true;
}


int i2c_master_slave_set_callback(subscribe_upcall callback, void* callback_args) {
  subscribe_return_t subval = subscribe2(DRIVER_NUM_I2CMASTERSLAVE, 0, callback, callback_args);
  if (subval.success == 0) {
    return tock_error_to_rcode(subval.error);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_set_master_write_buffer(uint8_t* buffer, uint32_t len) {
  allow_rw_return_t aval = allow_readwrite(DRIVER_NUM_I2CMASTERSLAVE, 0, (void*) buffer, len);
  if (aval.success == 0) {
    return tock_error_to_rcode(aval.error);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_set_master_read_buffer(uint8_t* buffer, uint32_t len) {
  allow_rw_return_t aval = allow_readwrite(DRIVER_NUM_I2CMASTERSLAVE, 1, (void*) buffer, len);
  if (aval.success == 0) {
    return tock_error_to_rcode(aval.error);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_set_slave_read_buffer(uint8_t* buffer, uint32_t len) {
  allow_rw_return_t aval = allow_readwrite(DRIVER_NUM_I2CMASTERSLAVE, 2, (void*) buffer, len);
  if (aval.success == 0) {
    return tock_error_to_rcode(aval.error);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_set_slave_write_buffer(uint8_t* buffer, uint32_t len) {
  allow_rw_return_t aval = allow_readwrite(DRIVER_NUM_I2CMASTERSLAVE, 3, (void*) buffer, len);
  if (aval.success == 0) {
    return tock_error_to_rcode(aval.error);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_write(uint8_t address, uint8_t length) {
  uint32_t a = (((uint32_t) length) << 16) | address;
  syscall_return_t comval = command2(DRIVER_NUM_I2CMASTERSLAVE, 1, a, 0);
  if (comval.type < TOCK_SYSCALL_SUCCESS) {
    return tock_error_to_rcode(comval.data[0]);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_write_read(uint8_t address, uint8_t write_length, uint8_t read_length) {
  uint32_t a = (((uint32_t) write_length) << 16) | ((uint32_t) read_length << 8) | address;
  syscall_return_t comval = command2(DRIVER_NUM_I2CMASTERSLAVE, 7, a, 0);
  if (comval.type < TOCK_SYSCALL_SUCCESS) {
    return tock_error_to_rcode(comval.data[0]);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_read(uint16_t address, uint16_t len) {
  uint32_t a = (((uint32_t) len) << 16) | address;
  syscall_return_t comval = command2(DRIVER_NUM_I2CMASTERSLAVE, 2, a, 0);
  if (comval.type < TOCK_SYSCALL_SUCCESS) {
    return tock_error_to_rcode(comval.data[0]);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_listen(void) {
  syscall_return_t comval = command2(DRIVER_NUM_I2CMASTERSLAVE, 3, 0, 0);
  if (comval.type < TOCK_SYSCALL_SUCCESS) {
    return tock_error_to_rcode(comval.data[0]);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_set_slave_address(uint8_t address) {
  syscall_return_t comval = command2(DRIVER_NUM_I2CMASTERSLAVE, 6, address, 0);
  if (comval.type < TOCK_SYSCALL_SUCCESS) {
    return tock_error_to_rcode(comval.data[0]);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_enable_slave_read(uint32_t len) {
  syscall_return_t comval = command2(DRIVER_NUM_I2CMASTERSLAVE, 4, len, 0);
  if (comval.type < TOCK_SYSCALL_SUCCESS) {
    return tock_error_to_rcode(comval.data[0]);
  } else {
    return TOCK_SUCCESS;
  }
}

int i2c_master_slave_write_sync(uint8_t address, uint8_t len) {
  int err;
  result.fired = false;

  err = i2c_master_slave_set_callback(i2c_master_slave_cb, (void*) &result);
  if (err < 0) return err;

  err = i2c_master_slave_write(address, len);
  if (err < 0) return err;

  // Wait for the callback.
  yield_for(&result.fired);

  return result.length;
}

int i2c_master_slave_write_read_sync(uint8_t address, uint8_t wlen, uint8_t rlen) {
  int err;
  result.fired = false;

  err = i2c_master_slave_set_callback(i2c_master_slave_cb, (void*) &result);
  if (err < 0) return err;

  err = i2c_master_slave_write_read(address, wlen, rlen);
  if (err < 0) return err;

  // Wait for the callback.
  yield_for(&result.fired);

  return result.length;
}

int i2c_master_slave_read_sync(uint16_t address, uint16_t len) {
  int err;
  result.fired = false;

  err = i2c_master_slave_set_callback(i2c_master_slave_cb, (void*) &result);
  if (err < 0) return err;

  err = i2c_master_slave_read(address, len);
  if (err < 0) return err;

  // Wait for the callback.
  yield_for(&result.fired);

  return result.length;
}
