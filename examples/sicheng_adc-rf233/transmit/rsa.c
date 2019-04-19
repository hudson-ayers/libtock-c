// RSA

#define ADC_SAMPLES 1000

int x_rsa, y_rsa, n_rsa, t_rsa, flag_rsa;
long int e_rsa[ADC_SAMPLES], d_rsa[ADC_SAMPLES], temp_rsa[ADC_SAMPLES], j_rsa, m_rsa[ADC_SAMPLES], en_rsa[ADC_SAMPLES];

int prime(long int pr) {
    int i;
    j_rsa = sqrt(pr);
    for(i = 2; i <= j_rsa; i++) {
        if (pr%i == 0)
            return 0;
    }
    return 1;
}

long int cd(long int a) {
    long int k = 1; 
    while (1) {
        k = k + t_rsa;
        if (k % a == 0)
            return (k/a); 
    }
}

// function to generate encryption key
void encryption_key() {
    int k = 0; 
    for (int i = 2; i < t_rsa; i++) {
        if (t_rsa % i == 0)
            continue; 
        flag_rsa = prime(i);
        if (flag_rsa == 1 && i != x_rsa && i != y_rsa) {
            e_rsa[k] = i;
            flag_rsa = cd(e_rsa[k]);
            if (flag_rsa > 0) {
                d_rsa[k] = flag_rsa;
                k++;
            }
            if (k == 99)
                break;
        }
    }
}

// function to encrypt the message
void encrypt() {
    long int pt, ct, key = e_rsa[0], k, len;
    int i = 0; 
    len = ADC_SAMPLES;
    while(i != len) {
        pt = m_rsa[i];
        pt = pt - 96; 
        k = 1; 
        for (j_rsa = 0; j_rsa < key; j_rsa++) {
            k = k * pt;
            k = k % n_rsa; 
        }
        temp_rsa[i] = k;
        ct = k + 96; 
        en_rsa[i] = ct; 
        i++;
    }
    en_rsa[i] = -1;
}

void compute_rsa(uint16_t *adc_buffer, unsigned length) {
  x_rsa = 7; // arbitrary prime
  y_rsa = 13; // arbitrary prime 

  for (int i = 0; i < length; i++)
      m_rsa[i] = adc_buffer[i];
  n_rsa = x_rsa * y_rsa;
  t_rsa = (x_rsa-1) * (y_rsa-1);
  encryption_key();
  encrypt();
}
