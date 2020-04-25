# LORA testing

## Issues
If two radio do not want to connect reliably, check if you are using a narrow bandwidth. Small amounts if misallignment between the frequencies used by the radios causes issues.

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
# Range testing
Somebody online suggests that the best predictor of success for a transmission is the SNR for the specific SF setting as documented in the datasheet. For the RF9x that is:

| SpreadingFactor (Cfg)        | SF (chips/symbol)| Demodular SNR  |
| ------------- |-------------| -----|
| 6      | 64 | -5 dB |
| 7      | 128 | -7.5 dB |
| 8      | 256 | -10 dB |
| 9      | 512 | -12.5 dB |
| 10      | 2024 | -15 dB |
| 11      | 2048 | -17.5 dB |
| 12      | 4086 | -20dB |


# Interesting reading
## Adafruit articles for their hardware
https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/

## Lora discussions
Long range tests comparing different chipsets, they use a custome encoding (SF=12, BW=125 kHz, CR=4/5) that is used by default in LoraWan environments
https://www.rocketscream.com/blog/2017/08/21/the-sx1276-modules-shootout-hoperfs-rfm95w-vs-nicerfs-lora1276-c1-vs-hpdteks-hpd13/

Long range tests with different hardware
https://www.cooking-hacks.com/documentation/tutorials/extreme-range-lora-sx1272-module-shield-arduino-raspberry-pi-intel-galileo.html

How setting and theoretical range combine
https://medium.com/home-wireless/testing-lora-radios-with-the-limesdr-mini-part-2-37fa481217ff

Long range (baloon) tests with some range results
http://forum.anarduino.com/posts/list/60.page

Code example of setting a custom config
https://arduino.stackexchange.com/questions/39609/radiohead-library-custom-configuration-for-rfm96-lora

Low data rate bit issue in Radiohead
https://hackaday.io/project/27791-esp32-lora-oled-module/log/69630-radiohead-rf95-driver-low-data-rate-optimization

Post discussion relationship between the different Lora protocol settings (RF)
https://electronics.stackexchange.com/questions/278192/understanding-the-relationship-between-lora-chips-chirps-symbols-and-bits

Paper describing the spreading factor etc in great detail (background on how that all works)
https://www.ncbi.nlm.nih.gov/pmc/articles/PMC5038744/
