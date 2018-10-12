UDP Sensor App
=============

An example app for platforms with sensors and an 802.15.4 radio that broadcasts
periodic sensor readings over the network. Currently, it sends UDP packets
using 6lowpan to a single neighbor with an IP address known ahead of time.
The contents of the payload are as follows:

1. A random number, generated by the application accessing the underlying hardware RNG.

2. Three sensor reaadings (temperature, relative humidity, light).

3. A count of how many times the user button has been clicked.

The precise format of the payload follows:

```
rand: 212; 2 deg C; 1%; 3 lux; btn_count: 5
```

## Running

To run this app, simply place the IP address of the destination node in xxx.
Notably, until Tock has neighbor discovery implemented, you also have to configure
the destination MAC address in the kernel (in boards/imix/src/main.rs).

### UDP Layer Reception Test

The best way to test this app is by using UDP reception on another Imix.
Program the kernel on two imixs. On one, program the `udp_rx` app in
`userland/examples/tests/udp/udp_rx/udp_rx`.
On the other, program this app.

You'll see packets printed on the console of the form:

```
2 deg C; 1%; 3 lux;2 deg C; 1%; 3 lux;
```

These lines contain the payload of the received UDP packet.