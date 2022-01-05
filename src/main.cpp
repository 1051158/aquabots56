#include <Arduino.h>
#include <SPI.h>
#include "DW1000Ranging.h"

///////////////////////////Device configuration/////////////////////////////////////

////Choose type of device
//#define TYPE_ANCHOR
#define TYPE_TAG

////Enable the filter to smooth the distance (default off)
//#define USE_RANGE_FILTERING

////Set unique device adress
#define UNIQUE_ADRESS "66:66:5B:D5:A9:9A:E2:9C"
//#define UNIQUE_ADRESS "83:17:5B:D5:A9:9A:E2:9C" // (default anchor)
//#define UNIQUE_ADRESS "7D:00:22:EA:82:60:3B:9C" // (default tag)


/////////////////////////////End of configuration////////////////////////////////////////////

//define pins
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

void newRange()
{
    Serial.print("from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m");
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newDevice(DW1000Device *device)
{
    Serial.print("ranging init; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}


void setup() {
  Serial.begin(115200);
  delay(3000);

  //init the configuration
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachInactiveDevice(inactiveDevice); 

  #ifdef TYPE_TAG
    Serial.println("\n\n\n\n\nTAG starting");
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.startAsTag(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
  #endif
  #ifdef TYPE_ANCHOR
    Serial.println("\n\n\n\n\nANCHOR starting");
    DW1000Ranging.attachBlinkDevice(newBlink);
    DW1000Ranging.startAsAnchor(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
  #endif
  
  #ifdef USE_RANGE_FILTERING
    DW1000Ranging.useRangeFilter(true);
  #endif

}

void loop() {
  DW1000Ranging.loop();
}