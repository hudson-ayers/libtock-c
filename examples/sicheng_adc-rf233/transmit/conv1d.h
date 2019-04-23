#pragma once

void convolve(const uint16_t Signal[/* SignalLen */], size_t SignalLen,
              const uint16_t Kernel[/* KernelLen */], size_t KernelLen,
              uint16_t Result[/* SignalLen + KernelLen - 1 */]);
void printSignal(const char* Name,
                 uint16_t Signal[/* SignalLen */], size_t SignalLen);
int compute_1dconv(uint16_t *adc_buffer, unsigned length, uint16_t *result);
