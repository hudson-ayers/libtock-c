#include <stdio.h>
#include <timer.h>
#include <internal/nonvolatile_storage.h>
#include <adc.h>
#include <clock.h>
#include "ninedof.h"
#include "gpio.h"

struct ninedof_data {
  int x;
  int y;
  int z;
  bool fired;
};
static struct ninedof_data res = { .fired = false };
static void ninedof_cb(int x, int y, int z, void* ud) {
        gpio_toggle(1);
  struct ninedof_data* result = (struct ninedof_data*) ud;
  result->x     = x;
  result->y     = y;
  result->z     = z;
  result->fired = true;
}

uint8_t readbuf[512];
uint8_t writebuf[512];
bool read_done = false;
bool write_done = false;
static void read_done_cb(int length,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* ud) {
    read_done = true;
}

static void write_done_cb(int length,
                       __attribute__ ((unused)) int arg1,
                       __attribute__ ((unused)) int arg2,
                       __attribute__ ((unused)) void* ud) {
    write_done = true;
}

// callback for timers
bool timer_done = false;
static void timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) {
    timer_done = true;
}


int main(void) {

    gpio_enable_output(0);
    gpio_enable_output(1);
    gpio_enable_output(2);
    printf("Begin test\n");
    
    // Setup I2C
    double accel_vals[15];
    int mag_size = sizeof(double);
    ninedof_subscribe(ninedof_cb, (void*) &res);

    // Setup flash
    int ret = nonvolatile_storage_internal_read_buffer(readbuf, 512);
    if (ret != 0) printf("ERROR setting read buffer\n");
    ret = nonvolatile_storage_internal_read_done_subscribe(read_done_cb, NULL);
    if (ret != 0) printf("ERROR setting read done callback\n");
    ret = nonvolatile_storage_internal_write_buffer(writebuf, 512);
    if (ret != 0) printf("ERROR setting write buffer\n");
    ret = nonvolatile_storage_internal_write_done_subscribe(write_done_cb, NULL);
    if (ret != 0) printf("ERROR setting write done callback\n");

    // Timer 
    tock_timer_t timer;
    timer_every(500, timer_cb, NULL, &timer);

    //clock_set(DFLL);
    while(1){
        for (int i=0; i< 5; i++) {
            res.fired = false;
            ninedof_start_accel_reading();
            yield_for(&res.fired);
            
            accel_vals[i*3] = res.x;
            accel_vals[i*3+1] = res.y;
            accel_vals[i*3+2] = res.z;
        }

        int x,y,z;
        for (int ii = 0; ii < 5; ii++) {
            x = accel_vals[ii*3];
            y = accel_vals[ii*3+1];
            z = accel_vals[ii*3+2];
            double mag = sqrt(x*x + y*y + z*z);
            memcpy(writebuf+mag_size*ii, &mag, mag_size);
        }

        // Write to flash
        read_done = false;
        ret = nonvolatile_storage_internal_read(0, 512);
        if (ret != 0) printf("ERROR calling read\n");
        yield_for(&read_done);
        gpio_toggle(2);

        write_done = false;
        ret  = nonvolatile_storage_internal_write(0, 512);
        if (ret != 0) printf("ERROR calling write\n");
        yield_for(&write_done);
        gpio_toggle(2);

        yield_for(&timer_done);
        timer_done = false;
    }

}

