#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// Blinky on receipt
#define LED 13

// Are we using reliable datagrams
const bool useReliable = true;

void setup()
{
    pinMode(LED, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(9600);
    /*
    while (!Serial)
    {
        delay(1);
    }
    */

    delay(100);

    Serial.println("Feather LoRa TX Client");

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    Serial.print("\nInitializing driver: ");
    while (!driver.init())
    {
        Serial.println("FAILED");
        while (1)
            ;
    }
    Serial.println("SUCCESS");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    Serial.print(".. setFrequency: ");
    if (!driver.setFrequency(RF95_FREQ))
    {
        Serial.println("FAILED");
        while (1)
            ;
    }
    else
    {
        Serial.print("SUCCESS, set to ");
        Serial.println(RF95_FREQ);
    }

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 5 to 23 dBm:

    Serial.println(".. setTxPower");
    driver.setTxPower(23, false);

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
    /*  Serial.print("\nSetting modem configuration: ");
  if (!driver.setModemConfig(RH_RF95::Bw125Cr45Sf128)) {
    Serial.println("FAILED");
    while (1);
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range."); */

    /*     Serial.print("\nSetting modem configuration: ");
  if (!driver.setModemConfig(RH_RF95::Bw500Cr45Sf128)) {
    Serial.println("FAILED");
    while (1);
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range."); */

    /* Serial.print("\nSetting modem configuration: ");
    if (!driver.setModemConfig(RH_RF95::Bw31_25Cr48Sf512))
    {
        Serial.println("FAILED");
        while (1)
            ;
    }
    Serial.println("SUCCESS\nSet Config to: Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range."); */

    /* Serial.print("\nSetting modem configuration: ");
  if (!driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
    Serial.println("FAILED");
    while (1);
  }
  Serial.println("SUCCESS\nSet Config to: Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range."); */

    /*     const RH_RF95::ModemConfig myProfile = {
        RH_RF95_BW_125KHZ | RH_RF95_CODING_RATE_4_5,
        RH_RF95_SPREADING_FACTOR_128CPS};

    Serial.print("\nSetting modem configuration: ");
    driver.setModemRegisters(&myProfile);
    Serial.println("SUCCESS\n Set Config to custom"); */

    if (useReliable)
    {
        // Initiate the manager process
        Serial.print("\nInitiazing ReliableDatagram manager: ");
        if (!manager.init())
        {
            Serial.println("FAILED");
        }
        else
        {
            Serial.println("SUCCESS");
        }

        Serial.println(".. Setting timeout");
        manager.setTimeout(200); // 200ms by default, not enough for slow modes

        Serial.println(".. Setting retries");
        manager.setRetries(3);
    }
}

uint32_t lastTX = millis();

int16_t packetnum = 0; // packet counter, we increment per xmission

void loop()
{
    if (millis() - lastTX > 2500)
    {
        Serial.println("\n\n-------------------------------------");
        Serial.println("Client");
        Serial.println("-------------------------------------");

        char radiopacket[20] = "Hello World #      ";
        itoa(packetnum++, radiopacket + 13, 10);
        radiopacket[19] = 0;

        if (useReliable)
        {
            Serial.println("Client -> rf95_reliable_datagram_server");
            // Send a message to manager_server

            // SendtoWait = Send the message (with retries) and waits for an ack. Returns true if an acknowledgement is received.
            // Synchronous: any message other than the desired ACK received while waiting is discarded. Blocks until an ACK is received or all retries are exhausted

            // Turn LED on
            digitalWrite(LED_BUILTIN, HIGH);
            if (manager.sendtoWait((uint8_t *)radiopacket, 20, SERVER_ADDRESS))
            {
                Serial.println(".. ACK received");
                digitalWrite(LED_BUILTIN, LOW);
                delay(250);
                digitalWrite(LED_BUILTIN, HIGH);
                delay(100);
                digitalWrite(LED_BUILTIN, LOW);
                delay(250);
                digitalWrite(LED_BUILTIN, HIGH);
                delay(100);
                digitalWrite(LED_BUILTIN, LOW);
            }
            else
            {
                Serial.println(".. timeout waiting for ACK");
                digitalWrite(LED_BUILTIN, LOW);
            }
        }
        else
        {
            Serial.println("Client -> Server (raw packet)");
            Serial.println(radiopacket);
            // Turn LED on
            digitalWrite(LED_BUILTIN, HIGH);
            driver.send((uint8_t *)radiopacket, 20);

            Serial.print(".. Waiting for packet to complete:");
            delay(10);
            driver.waitPacketSent();
            Serial.println(" COMPLETED");
            digitalWrite(LED_BUILTIN, LOW);
        }
        lastTX = millis();
    }
}
