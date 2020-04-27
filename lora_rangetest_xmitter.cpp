// Simple test program to do time based range tests

#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Range test global vars
uint32_t lastTX = millis();
uint32_t txInterval = 1000; // THis value must be large enough to fit the transmission and all retries in
uint8_t txRetries = 0;
uint32_t lastModeChange;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LED
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LED 13

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LORA modem (RFM9x)
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LORA_ADDRESS 101

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

uint16_t loraAckTimeout;
uint8_t loraRetries = 0;
uint8_t loraConfigChoice = 5;

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, LORA_ADDRESS);

uint8_t data[] = "PING";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

/// Function to walk thru the differnt Lora configurations. Longest range to shortest range
void loraConfig()
{
  // Loop back if did all tests
  if (loraConfigChoice < 1)
  {
    Serial.println("\n---------------------------------");
    Serial.println(" All tests completed. Restart now");
    Serial.println("---------------------------------");
    loraConfigChoice = 5;
  }

  // Determine what modem config we need to set
  Serial.print("\nSetting modem configuration: [");
  Serial.print(loraConfigChoice);
  Serial.print("] ");

  switch (loraConfigChoice)
  {
  case 5:
    //Bw41_7Cr48Sf4086
    driver.setSignalBandwidth(41700);
    driver.setCodingRate4(8);
    driver.setSpreadingFactor(11); // 2048
    Serial.println("SUCCESS\nSet Config to: Bw = 41.7 kHz, Cr = 4/8, Sf = 2048chips/symbol, CRC on. Slow+long range");
    loraAckTimeout = 2500;
    break;
  case 4:
    if (!driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096))
      Serial.println("FAILED");
    Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range");
    loraAckTimeout = 2000;
    break;
  case 3:
    //Bw41_7Cr45Sf512
    driver.setSignalBandwidth(41700);
    driver.setCodingRate4(5);
    driver.setSpreadingFactor(9); // 512
    Serial.println("SUCCESS\nSet Config to: Bw = 41.7 kHz, Cr = 4/5, Sf = 512chips/symbol, CRC on. Slow+long range");
    loraAckTimeout = 2000;
    break;
  case 2:
    //Bw500Cr45Sf4096
    driver.setSignalBandwidth(500000);
    driver.setCodingRate4(5);
    driver.setSpreadingFactor(12); // 512
    Serial.println("SUCCESS\nSet Config to: Bw = 500 kHz, Cr = 4/5, Sf = 4096chips/symbol, CRC on. Bas Fast+long range");
    loraAckTimeout = 2000;
    break;
  case 1:
    if (!driver.setModemConfig(RH_RF95::Bw500Cr45Sf128))
      Serial.println("FAILED");
    Serial.println("SUCCESS\nSet Config to: Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range");
    loraAckTimeout = 250;
    break;
  }

  /*
  // With the modem roughly configured, do an On Air test to set better timeout
    // Build the packet
  snprintf(data, RH_RF95_MAX_MESSAGE_LEN, "GNIP XXX"); // Same length string to get correct on air time
  lastTX = millis();

  // We send a broadcast packet to determine what our on air time is (it does not wait for an ACK)
  Serial.println("\nSending a broadcast packet for On Air time calculation");
  digitalWrite(LED, HIGH);
  manager.sendtoWait((uint8_t *)data, sizeof(data), 255);
  digitalWrite(LED, LOW);
  uint16_t loraOnAirTime = millis() - lastTX;
  Serial.print(".. On Air time = ");Serial.println(loraOnAirTime);

  // Do some sanity checking on the timeOut setting. The driver varies the timeout between 1 x timeout to 1.5 x timeout
  // So create a small buffer for the remote end to respond
  if (loraOnAirTime + 50 > loraAckTimeout) // Remote end should be able to do it's processing in 50 ms
  {
    // The timeout is probably not sufficient for the current settings
    Serial.print(".... Adjusting timeout to: ");
    loraAckTimeout = loraOnAirTime + 50;
    Serial.println(loraAckTimeout);
  }
  */

  // Setting timeout
  manager.setTimeout(loraAckTimeout);

  // Setting retries
  manager.setRetries(loraRetries);

  // Setting the Xmit delay to 2 the timeout. Otherwise the ACK from the bouncer might still be in flight on MSG from xmitter
  txInterval = 2 * loraAckTimeout;

  delay(1000);

  // And set the timer for this mode
  lastModeChange = millis();
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // uncomment to have the sketch wait until Serial is ready

  Serial.println("----------------------------------------");
  Serial.println(" Range test xmitter v 0.0");
  Serial.println("----------------------------------------");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  // LED
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(LED, OUTPUT);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Lora
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println("\nSetting up the LORA modem");

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(250);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(100);
  digitalWrite(RFM95_RST, HIGH);
  delay(500);

  // Initiate the manager process, this will also initialize the underlying driver
  Serial.print(".. Initializing Manager: ");

  while (!manager.init())
  {
    Serial.println("FAILED");
    delay(1000);
  }
  Serial.println("SUCCESS");
  delay(500);

  Serial.print(".. Setting radio frequency: ");
  if (!driver.setFrequency(RF95_FREQ))
  {
    Serial.println("FAILED");
    while (1)
      ;
  }
  else
  {
    Serial.print(" Successfully set to ");
    Serial.println(RF95_FREQ);
  }

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setTxPower(23, false);

  loraConfig();

  Serial.println("\nAll setup tasks complete");
}

/// Send a packet and report on success (with retries etc)
void loraPing()
{
  // Send a ping Out

  if (millis() - lastTX < txInterval)
    return;

  // Now we can get to the work of sending something out
  Serial.println("\n----------------------");
  Serial.print("Sending a ping out ["); Serial.print(loraConfigChoice);Serial.println("]");
  Serial.println("----------------------");
  digitalWrite(LED, HIGH);

  // Now we send the PING
  //snprintf(data, RH_RF95_MAX_MESSAGE_LEN, "PING %d", txCounter);

  // Reset the retry counter
  manager.resetRetransmissions();
  // Reset the TX timer
  lastTX = millis();

  Serial.print("Active Timeout = ");Serial.println(loraAckTimeout);

  if (!manager.sendtoWait(data, sizeof(data), 2))
  {
    Serial.println(".. No Ack received");
    txRetries += 1;
  }
  else
  {
    Serial.println(".. Ack received");
    Serial.print(".... RSSI=");
    Serial.print(driver.lastRssi(), DEC);
    Serial.print(" / SNR=");
    Serial.println(driver.lastSNR(), DEC);

    Serial.print(".... Time in sendtoWait: ");
    Serial.println(millis() - lastTX);

    // How many retransmissions before we got a reply back
    Serial.print(".... Retries needed: ");
    Serial.println(txRetries);

    Serial.print("For config choice ");
    Serial.print(loraConfigChoice);
    Serial.print(" the timeOut that worked was ");
    Serial.println(loraAckTimeout);

    
    loraConfigChoice -= 1;
    loraConfig();
  }

  digitalWrite(LED, LOW);
  // Reset the TX timer
  lastTX = millis();
}

void loop()
{  
  // Proceed to next mode after 3 minutes
  if ( (millis() - lastModeChange) > (3.0*60.0*1000.0))
  {
    Serial.println("\n---------------------------------");
    Serial.println(" Retry timeout, try next mode ");
    Serial.println("---------------------------------");
    loraConfigChoice -= 1;
    loraConfig();
  }

  loraPing();
}
