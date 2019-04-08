#include <stdbool.h>
#include <stdio.h>

#include <adc.h>
#include <timer.h>

#include <ieee802154.h>
#include <udp.h>
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
int main(void) {
  if (DEBUG) {
    printf("Starting Power Clocks Test App.\n");
  }

  char packet[64];

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
  while (1) {
    /*channels_done = 0;
    for(int i=0; i<7; i++) {
        adc_sample_buffer_async(i, FREQS[i], adc_buffer, FREQS[i]);
    }*/
    yield_for(&adc_sample);
    adc_sample = false;
    if (DEBUG) {
      printf("About to sample...\n");
    }
    int err = adc_sample_buffer_sync(0, 31000, adc_buffer, length);
    if (err < 0) {
      printf("Error sampling ADC: %d\n", err);
    } else {
      if (DEBUG) {
        printf("first sample: %d\n", adc_buffer[0]);
      }
    }

    // Begin computation of average

    uint16_t sum = 0;
    for (unsigned i=0; i<length; i++) {
        sum += adc_buffer[i];
    }
    avg_buffer[buffer_idx++] = sum/length;
    if (DEBUG) {
      printf("Average of last %d samples: %d\n", ADC_SAMPLES, avg_buffer[buffer_idx - 1]);
    }

    // End computation

    //Keep sampling, computing averages on new buffers until num_averages reached
    if (buffer_idx < num_averages)
        continue;
    buffer_idx = 0;

    // Send list of averages in a UDP packet
    int max_tx_len = udp_get_max_tx_len();
    int payload_len = num_averages*sizeof(uint16_t);
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
        timer_cancel(&timer);
        delay_ms(5000); //So that individual runs are visible on scope
        timer_every(500, timer_cb, NULL, &timer);
        break;
      default:
        if (DEBUG) {
          printf("Error sending packet %d\n\n", result);
        }
        break;
    }
  }
}