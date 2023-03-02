#include <driver/uart.h>
#include "antennaDelayExcel.cpp"
#include <cmath>

unsigned long lastTimestamp = millis();

//////////////Initialize all the anchors that will be used///////////////
#ifdef TYPE_TAG
  #define WIFI_ON
    #define UNIQUE_ADRESS "7D:00:22:EA:82:60:3B:9C" // (default tag) 
    static void initializeAnchors()
  {
      //Note addAnchor takes the decimal representation of the first four hex characters of the UNIQUE_ADRESS
      #ifndef Z_TEST
      addAnchor(ANCHOR_ID_1, ANCHOR_X_1, ANCHOR_Y_1);
      addAnchor(ANCHOR_ID_2, ANCHOR_X_2, ANCHOR_Y_2);
      addAnchor(ANCHOR_ID_3, ANCHOR_X_3, ANCHOR_Y_3);
      addAnchor(ANCHOR_ID_4, ANCHOR_X_4, ANCHOR_Y_4);
      //addAnchor(ANCHOR_ID_5, ANCHOR_X_5, ANCHOR_Y_5);
      //addAnchor(ANCHOR_ID_6, ANCHOR_X_6, ANCHOR_Y_6);
      CalibrationDistances();
      #endif
      #ifdef Z_TEST
      addAnchor(ANCHOR_ID_1, ANCHOR_X_1, ANCHOR_Y_1, ANCHOR_Z_1);
      addAnchor(ANCHOR_ID_2, ANCHOR_X_2, ANCHOR_Y_2, ANCHOR_Z_2);
      addAnchor(ANCHOR_ID_3, ANCHOR_X_3, ANCHOR_Y_3, ANCHOR_Z_3);
      addAnchor(ANCHOR_ID_4, ANCHOR_X_4, ANCHOR_Y_4, ANCHOR_Z_4);
      CalibrationDistances();
      #endif
    // keep adding anchors this way to your likinG
  }
#endif
 
// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin
//timing variable

void newRange()
{
  #ifdef TYPE_ANCHOR
  #ifdef I2C
  /*String range = "";
  range = range + range_counter;
  range_counter++;
  _i2c.print(range.c_str(), true);*/
  #endif
    #ifdef ANCHOR_CALIBRATION
      calibration();//calibrate the anchor when antenna delay is unknown
    #endif
    #ifndef ANCHOR_CALIBRATION
  #endif
  #endif
  // if new range is found by Tag it should store the distance in the anchorManager
  #ifdef TYPE_TAG
  /////////////////First check which menus are on//////////////////

    checkMenus();

  #endif
}

////////////////////Adding new device////////////////////////////////

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

////////////////////Delete inactive devices///////////////////////////

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

/////////////////////////////////////////////////////////////////////////

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

//////////Setup function(go to device.cpp to turn features ON/OFF////////
void setup() 
{
  Serial.begin(115200);//baud rate
  
  #ifdef I2C  //setup the ug2b lib //ug2b class is defined in i2c.cpp//
    _i2c.settings();
  #endif
  //uart_set_wakeup_threshold(UART_NUM_0, 0xFF);
  //esp_sleep_enable_uart_wakeup(ESP_SLEEP_WAKEUP_UART);
  //when wifi is desired uncomment the WIFI_EXTERN_ON in
  #ifdef WIFI_EXTERN_ON
    #ifdef TYPE_TAG
    WiFiSettingsExtern(); 
    #endif
  #endif
  
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  
  #ifdef TYPE_TAG
  #ifdef I2C
    interruptfunctions();
  #endif
    ///////////defines underneath are in Wifi.cpp///////////////////
    #ifdef WIFI_AP_ON
      WiFiSettingsAP();
    #endif

    Serial.println("\n\nTAG starting");
    DW1000.setAntennaDelay(ANTENNA_DELAY_START);//set the defined antenna delay

    #ifndef RANGETEST
    DW1000.setAntennaDelay(ANTENNA_DELAY_START);
    #endif

    //initialize all anchors
    initializeAnchors();

    //uncomment printAnchorArray(); to check if all anchors are initilized
    //printAnchorArray();

    DW1000Ranging.attachNewDevice(newDevice);

    #ifdef LOWPOWER
    DW1000Ranging.startAsTag(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    #endif

    #ifdef ACCURACY
      DW1000Ranging.startAsTag(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);
    #endif

    #ifdef I2C
  //print on display end of setup
    _i2c.print("end of tag setup!", true);
    #endif
  #endif
  #ifdef TYPE_ANCHOR
    DW1000.setAntennaDelay(ANTENNA_DELAY);
    Serial.println("\n\n\n\n\nANCHOR starting");
    DW1000Ranging.attachBlinkDevice(newBlink);
    #ifdef LOWPOWER
    DW1000Ranging.startAsAnchor(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    #endif    
    #ifdef ANCHOR_CALIBRATION
    Serial.println("calibration mode started:");
    #endif
    #ifdef ACCURACY
      DW1000Ranging.startAsAnchor(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);
    #endif
    //print on display end of setup
    #ifdef I2C
    _i2c.print(UNIQUE_ADRESS, true);
    #endif
  #endif
  #ifdef USE_RANGE_FILTERING
    DW1000Ranging.useRangeFilter(true);
  #endif
}

/////////////////////////////////////////////////////////////////////////
#ifdef TESTING_I2C
void testi2c(void)
{
  String testString = "test\tString\ti2c\n";
  delay(1000);
  _i2c.print("test\twith\ttyping..\n", false);
  delay(1000);
  _i2c.print(testString.c_str(), false);
  testString = "";
}
#endif

void loop() 
{
  #ifndef TESTING_I2C
  //when the wifi integration is complete the menu can be used
  #ifdef I2C
    checkMenuInterrupts();
  #endif
  DW1000Ranging.loop();
  #endif
  #ifdef TESTING_I2C
  testi2c();
  #endif
}


