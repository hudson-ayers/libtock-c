#include <stdbool.h>
#include <stdio.h>

#include <adc.h>
#include <timer.h>

#include <math.h> // Added for RSA

#include <ieee802154.h>
#include <udp.h>
#include "fft.h"
#define DEBUG 0

static unsigned char BUF_BIND_CFG[2 * sizeof(sock_addr_t)];

static bool adc_sample;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    adc_sample = true;
}


const uint32_t FREQS[7] = {25,100,500,1000,2000,5000,10000};

#define ADC_SAMPLES 1000
static uint16_t adc_buffer[ADC_SAMPLES];

/* RSA */
int x_rsa, y_rsa, n_rsa, t_rsa, i_rsa, flag_rsa;
long int e_rsa[ADC_SAMPLES], d_rsa[ADC_SAMPLES], temp_rsa[ADC_SAMPLES], j_rsa, m_rsa[ADC_SAMPLES], en_rsa[ADC_SAMPLES];
int prime(long int);
void encryption_key(); 
long int cd(long int);
void encrypt(); 

int main(void) {
  if (DEBUG) {
    printf("Starting Power Clocks Test App.\n");
  }

  char packet[150];

  ieee802154_set_pan(0xABCD);
  ieee802154_config_commit();
  ieee802154_up();

  ipv6_addr_t ifaces[10];
  udp_list_ifaces(ifaces, 10);

  sock_handle_t handle;
  sock_addr_t addr = {
    ifaces[0],
    15123
  };

  if (DEBUG) {
    printf("Opening socket.\n");
  }
  int bind_return = udp_bind(&handle, &addr, BUF_BIND_CFG);

  if (bind_return < 0) {
    printf("Failed to bind to port: failure=%d\n", bind_return);
    return -1;
  } else if (DEBUG) {
    printf("Done binding. \n");
  }

  sock_addr_t destination = {
    ifaces[1],
    16123
  };

  tock_timer_t timer;
  timer_every(500, timer_cb, NULL, &timer);

  uint16_t length = ADC_SAMPLES;
  uint16_t buffer_idx = 0;
  unsigned num_averages = 1;
  uint16_t avg_buffer[num_averages];
  int fft_buf[16];
  int fft_mag[8];
  while (1) {
    /*channels_done = 0;
    for(int i=0; i<7; i++) {
        adc_sample_buffer_async(i, FREQS[i], adc_buffer, FREQS[i]);
    }*/
    yield_for(&adc_sample);
    timer_cancel(&timer);
    adc_sample = false;
    if (DEBUG) {
      printf("About to sample...\n");
    }
    int err = adc_sample_buffer_sync(0, 31500, adc_buffer, length);
    if (err < 0) {
      printf("Error sampling ADC: %d\n", err);
    } else {
      if (DEBUG) {
        printf("first sample: %d\n", adc_buffer[0]);
      }
    }

    /*---------*
     * Average *
     *---------*/
    
    uint16_t sum = 0;
    for (unsigned i=0; i<length; i++) {
        sum += adc_buffer[i];
    }
    avg_buffer[buffer_idx++] = sum/length;
    if (DEBUG) {
      printf("Average of last %d samples: %d\n", ADC_SAMPLES, avg_buffer[buffer_idx - 1]);
    }

    /*-----*
     * FFT *
     *-----*/
    
    // Begin fft computation on sets of 16 samples
    int i;
    volatile int k;
    volatile int res;
    for (k=0; k<1000/16; k++) {
      for (i=k*16; i<(k+1)*16; i++) {
        fft_buf[i % 16] = adc_buffer[i]; // Copy needed bc fft alg I found is int not uint16
      }
      res += fft(fft_buf, fft_mag);
      if (DEBUG) {
        int j;
        printf("fft mags: ");
        for (j=0; j<8; j++) {
          printf("%d\n", fft_mag[i]);
        }
        printf("\n");
      }
    }
    
    /*--------------------------*
     * Caesar Cypher Encryption *
     *--------------------------*/

    char encrypted_adc_buffer[ADC_SAMPLES];
    for (i = 0; (i < 100 && adc_buffer[i] != '\0'); i++)
        encrypted_adc_buffer[i] = adc_buffer[i] + 3; 
    
    /*----------------*
     * RSA Encryption *
     *----------------*/

    x_rsa = 7; // arbitrary prime
    y_rsa = 13; // arbitrary prime 

    for (i_rsa = 0; adc_buffer[i] != NULL; i_rsa++)
        m_rsa[i_rsa] = adc_buffer[i_rsa];
    n_rsa = x_rsa * y_rsa;
    t_rsa = (x_rsa-1) * (y_rsa-1);
    encryption_key();
    encrypt();
    
    /*---------*/
    
    int l;
    for (l=0; l<150000; l++) {
      k = k+1 - i;
      i = k;
    }

    //printf("k: %d\n", k);
    //printf("res: %d\n", res);


    // End computation

    //Keep sampling, computing averages on new buffers until num_averages reached
    if (buffer_idx < num_averages)
        continue;
    buffer_idx = 0;

    // Send list of averages in a UDP packet
    int max_tx_len = udp_get_max_tx_len();
    int payload_len = num_averages*sizeof(uint16_t);
    payload_len = 129;  // tmp to extend packet size to multiple frames
    if (payload_len > (int)sizeof(packet)){
        payload_len = sizeof(packet);
    }

    memcpy(packet, &avg_buffer, payload_len);

    if (payload_len > max_tx_len) {
      printf("Cannot send packets longer than %d bytes without changing"
             " constants in kernel\n", max_tx_len);
      return 0;
    }
    if (DEBUG) {
      printf("Sending packet (length %d) --> \n", payload_len);
    }
    ssize_t result = udp_send_to(packet, payload_len, &destination);

    switch (result) {
      case TOCK_SUCCESS:
        if (DEBUG) {
          printf("Packet sent.\n\n");
        }
        //timer_cancel(&timer);
        delay_ms(2000); //So that individual runs are visible on scope
        timer_every(500, timer_cb, NULL, &timer);
        break;
      default:
        if (DEBUG) {
          printf("Error sending packet %d\n\n", result);
        }
        //timer_cancel(&timer);
        delay_ms(2000); //So that individual runs are visible on scope
        timer_every(500, timer_cb, NULL, &timer);
        break;
    }
  }
}

