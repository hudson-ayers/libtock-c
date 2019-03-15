#include <stdbool.h>
#include <stdio.h>

#include <adc.h>
#include <timer.h>

#include <ieee802154.h>
#include <udp.h>

static unsigned char BUF_BIND_CFG[2 * sizeof(sock_addr_t)];

void print_ipv6(ipv6_addr_t *);

void print_ipv6(ipv6_addr_t *ipv6_addr) {
  for (int j = 0; j < 14; j += 2) {
    printf("%02x%02x:", ipv6_addr->addr[j], ipv6_addr->addr[j + 1]);
  }
  printf("%02x%02x", ipv6_addr->addr[14], ipv6_addr->addr[15]);
}

static bool adc_sample;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    adc_sample = true;
}

/*
const uint32_t FREQS[7] = {25,100,500,1000,2000,5000,10000};

static int channels_done;
static bool adc_done;
static void adc_cb(int callback_type,
                   int arg1,
                   int arg2,
                   void* callback_args) {
    channels_done++;
    if (channels_done == 7) {
        adc_done = true;
    }
}
*/
static adc_buffer[100];
int main(void) {
  //printf("[IPv6_Sense] Starting IPv6 Sensors App.\n");
  //printf("[IPv6_Sense] Sensors will be sampled and transmitted.\n");

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

  //printf("Opening socket on ");
  //print_ipv6(&ifaces[0]);
  //printf(" : %d, and binding to that socket.\n", addr.port);
  int bind_return = udp_bind(&handle, &addr, BUF_BIND_CFG);

  if (bind_return < 0) {
    printf("Failed to bind to port: failure=%d\n", bind_return);
    return -1;
  }

  sock_addr_t destination = {
    ifaces[1],
    16123
  };

  tock_timer_t timer;
  timer_every(1000, timer_cb, NULL, &timer);

  uint32_t length = 100;
  unsigned buffer_idx = 0;
  unsigned num_samples = 1;
  uint16_t avg_buffer[num_samples];
  while (1) {
    //channels_done = 0;
    //for(int i=0; i<7; i++) {
    //    adc_sample_buffer_async(i, FREQS[i], adc_buffer, FREQS[i]);
    //}
    yield_for(&adc_sample);
    adc_sample = false;
    int err = adc_sample_buffer_sync(0, 7000, adc_buffer, length);
    if (err < 0) {
        printf("Error sampling ADC: %d\n", err);
    }
    
    uint16_t sum = 0;
    for (unsigned i=0; i<length; i++) {
        sum += adc_buffer[i];
    }
    avg_buffer[buffer_idx++] = sum/length;

    if (buffer_idx < num_samples)
        continue;
    buffer_idx = 0;

    int max_tx_len = udp_get_max_tx_len();
    int packet_length = num_samples*sizeof(uint16_t);
    if (packet_length > (int)sizeof(packet)){
        packet_length = sizeof(packet);
    }
    int buf_idx=0;
    do {
        memcpy(packet, &avg_buffer[buf_idx], packet_length);

        if (packet_length > max_tx_len) {
          printf("Cannot send packets longer than %d bytes without changing"
                 " constants in kernel\n", max_tx_len);
          return 0;
        }
        //printf("Sending packet (length %d) --> ", packet_length);
        //print_ipv6(&(destination.addr));
        //printf(" : %d\n", destination.port);
        ssize_t result = udp_send_to(packet, packet_length, &destination);

        switch (result) {
          case TOCK_SUCCESS:
            //printf("Packet sent.\n\n");
            break;
          default:
            printf("Error sending packet %d\n\n", result);
        }

        buf_idx += packet_length;
        packet_length = (num_samples-buf_idx-1)*sizeof(uint16_t);
        if (packet_length > (int) sizeof(packet)){
            packet_length = sizeof(packet);
        }
    } while(packet_length > 0);

  }
}
