// This set of small applications will run thru timeout settings until a timeout has
// been found that results in > 97% reliable transmission
// See https://github.com/basrijn/lora_experiments/blob/master/README.md for details

// You have to manually set the encoding for each test run, see around line 92

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
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// Blinky on transmit
#define LED 13

// Global variable
uint16_t ackTimeout = 10;

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

  Serial.println("Feather LoRa Timeout Xmitter");

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
  manager.setRetries(0);          // 3 by default, 0  means only ever send once

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
  ackTimeout = 500;

  int16_t packetnum = 0; // packet counter, we increment per xmission
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

  Serial.println("Done with setup()");
} // End of setup

uint8_t data[] = "."; // Short message to get a ballpark nr. Then try with a longer message
// uint8_t data[] = "Hello World. How are you doing today? Hope all is well";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
int16_t counterSuccess = 0;
int16_t counterFail = 0;

uint32_t lastTX = millis();
uint32_t txDelay = 1100;

void loop()
{

  if (millis() - lastTX > 1000)
  {
    Serial.print("\nTX -> Bouncer client [");
    Serial.print(counterSuccess + counterFail);
    Serial.println("]");

    // Send a message to manager_server
    // Reset the retry counter
    manager.resetRetransmissions();
    long lastMillis = millis();

    // SendtoWait = Send the message (with retries) and waits for an ack. Returns true if an acknowledgement is received.
    // Synchronous: any message other than the desired ACK received while waiting is discarded. Blocks until an ACK is received or all retries are exhausted
    digitalWrite(LED, HIGH);
    Serial.print(".. sendtoWait: ");
    if (manager.sendtoWait(data, sizeof(data), CLIENT_ADDRESS))
    {
      Serial.println("SUCCESS");
      counterSuccess++;

      Serial.print("   RSSI: ");
      Serial.println(driver.lastRssi(), DEC);

      Serial.print("   SNR: ");
      Serial.println(driver.lastSNR(), DEC);

      Serial.print(".... Time in sendtoWait: ");
      Serial.println(millis() - lastMillis);

      // How many retransmissions before we got a reply back
      Serial.print(".... Retransmissions needed: ");
      Serial.println(manager.retransmissions());
      // How many retries before we got a reply back
      Serial.print(".... Retries completed: ");
      Serial.println(manager.retries());

      // oprint(".. Message send");
      // Now wait for a reply from the server
      uint8_t len = sizeof(buf);
      uint8_t from;
    }
    else
    {
      Serial.println("FAILED");
      counterFail++;
      // How many retransmissions before we got a reply back
      Serial.print(".... Retransmissions needed: ");
      Serial.println(manager.retransmissions());
      // How many retries before we got a reply back
      Serial.print(".... Retries completed: ");
      Serial.println(manager.retries());

      Serial.print(".... Time in sendtoWait: ");
      Serial.println(millis() - lastMillis);
      // oprint("SendtoWait failed");
    }
    digitalWrite(LED, LOW);

    lastTX = millis();

    // What is our successrate so far
    Serial.print("\nTX SUCCESS % = ");
    Serial.print(((float)counterSuccess / float(counterSuccess + counterFail)) * 100.0);
    Serial.print(" [ S|");
    Serial.print(counterSuccess);
    Serial.print(" - F|");
    Serial.print(counterFail);
    Serial.println(" ]");

    // Run some timeout autotune logic
    uint16_t ackIncrease = 9999;

    if ((counterFail > 5) && (counterSuccess == 0))
    {
      // Things are going really poorly! Skip ahead 10%
      ackIncrease = 0.10 * ackTimeout;
    }
    else if (((counterFail + counterSuccess) > 20) && (counterFail >= 3))
    {
      ackIncrease = 0.05 * ackTimeout; // Step 5%
    }
    // Stop testing if we found a good result
    if ((counterSuccess > 100) && (counterFail < 3))
    {
      Serial.print("\n Timeout autotune successfull with a timeout value of ");
      Serial.println(ackTimeout);
      while (true)
        ;
    }

    if (ackIncrease != 9999) // We have had an update to the timeout
    {
      if (ackIncrease < 1)
      {
        ackIncrease = 1;
      }
      else if ((ackIncrease > 15) && (ackTimeout < 500)) // Regulate stepsize somewhat
      {
        ackIncrease = 15;
      }

      ackTimeout += ackIncrease;

      Serial.println("\n\n-------------------------------------------");
      Serial.println(".. Timeout to low for reliable transmission");
      Serial.print("   Setting new timeOut = ");
      Serial.println(ackTimeout);
      Serial.println("-------------------------------------------");
      manager.setTimeout(ackTimeout);
      counterSuccess = counterFail = 0;
    }
  }
}
