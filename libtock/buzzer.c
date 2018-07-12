#include "buzzer.h"
#include "tock.h"

struct data {
  bool done;
};

static struct data result = { .done = false };

// Internal callback for faking synchronous reads
static void cb(__attribute__ ((unused)) int unused0,
               __attribute__ ((unused)) int unused1,
               __attribute__ ((unused)) int unused2,
               void* ud) {
  struct data* data = (struct data*) ud;
  data->done = true;
}

int buzzer_set_callback(subscribe_cb callback, void* callback_args) {
  return subscribe(DRIVER_NUM_BUZZER, 0, callback, callback_args);
}

int buzzer_buzz(uint32_t frequency_hz, uint32_t duration_ms) {
  return command(DRIVER_NUM_BUZZER, 1, frequency_hz, duration_ms);
}

int buzzer_buzz_sync(uint16_t frequency_hz, uint16_t duration_ms) {
  int err;
  result.done = false;

  err = buzzer_set_callback(cb, (void*) &result);
  if (err < 0) return err;

  err = buzzer_buzz(frequency_hz, duration_ms);
  if (err < 0) return err;

  // Wait for the callback.
  yield_for(&result.done);

  return 0;
}
