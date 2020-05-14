# LORA testing

## Issues
If two radio do not want to connect reliably, check if you are using a narrow bandwidth and high spreading factor. Configurations < 125 kHz and SF 12 are not likely to work reliably.

On the opposite side, the very fast modes, I've not been able to get anything faster then Bw=500 kHz, CR=4/6, Sf=7 (128) to work with the radios next to each other on the desk.

## Reliable datagram
Make sure to only do a manager.init() and not a driver.init() AND an manager.init(). The effect is that the TX power is dropped to near zero.

For the slower modes the setTimeout must be adjusted to get a reliable connection. Based on a 100 packet experiment, with a required pass rate of 97% the following timeouts could be used (pick higher to be save). Below are my custom settings I tuned for my own needs:

|Notes| Bandwdith | Coding Rate |Spreading factor | Reliable timeout|
|-----|-----|-----|-----|-----|
|Slowest| 62.5 kHz|4/8|11 (2048)|1300|
|SF 12 / Lowest Bw| 125 kHz|4/8|12 (4086)|1000|
|Bas Mode 1|250 kHz|4/8|12 (4096)|550|
|Bas Mode 2|125 kHz|4/8|9 (512)|365|
|SF 11 / Highest Bw| 500 kHz|4/8|11 (4086)|250|
|Bas Mode 3|250 kHz|4/6|9 (512)|77|
|Fastest, reliable in bench tests|500 kHz|4/6|7 (128)|12|

Below are the default modes
### RH_RF95::Bw125Cr45Sf128, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range
Timeout 40

### RH_RF95::Bw500Cr45Sf128, Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range
Timeout 20

### RH_RF95::Bw31_25Cr48Sf512, Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range
Timeout set to 700ms

### RH_RF95::Bw125Cr48Sf4096, Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range
Timeout set to 1000

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

| SpreadingFactor (Cfg)        | SF (chips/symbol)| Demodulator SNR  |
| ------------- |-------------| -----|
| 6      | 64 | -5 dB |
| 7      | 128 | -7.5 dB |
| 8      | 256 | -10 dB |
| 9      | 512 | -12.5 dB |
| 10      | 2024 | -15 dB |
| 11      | 2048 | -17.5 dB |
| 12      | 4086 | -20dB |

To optimize range, going by the "How setting and theoretical range combine" article I listed below:
* Each doubling of the bandwidth correlates to almost 3dB less link budget
* Each increase in S of 1 unit (in Sf) increases the SNR at the demodulator of 2.5 dB.
* Maximize coding rate (CR) to increase reliability. Overhead goes from 1.25 for 4/5 to 2.00 for 4/8

## Test equipment
* Xmitter: Feather 32u4 with RFM95C and external antenna
* Receiver: Feather 32u4 with RFM95 and external antenna

## Test results
### Burnaby Lake
From the dock to a path close to the finish line for a distance of 2.87km over mostly water and some trees
| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-12|2427|
|4|Bw=125, CR=4/8, SF=12|-15|2168|
|3|Bw=250, CR=4/8, SF=12|-4|1086|
|2|Bw=500, CR=4/8, SF=11|-12|274|

For a second run

| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-12|2427|
|4|Bw=125, CR=4/8, SF=12|-9|2169|
|3|Bw=250, CR=4/8, SF=12|-10|1085|
|2|Bw=500, CR=4/8, SF=11|-13|275|
|1|Bw=250, CR=4/6, SF=9|-9|151

### Cabin

At park entrance bridge, 2.87km
| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-14|2427|
|4|Bw=125, CR=4/8, SF=12|-12|2169|
|3|Bw=250, CR=4/8, SF=12|-16|1085|

Second set at little hill

| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-10|2427|
|4|Bw=125, CR=4/8, SF=12|-15|2169|
|3|Bw=250, CR=4/8, SF=12|-15|1085|
|2|Bw=500, CR=4/8, SF=11|-17|275|

Edge of field, 1.65km

| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-9|2427|
|4|Bw=125, CR=4/8, SF=12|-3|2169|
|3|Bw=250, CR=4/8, SF=12|-6|1085|
|2|Bw=500, CR=4/8, SF=11|-12|275|
|1|Bw=250, CR=4/6, SF=9|-9|151

Run 2

| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-6|2427|
|4|Bw=125, CR=4/8, SF=12|-10|2169|
|3|Bw=250, CR=4/8, SF=12|-15|1085|
|2|Bw=500, CR=4/8, SF=11|-16|275|
|1|Bw=250, CR=4/6, SF=9|-11|151

### Close to home
LaCrosse box

| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-3|2427|
|4|Bw=125, CR=4/8, SF=12|-7|2456|
|3|Bw=250, CR=4/8, SF=12|-9|1218|
|2|Bw=500, CR=4/8, SF=11|-12|274|
|1|Bw=250, CR=4/6, SF=9|-9|149

First river spot, 280m behind a hill

| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-7|2427|
|4|Bw=125, CR=4/8, SF=12|-4|2169|
|3|Bw=250, CR=4/8, SF=12|-8|1085|
|2|Bw=500, CR=4/8, SF=11|-10|274|
|1|Bw=250, CR=4/6, SF=9|-6|150

Second river spot, 580m behind a hill
| Protocol #| Protocol detail| SNR|Xmit time
|---|---|---|---|
|5|Bw=62.5, CR=4/8, SF=11|-17|2426|
|4|Bw=125, CR=4/8, SF=12|-17|2169|
|3|Bw=250, CR=4/8, SF=12|-19|1086|

## Lora Mesh
By using the Radiohead Mesh mode, nodes can be added when coverage is not sufficient for the START and FINISH to reach each other directly. It will take longer for the packets to arrive, but they will. In theory the mesh can be extended as far as needed. 
https://nootropicdesign.com/projectlab/2018/10/20/lora-mesh-networking/

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
