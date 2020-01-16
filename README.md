# LORA testing
## Reliable datagram
Make sure to only do a manager.init() and not a driver.init() AND an manager.init(). The effect is that the TX power is dropped to near zero.

For the slower modes the setTimeout must be adjusted to get a reliable connection. Based on a 100 packet experiment, with a required pass rate of 99% the following timeouts could be used (pick higher to be save):

### RH_RF95::Bw125Cr45Sf128, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range
Timeout 40

### RH_RF95::Bw500Cr45Sf128, Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range
Timeout 20

### RH_RF95::Bw31_25Cr48Sf512, Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range
Timeout set to 700ms

### RH_RF95::Bw125Cr48Sf4096, Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range
Timeout set to 1000

### Custom profile as used by LoRaWan nodes
```
driver.setSignalBandwidth(125000);
driver.setCodingRate4(5);
driver.setSpreadingFactor(7);
```
Timeout set to 40

## Cheat sheet for the spreading factors
The Radiohead library uses easy to translate values for most setX functions. Spreading factor is a little different and needs checking the header file. For easy translations:

- RH_RF95_SPREADING_FACTOR_64CPS == setSpreadingFactor(6)
- RH_RF95_SPREADING_FACTOR_128CPS == setSpreadingFactor(7)
- RH_RF95_SPREADING_FACTOR_256CPS == setSpreadingFactor(8)
- RH_RF95_SPREADING_FACTOR_512CPS == setSpreadingFactor(9)
- RH_RF95_SPREADING_FACTOR_1024CPS == setSpreadingFactor(10)
- RH_RF95_SPREADING_FACTOR_2048CPS == setSpreadingFactor(11)
- RH_RF95_SPREADING_FACTOR_4096CPS == setSpreadingFactor(12)

If you use a value < 6, it will 6 and > 12 will use 12
