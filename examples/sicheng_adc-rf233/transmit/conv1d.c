
// Source: https://stackoverflow.com/questions/8424170/1d-linear-convolution-in-ansi-c-code

#include <stddef.h>
#include <stdio.h>
#include "conv1d.h"

void convolve(const uint16_t Signal[/* SignalLen */], size_t SignalLen,
              const uint16_t Kernel[/* KernelLen */], size_t KernelLen,
              uint16_t Result[/* SignalLen + KernelLen - 1 */])
{
  size_t n;

  for (n = 0; n < SignalLen + KernelLen - 1; n++)
  {
    size_t kmin, kmax, k;

    Result[n] = 0;

    kmin = (n >= KernelLen - 1) ? n - (KernelLen - 1) : 0;
    kmax = (n < SignalLen - 1) ? n : SignalLen - 1;

    for (k = kmin; k <= kmax; k++)
    {
      Result[n] += Signal[k] * Kernel[n - k];
    }
  }
}

void printSignal(const char* Name,
                 uint16_t Signal[/* SignalLen */], size_t SignalLen)
{
  size_t i;

  for (i = 0; i < SignalLen; i++)
  {
    printf("%s[%zu] = %d\n", Name, i, Signal[i]);
  }
  printf("\n");
}

int compute_1dconv(uint16_t *adc_buffer, unsigned length, uint16_t *result)
{
  unsigned kernel_length = 5;
  uint16_t kernel[] = { 1, 1, 1, 1, 1 };

  convolve(adc_buffer, length, 
           kernel, kernel_length,
           result);

  printSignal("signal", adc_buffer, length);
  printSignal("kernel", kernel, kernel_length);
  printSignal("result", result, length + kernel_length - 1);

  return 0;
}
