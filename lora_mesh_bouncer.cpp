// Simple test program to do range tests
#include <SPI.h>
#include <RH_RF95.h>
#include <RHRouter.h>
#include <RHMesh.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t lastMillis = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configure the LORA modem (RFM9x)
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LORA_ADDRESS 2

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define LORA_MAX_MESSAGE_LEN RH_MESH_MAX_MESSAGE_LEN

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHMesh *manager;


char data[LORA_MAX_MESSAGE_LEN] = "";
// Dont put this on the stack:
uint8_t buf[LORA_MAX_MESSAGE_LEN];

void setup()
{
  Serial.begin(115200);
  // while (!Serial)
  ; // uncomment to have the sketch wait until Serial is ready

  Serial.println("----------------------------------------");
  Serial.println(" Range test bouncer v 0.0");
  Serial.println("----------------------------------------");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  // LED
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(LED_BUILTIN, OUTPUT);
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
  manager = new RHMesh(driver, LORA_ADDRESS);
  Serial.print(".. Initializing Manager: ");

  while (!manager->init())
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
  driver.setTxPower(1, false);

  if (!driver.setModemConfig(RH_RF95::Bw125Cr45Sf128)) {
    Serial.println("FAILED");
    while (1);
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range."); 


  // Manager settings
  manager->setTimeout(500); // 200ms by default, not enough for slow modes?
  manager->setRetries(3);          // 3 by default, 0  means only ever send once

  Serial.println("\nAll setup tasks complete");
  Serial.println("------------------------");
}

void showMesh(uint8_t node)
{
  RHRouter::RoutingTableEntry *route = manager->getRouteTo(node);
  Serial.print("Dest = "); Serial.println(route->dest);
  Serial.print("Next hop = "); Serial.println(route->next_hop);
  Serial.print("State = "); Serial.println(route->state);
}

/// Handle inbound traffic here
void processLora()
{
  if (manager->available()) // There is something received
	{
		// Should be a message for us now
		uint8_t len = sizeof(buf);
		uint8_t from = 0;
    uint8_t dest = 0;

    Serial.println("\n----------------------");
    Serial.print(millis()); Serial.println(" - Received a packet");
    Serial.println("----------------------");
		digitalWrite(LED_BUILTIN, HIGH);

		// If there is a valid message available for this node, send an acknowledgement to the SRC address (blocking until this is complete),
		// then copy the message to buf and return true else return false
		if (manager->recvfromAck(buf, &len, &from, &dest))
		{
			Serial.print("Got request from : ");
			Serial.print(from, DEC);

      Serial.print(" / to : ");
			Serial.print(dest, DEC);

			Serial.print(" with RSSI=");
			Serial.print(driver.lastRssi(), DEC);

			Serial.print(" / SNR=");
			Serial.print(driver.lastSNR(), DEC);

			Serial.print(" / MSG = [");
			Serial.print((char *)buf);
			Serial.println("]");

      showMesh(from);
		}
		else
		{
      // Packet not for us
      Serial.println("Not for us, ignored");
		}
    digitalWrite(LED_BUILTIN, LOW);
  }
  
}



void loop()
{

  processLora();

  if (millis() - lastMillis > 1000)
  {
    lastMillis = millis();
  }

}
