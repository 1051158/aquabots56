#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>
#include "DW1000Ranging.h"
#include "anchorManager.cpp"
#include <U8g2lib.h>



///////////////////////////Device configuration/////////////////////////////////////

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 

////Choose whether debug output should be printed
//#define DEBUG_MAIN

#define AVERAGE_COUNT_MAX 35

////Choose if Serial output is required and interval in microseconds of output
////(comment out if not needed)
//#define USE_SERIAL
//int outputInterval = 1000;


////Choose type of device

#define TYPE_ANCHOR

#define I2C_MODULE

//#define TYPE_TAG

#ifdef TYPE_ANCHOR
// choose which anchor u want to flash/calibrate keep one uncommented even if u use tag for compile errors
#define ANCHOR_1
//#define ANCHOR_2
//#define ANCHOR_3
//#define ANCHOR_4
double average_distance = 0;
uint8_t average_counter = 0;
#endif

////Enable the filter to smooth the distance (default off)
//#define USE_RANGE_FILTERING

////Set unique device adress (first four digit cannot all be 0!)

//#define UNIQUE_ADRESS "83:17:5B:D5:A9:9A:E2:9C" // (default anchor)

#ifdef TYPE_TAG
  static void initializeAnchors(){
    // define all anchors this tag should consider


    // system supports up to 6 anchors by default,
    // change MAX_ANCHORS in anchorManager.cpp if more are needed
    // addAnchor takes 3 parameters:
    //    ID(int) (takes the HEX first four characters of the UNIQUE_ADRESS)
    //    X(double) coordinate in meters
    //    Y(double) coordinate in meters

    //Note addAnchor takes the decimal representation of the first four hex characters of the UNIQUE_ADRESS
    addAnchor(4369, 0.0, 0.0);
    addAnchor(8738, 10.0, 0.0);
    addAnchor(13107, 0.0, 10.0);
    addAnchor(17476, 10.0, 10.0);
    // keep adding anchors this way to your liking
  }
#endif

/////////////////////////////End of configuration////////////////////////////////////////////

//define pins
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4
#define SCL 21
#define SDA 22


// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

// timing variable
unsigned long lastTimestamp = millis();

void newRange()
{
    #ifdef TYPE_ANCHOR
    average_distance += DW1000Ranging.getDistantDevice()->getRange();
    average_counter++;
      //Serial.print("from: ");
      //Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
      //Serial.print("Range: ");
      //Serial.println(DW1000Ranging.getDistantDevice()->getRange());
      //Serial.print(" m");
      //Serial.print("\t RX power: ");
      //Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
      //Serial.println(" dBm");
      if(average_counter >= AVERAGE_COUNT_MAX)
      {
        average_distance /= AVERAGE_COUNT_MAX;
        #ifndef I2C_MODULE
        Serial.print("distance");
        Serial.println(average_distance);
        #endif
        #ifdef I2C_MODULE
        u8g2.print("distance");
        u8g2.print(average_distance);
        #endif
        average_counter = 0;
        average_distance = 0.00;
      }
    #endif

    // if new range is found by Tag it should store the distance in the anchorManager
    #ifdef TYPE_TAG
      setDistanceIfRegisterdAnchor(
        DW1000Ranging.getDistantDevice()->getShortAddress(),
        DW1000Ranging.getDistantDevice()->getRange(),
        DW1000Ranging.getDistantDevice()->getRXPower()
      );


      #ifdef USE_SERIAL
        if(millis() - lastTimestamp > outputInterval){
          outputDataJson();
          lastTimestamp = millis();
        }
      #endif
    #endif


}

void newDevice(DW1000Device *device)
{
    #ifdef DEBUG_MAIN
      Serial.print("ranging init; 1 device added ! -> ");
      Serial.print(" short:");
      Serial.println(device->getShortAddress(), HEX);
    #endif

    // TODO log activeness in manager
    #ifdef TYPE_TAG
      setAnchorActive(device->getShortAddress(), true);
    #endif
}

void inactiveDevice(DW1000Device *device)
{
    #ifdef TYPE_ANCHOR
      Serial.print("delete inactive device: ");
      Serial.println(device->getShortAddress(), HEX);
    #endif

    #ifdef TYPE_TAG
      setAnchorActive(device->getShortAddress(), false);
    #endif
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
  #ifdef I2C_MODULE
  u8g2.begin();
  u8g2.enableUTF8Print();
  #endif

  //init the configuration
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  #ifdef TYPE_TAG
  #define UNIQUE_ADRESS "7D:00:22:EA:82:60:3B:9C" // (default tag) 
    Serial.println("\n\nTAG starting");

    //initialize all anchors
    initializeAnchors();
    printAnchorArray();

    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.startAsTag(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
  #endif
  #ifdef TYPE_ANCHOR
   #ifdef ANCHOR_1
  #define UNIQUE_ADRESS "11:11:5B:D5:A9:9A:E2:9C"
#endif

#ifdef ANCHOR_2
#define UNIQUE_ADRESS "22:22:5B:D5:A9:9A:E2:9C"
#endif

#ifdef ANCHOR_3
#define UNIQUE_ADRESS "33:33:5B:D5:A9:9A:E2:9C"
#endif

#ifdef ANCHOR_4
#define UNIQUE_ADRESS "44:44:5B:D5:A9:9A:E2:9C"
#endif
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
