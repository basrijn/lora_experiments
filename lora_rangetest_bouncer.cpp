// Simple test program to do range tests
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <Adafruit_NeoPixel.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t lastMillis;
uint32_t lastModeChange;
uint8_t loraConfigDelay = 15.0; // Default of 15 second wait

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LED and Neopixel
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LED 13
#define NEO  11
Adafruit_NeoPixel pixels(1, NEO, NEO_RGB + NEO_KHZ800);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LORA modem (RFM9x)

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LORA_ADDRESS 2

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

uint16_t loraAckTimeout = 2500;
uint8_t loraRetries = 0;
uint8_t loraConfigChoice = 5;

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, LORA_ADDRESS);

char data[RH_RF95_MAX_MESSAGE_LEN] = "";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void neoPixelBySNR()
{
  int red = 0;
  int green = 0;
  int blue = 0;

  if ( driver.lastSNR() > 10)
  {
    green = 125;
  }
  else if (driver.lastSNR() < -20 )
  {
    red = 125;
  }
  else
  {
    green = round(( (driver.lastSNR()+20.0) / 30.0 )  * 125.0);
    Serial.print("Green = "); Serial.println(green);
    Serial.print("For SNR = "); Serial.println(driver.lastSNR());
    red = 125 - green; 
  }
  

  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();
}

void ledShowConfig()
{
  // Turn on the LED with a specific color to show the active mode
  int red = 0;
  int green = 0;
  int blue = 0;

  pixels.clear();
  
  
  switch (loraConfigChoice)
  {
  case 5:
    blue = 255;
    break;
  case 4:
    red = 255;
    green = 255;
    break;
  case 3:
    red = 128;
    blue = 128;
    break;
  case 2:
    red = 255;
    green = 255;
    blue=255;
    break;
  case 1:
    red = 255;
    green = 165;
    break;
  }

  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();
  delay(100);
  pixels.clear();
  pixels.show();
  delay(100);
  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();
  delay(100);
  pixels.clear();
  pixels.show();
  
}

/// Function to walk thru the differnt Lora configurations. Longest range to shortest range
void loraConfig()
{
  // Cycle the modem, just to be sure
  digitalWrite(RFM95_RST, LOW);
  delay(500);
  digitalWrite(RFM95_RST, HIGH);
  delay(1500);
  manager.init();

  driver.setFrequency(RF95_FREQ);
  driver.setTxPower(23, false);

  // Loop back if did all tests
  if (loraConfigChoice < 1)
  {
    Serial.println("\n---------------------------------");
    Serial.println(" All tests completed. Restart now");
    Serial.println("---------------------------------");
    loraConfigChoice = 5;
  }
  else if (loraConfigChoice > 5)
  {
    // This should never have happened, but .. 
    loraConfigChoice = 5;
  }

  // Determine what modem config we need to set
  Serial.print("\nSetting modem configuration: [");Serial.print(loraConfigChoice);Serial.print("] ");

  // Show the mode by coolor
  ledShowConfig();

  switch (loraConfigChoice)
  {
  case 5:
    driver.setSignalBandwidth(62500);
    driver.setCodingRate4(8);
    driver.setSpreadingFactor(11); // 2048
    Serial.println("SUCCESS");
    Serial.println("Set Config to: Bw = 62.5 kHz, Cr = 4/8, Sf = 2048 chips/symbol | Slowest config");
    loraAckTimeout = 1500;
    loraConfigDelay = 70.0; // We wait for a minute in Mode 5, to pick the other side up
    break;
  case 4:
    driver.setSignalBandwidth(125000);
    driver.setCodingRate4(8);
    driver.setSpreadingFactor(12); // 4096
    Serial.println("SUCCESS");
    Serial.println("Set Config to: Bw = 125 kHz, Cr = 4/8, Sf = 4096 chips/symbol | SF 12 / Lowest Bw config");
    loraAckTimeout = 1200;
    loraConfigDelay = 15.0;
    break;
  case 3:
    driver.setSignalBandwidth(250000);
    driver.setCodingRate4(8);
    driver.setSpreadingFactor(12); // 4096
    Serial.println("SUCCESS");
    Serial.println("Set Config to: Bw = 250 kHz, Cr = 4/8, Sf = 4096 chips/symbol | Bas Mode 1");
    loraAckTimeout = 650;
    break;
  case 2:
    driver.setSignalBandwidth(500000);
    driver.setCodingRate4(8);
    driver.setSpreadingFactor(11); // 2048
    Serial.println("SUCCESS");
    Serial.println("Set Config to: Bw = 500 kHz, Cr = 4/8, Sf = 2048 chips/symbol | SF 11 / Highest Bw");
    loraAckTimeout = 500;
    break;
  case 1:
    driver.setSignalBandwidth(250000);
    driver.setCodingRate4(6);
    driver.setSpreadingFactor(9); // 512
    Serial.println("SUCCESS");
    Serial.println("Set Config to: Bw = 250 kHz, Cr = 4/6, Sf = 512 chips/symbol | Bas Mode 3");
    loraAckTimeout = 300;
    break;
  }

  // Setting timeout
  manager.setTimeout(loraAckTimeout);

  // Setting retries
  manager.setRetries(loraRetries);

  // And set the timer for this mode
  lastModeChange = millis();

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
  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0 , 0, 0));
  pixels.show();

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

    Serial.println("\n----------------------");
    Serial.print("Received a ping ["); Serial.print(loraConfigChoice);Serial.println("]");
    Serial.println("----------------------");
		digitalWrite(LED, HIGH);
    neoPixelBySNR();

		// If there is a valid message available for this node, send an acknowledgement to the SRC address (blocking until this is complete),
		// then copy the message to buf and return true else return false
		if (manager.recvfromAck(buf, &len, &from))
		{
			Serial.print("Got request from : ");
			Serial.print(from, DEC);

			Serial.print(" with RSSI=");
			Serial.print(driver.lastRssi(), DEC);

			Serial.print(" / SNR=");
			Serial.print(driver.lastSNR(), DEC);

			Serial.print(" / MSG = [");
			Serial.print((char *)buf);
			Serial.println("]");

      Serial.println("Moving to faster Lora setting");
      loraConfigChoice -= 1;
      loraConfig();
		}
		else
		{
			Serial.println(".. Failed to Deliver ACK");
		}
    digitalWrite(LED, LOW);
    pixels.setPixelColor(0, pixels.Color(0 , 0, 0));
    pixels.show();

  }
  
}



void loop()
{
  // Fall back to the longest range mode after 15 seconds of failures
  if ( (millis() - lastModeChange) > (loraConfigDelay*1000.0))
  {
    Serial.println("\n---------------------------------");
    Serial.println(" Retry timeout, next Config");
    Serial.println("---------------------------------");

    loraConfigChoice -= 1;
    loraConfig();
  }

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

    Serial.print("In LORA mode = ["); Serial.print(loraConfigChoice);Serial.println("]");
  }

}
