
/*---------*
 * Average *
 *---------*/

void compute_averages(uint16_t *adc_buffer, uint16_t length, uint16_t *avg_buffer, unsigned num_averages, uint16_t *buffer_idx) {
    uint16_t sum = 0;
    for (unsigned i=0; i<length; i++) {
        sum += adc_buffer[i];
    }
    avg_buffer[*buffer_idx++] = sum/length;
    printf("Average of last %d samples: %d\n", length, avg_buffer[*buffer_idx - 1]);
}
