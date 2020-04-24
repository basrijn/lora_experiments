// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-

// See https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module/

// LORA
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Lora Feather

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7


// Feather basic with fly wired Lora
/*
#define RFM95_CS 1
#define RFM95_RST 11
#define RFM95_INT 2
*/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// Blinky on transmit
#define LED 13

// Global variable
uint16_t ackTimeout = 10;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);

  while (!Serial)
  {
    delay(1);
  }

  delay(100);

  Serial.println("Feather LoRa Client");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Interesting reading
  // https://www.rocketscream.com/blog/2017/08/21/the-sx1276-modules-shootout-hoperfs-rfm95w-vs-nicerfs-lora1276-c1-vs-hpdteks-hpd13/
  // https://www.ncbi.nlm.nih.gov/pmc/articles/PMC5038744/
  // https://www.cooking-hacks.com/documentation/tutorials/extreme-range-lora-sx1272-module-shield-arduino-raspberry-pi-intel-galileo/
  // http://forum.anarduino.com/posts/list/60.page
  // https://arduino.stackexchange.com/questions/39609/radiohead-library-custom-configuration-for-rfm96-lora
  // https://electronics.stackexchange.com/questions/278192/understanding-the-relationship-between-lora-chips-chirps-symbols-and-bits

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Change the default config to use slow but long range
  // Options are:
  // Bw125Cr45Sf128, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range.
  // Bw500Cr45Sf128, Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range
  // Bw31_25Cr48Sf512, Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range.
  // Bw125Cr48Sf4096, Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range
  // Bw125Cr45Sf4096, Bw = 125 kHz, Cr = 4/5, Sf = 4096chips/symbol, CRC on. LoraWan default

  // Initiate the manager process
  while (!manager.init())
  {
    Serial.println("Init of Manager failed");
    delay(1000);
  }
  Serial.println("Manager initialized");

  manager.setTimeout(ackTimeout); // 200ms by default, not enough for slow modes?
  manager.setRetries(0);    // 3 by default, 0  means only ever send once

  if (!driver.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  else
  {
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);
  }

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setTxPower(23, false);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
   /* Serial.print("\nSetting modem configuration: ");
  if (!driver.setModemConfig(RH_RF95::Bw125Cr45Sf128)) {
    Serial.println("FAILED");
    while (1);
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range."); */
/*
  Serial.print("\nSetting modem configuration: ");
  if (!driver.setModemConfig(RH_RF95::Bw500Cr45Sf128)) {
    Serial.println("FAILED");
    while (1);
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range.");
 */


  Serial.print("\nSetting modem configuration: ");
    if (!driver.setModemConfig(RH_RF95::Bw31_25Cr48Sf512))
    {
        Serial.println("FAILED");
        while (1)
            ;
    }
    Serial.println("SUCCESS\nSet Config to: Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range.");

  /* Serial.print("\nSetting modem configuration: ");
  if (!driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096))
  {
    Serial.println("FAILED");
    while (1)
      ;
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range."); */

  /* Serial.print("\nSetting modem configuration: ");
  driver.setSignalBandwidth(125000);
  driver.setCodingRate4(5);
  driver.setSpreadingFactor(7);
  Serial.println("SUCCESS\n Set Config to custom (LoraWAN default)"); */



  Serial.println("Done with setup()");
} // End of setup

void loop()
{
  if (manager.available()) // There is something received
  {
    // Should be a message for us now
    uint8_t len = sizeof(buf);
    uint8_t from;
    uint8_t to;

    Serial.println("\nMessage received");
    digitalWrite(LED, HIGH);

    // If there is a valid message available for this node, send an acknowledgement to the SRC address (blocking until this is complete),
    // then copy the message to buf and return true else return false
    if (manager.recvfromAck(buf, &len, &from, &to))
    {
      Serial.print("\nRX: Got request from: ");
      Serial.print(from, DEC);

      Serial.print(" for: ");
      Serial.print(to, DEC);

      Serial.print(" with RSSI=");
      Serial.print(driver.lastRssi(), DEC);

      Serial.print(" / SNR=");
      Serial.print(driver.lastSNR(), DEC);

      Serial.print(" / MSG = [");
      Serial.print((char *)buf);
      Serial.println("]");

      Serial.println("ACK send");
    }
    else
    {
      Serial.println(".. Failed to Deliver ACK");
    }
    digitalWrite(LED, LOW);
  }
}
