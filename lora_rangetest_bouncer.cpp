// Simple test program to do range tests
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t lastMillis;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LED
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LED 13

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LORA modem (RFM9x)

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LORA_ADDRESS 2

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

uint16_t loraAckTimeout = 20;
uint8_t loraRetries = 1;
uint8_t loraConfigChoice = 5;

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, LORA_ADDRESS);

char data[RH_RF95_MAX_MESSAGE_LEN] = "";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];


/// Function to walk thru the differnt Lora configurations. Longest range to shortest range
void loraConfig()
{
  // Determine what modem config we need to set
  Serial.print("\nSetting modem configuration: [");Serial.print(loraConfigChoice);Serial.print("] ");

  switch (loraConfigChoice)
  {
  case 5:
    //Bw41_7Cr48Sf4086
    driver.setSignalBandwidth(62500);
    driver.setCodingRate4(8);
    driver.setSpreadingFactor(12); // 4096
    Serial.println("SUCCESS\nSet Config to: Bw = 41.7 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range");
    break;
  case 4:
    if (!driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096))
      Serial.println("FAILED");
    Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range");
    break;
  case 3:
    if (!driver.setModemConfig(RH_RF95::Bw125Cr45Sf128))
      Serial.println("FAILED");
    Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range");
    break;
  case 2:
    if (!driver.setModemConfig(RH_RF95::Bw500Cr45Sf128))
      Serial.println("FAILED");
    Serial.println("SUCCESS\nSet Config to: Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range");
    break;
  case 1:
    //Bw125Cr45Sf4086
    driver.setSignalBandwidth(125000);
    driver.setCodingRate4(5);
    driver.setSpreadingFactor(12); // 4096
    loraAckTimeout = 1000;
    Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/5, Sf = 4096chips/symbol, CRC on. LoraWan default range");
    break;
  }

  // Setting timeout
  Serial.print("\nSetting modem timeout: ");
  loraAckTimeout = 2000;
  manager.setTimeout(loraAckTimeout);
  Serial.println("DONE");

  // Setting retries
  Serial.print("\nSetting modem retries: ");
  manager.setRetries(loraRetries);
  Serial.println("DONE");
}

void setup()
{
  Serial.begin(115200);
  //while (!Serial)
  ; // uncomment to have the sketch wait until Serial is ready

  Serial.println("----------------------------------------");
  Serial.println(" Range test bouncer v 0.0");
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

  // Manager settings
  manager.setTimeout(loraAckTimeout); // 200ms by default, not enough for slow modes?
  manager.setRetries(loraRetries);          // 3 by default, 0  means only ever send once

  loraConfig();

  Serial.println("\nAll setup tasks complete");
  Serial.println("------------------------");
}

/// Handle inbound traffic here
void processLora()
{
  if (driver.available()) // There is something received
	{
		// Should be a message for us now
		uint8_t len = sizeof(buf);
		uint8_t from;

    Serial.println("\nMessage received. Sending ACK");
		digitalWrite(LED, HIGH);

		// If there is a valid message available for this node, send an acknowledgement to the SRC address (blocking until this is complete),
		// then copy the message to buf and return true else return false
		if (manager.recvfromAck(buf, &len, &from))
		{
			Serial.print("\nRX: Got request from : ");
			Serial.print(from, DEC);

			Serial.print(" with RSSI=");
			Serial.print(driver.lastRssi(), DEC);

			Serial.print(" / SNR=");
			Serial.print(driver.lastSNR(), DEC);

			Serial.print(" / MSG = [");
			Serial.print((char *)buf);
			Serial.println("]");

      Serial.println("\nMoving to faster Lora setting");
      loraConfigChoice -= 1;
      loraConfig();
		}
		else
		{
			Serial.println(".. Failed to Deliver ACK");
		}
    digitalWrite(LED, LOW);
  }
  
}



void loop()
{
  processLora();

 
  // Quick blink on the LED three seconds to show we are indeed alive
  if (millis() - lastMillis > 3000)
  {
    lastMillis = millis();
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(150);
    digitalWrite(LED, LOW);
  }

}
