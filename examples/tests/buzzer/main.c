#include <buzzer.h>

int main(void) {
  buzzer_buzz_sync(3000, 1000);
  delay_ms(2000);
  buzzer_buzz_sync(3500, 1000);
  delay_ms(2000);
  buzzer_buzz_sync(4000, 2000);
  return 0;
}
