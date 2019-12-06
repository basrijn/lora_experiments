# LORA testing
## Reliable datagram

When encodings don't match, the devices still connect?

### RH_RF95::Bw125Cr45Sf128
On the desk: RSSI -103, SNR 9

Signal loss: 9 meters, RSSI -133, SNR -7

### RH_RF95::Bw500Cr45Sf128
On the desk: RSSI -103, SNR 9

Signal loss: 9 meters, RSSI -136, SNR -9

### RH_RF95::Bw31_25Cr48Sf512
On the desk: RSSI -105, SNR 9

Signal loss: 9 meters, RSSI -136, SNR -9

### RH_RF95::Bw125Cr48Sf4096
On the desk: RSSI -111, SNR 9

Signal loss: 10 meters, RSSI -136, SNR -8

### Custom modem config
```
const RH_RF95::ModemConfig myProfile = {
        RH_RF95_BW_125KHZ | RH_RF95_CODING_RATE_4_5,
        RH_RF95_SPREADING_FACTOR_128CPS};
```
Exact same results

## RAW packet (driver.send())
Adafruit example code

On the desk: RSSI -32, SNR 9
