// Caesar Cypher

#include "caesar_cypher.h"

void compute_caesar_cypher(uint16_t *adc_buffer, unsigned length, char *encrypted_adc_buffer) {
  char *casted_buffer = (char *)adc_buffer; 
  unsigned new_length = length * sizeof(uint16_t) / sizeof(char); 
  for (unsigned i = 0; i < new_length; i++)
    encrypted_adc_buffer[i] = casted_buffer[i] + 3; 
}
