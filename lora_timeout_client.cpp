// This set of small applications will run thru timeout settings until a timeout has
// been found that results in > 97% reliable transmission
// See https://github.com/basrijn/lora_experiments/blob/master/README.md for details

// You have to manually set the encoding for each test run, see around line 97

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
uint16_t packetCounter = 0;
uint32_t lastMillis = millis();
uint8_t data[] = "Hello World. How are you doing today? Hope all is well";

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

  Serial.println("Feather LoRa Timeout Client");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

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

  driver.setSignalBandwidth(250000);
  driver.setCodingRate4(8);
  driver.setSpreadingFactor(12);
  ackTimeout = 1500;


  Serial.println("Done with setup()");
} // End of setup

void loop()
{
  if (driver.available()) // There is something received
  {
    // Should be a message for us now
    uint8_t len = sizeof(buf);
    uint8_t from;
    uint8_t to;
    packetCounter+= 1;

    Serial.print("\nMessage received ["); Serial.print(packetCounter); Serial.println("]");
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
