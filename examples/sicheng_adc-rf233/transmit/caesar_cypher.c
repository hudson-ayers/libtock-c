// Caesar Cypher

void compute_caesar_cypher(uint16_t *adc_buffer, unsigned length, char *encrypted_adc_buffer) {
  char *casted_buffer = (char *)adc_buffer; 
  for (int i = 0; (i < 100 && casted_buffer[i] != '\0'); i++)
    encrypted_adc_buffer[i] = casted_buffer[i] + 3; 
}