/***** Helper functions for RSA *****/

int prime(long int pr) {
    int i;
    j_rsa = sqrt(pr);
    for(i = 2; i <= j_rsa; i++) {
        if (pr%i_rsa == 0)
            return 0;
    }
    return 1;
}

// function to generate encryption key
void encryption_key() {
    int k = 0; 
    for (i_rsa = 2; i_rsa < t_rsa; i_rsa++) {
        if (t_rsa % i_rsa == 0)
            continue; 
        flag_rsa = prime(i_rsa);
        if (flag_rsa == 1 && i_rsa != x_rsa && i_rsa != y_rsa) {
            e_rsa[k] = i_rsa;
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

long int cd(long int a) {
    long int k = 1; 
    while (1) {
        k = k + t_rsa;
        if (k % a == 0)
            return (k/a); 
    }
}

// function to encrypt the message
void encrypt() {
    long int pt, ct, key = e_rsa[0], k, len;
    i_rsa = 0; 
    len = ADC_SAMPLES;
    while(i_rsa != len) {
        pt = m_rsa[i_rsa];
        pt = pt - 96; 
        k = 1; 
        for (j_rsa = 0; j_rsa < key; j_rsa++) {
            k = k * pt;
            k = k % n_rsa; 
        }
        temp_rsa[i_rsa] = k;
        ct = k + 96; 
        en_rsa[i_rsa] = ct; 
        i_rsa++;
    }
    en_rsa[i_rsa] = -1;
}

