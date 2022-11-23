#include <driver/uart.h>
#include "Wifi.cpp"
#include <cmath>
#include <DW1000Ranging.h>

////////////////////////interrupt buttons////////////////////////////

bool sendAntennaInfo = true;
bool bool_nummer = false;


unsigned long lastTimestamp = millis();

//#define ANTENNA_DELAY

//#define USE_TIMER////Choose if Serial output is required and interval in microseconds of output
int outputInterval = 1000;////(comment out if not needed)

//#define DEBUG_MAIN ////Choose whether debug output should be printed


///uncomment if u want to do unit test i2c///////////////////////////////////////////////////////////////////

//#define TESTING_I2C

////Choose type of device////////////////////////////////////////////////////////////////////////////////////
#define TYPE_TAG
//#define TYPE_ANCHOR


///////////////////////to program the right anchor///////////////////////////////////////////////////////////
#ifdef TYPE_ANCHOR
    //#define ANCHOR_1
//valeus for the right anchor for the void setup() function
    #ifdef ANCHOR_1
        #define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #1
        #define UNIQUE_ADRESS "11:11:5B:D5:A9:9A:E2:9C"
    #endif
    //#define ANCHOR_2
//valeus for the right anchor for the void setup() function
    #ifdef ANCHOR_2
        #define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #2
        #define UNIQUE_ADRESS "22:22:5B:D5:A9:9A:E2:9C"
    #endif
    //#define ANCHOR_3
//valeus for the right anchor for the void setup() function
    #ifdef ANCHOR_3
        #define UNIQUE_ADRESS "33:33:5B:D5:A9:9A:E2:9C"
        #define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #3
    #endif
    #define ANCHOR_4
//valeus for the right anchor for the void setup() function
    #ifdef ANCHOR_4
        #define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #4
        #define UNIQUE_ADRESS "44:44:5B:D5:A9:9A:E2:9C"
    #endif
#endif

////Choose measure mode
#define LOWPOWER
//#define ACCURACY

//#define USE_RANGE_FILTERING ////Enable the filter to smooth the distance (default off)


#ifdef TYPE_TAG
  #define WIFI_ON
    #define UNIQUE_ADRESS "7D:00:22:EA:82:60:3B:9C" // (default tag) 
    static void initializeAnchors()
  {
      //Note addAnchor takes the decimal representation of the first four hex characters of the UNIQUE_ADRESS
      addAnchor(ANCHOR_ID_1, ANCHOR_X_1, ANCHOR_Y_1);
      addAnchor(ANCHOR_ID_2, ANCHOR_X_2, ANCHOR_Y_2);
      addAnchor(ANCHOR_ID_3, ANCHOR_X_3, ANCHOR_Y_3);
      addAnchor(ANCHOR_ID_4, ANCHOR_X_4, ANCHOR_Y_4);
      CalibrationDistances();
    // keep adding anchors this way to your likinG
  }
#endif

/////////////////////////////End of configuration////////////////////////////////////////////

//define pins
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4
 

uint8_t pos = 1;

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

// timing variable

void newRange()
{
  //Serial.print('2');
    #ifdef TYPE_ANCHOR
      #ifdef ANCHOR_CALIBRATION
        calibration();//calibrate the anchor when antenna delay is unknown
      #endif
      #ifndef ANCHOR_CALIBRATION
//    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
    //Serial.print(DW1000Ranging.getDistantDevice()->getRange());
      //Serial.print("from: ");
      //Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
      //Serial.print("Range: ");
      //Serial.println(DW1000Ranging.getDistantDevice()->getRange());
      //Serial.print(" m");
      //Serial.print("\t RX power: ");
      //Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
      //Serial.println(" dBm");
      #endif
    #endif
    // if new range is found by Tag it should store the distance in the anchorManager
    #ifdef TYPE_TAG

      if(button_end.pressed)
      {
        #ifdef DEBUG_INTERRUPT
          Serial.print('E');
          i2cprint("E");
        #endif
        for(uint8_t i = 0; i < MAX_ANCHORS; i++)
        {
          anchors[i].total_data = "stop";
        }
        _i2c.print("program ended", true);
        while(1)
        {}
      }
      if(button_send.pressed && !button_backspace.pressed)//press the interrupt button to start measurement
      {/*
        for(uint8_t i = 0; i < MAX_ANCHORS; i++)
        {
          anchors[i].total_time = millis();
          anchors[i].total_time_1 = anchors[i].total_time;
        }*/
        change_delay_counter = 0;
        if(!start_test)
        {
          _i2c.print("not connected", true);
          button_send.pressed = false;
        }
        #ifdef DEBUG_INTERRUPT
          Serial.print('S');
          _i2c.print("S");
        #endif
        #ifndef DEBUG_INTERRPUT
        //Serial.print('3');
        for(uint8_t i = 0; i < MAX_ANCHORS;i++)
        {
//when ID's match and the distance hasn't been created in the pas yet go further into funtion
              if(anchors[i].ID == DW1000Ranging.getDistantDevice()->getShortAddress() && !anchors[i].done)
//start with renewing http if the last http of the defined anchor array hasn't been made yet
              {
                setDistanceIfRegisterdAnchor(DW1000Ranging.getDistantDevice()->getShortAddress(),DW1000Ranging.getDistantDevice()->getRange()); 
                //Serial.print('4');
                #ifdef RANGETEST
              anchors[i].total_data = updateDataWiFi(i);//send data through wifi to wifi-tag(i stands for the anchor number)
              if(anchors[i].total_data == "not")
              {
                //Serial.print('n');
                anchors[i].total_data = "";
              }
              }
              //Serial.print("main: ");
              //Serial.print(i);
              //Serial.println(anchors[i].total_data);
              //distance is determined waiting for the distances of the other anchors in the array
              if(anchors[i].hulp_change_delay)
                change_delay_counter++;
            }
            Serial.print(change_delay_counter);
              if(change_delay_counter >= MAX_ANCHORS)
                {
                  antenna_delay += ANTENNA_INTERVAL;
                  String AD = "";
                  AD = AD + antenna_delay;
                  _i2c.print(AD.c_str(), true);
                  //Serial.println(anchors[i].antenna_delay);
                  DW1000.setAntennaDelay(antenna_delay);
                  if(antenna_delay > ANTENNA_DELAY_END)
                  {
                    ///Reset all the anchors for the measurement at the new coördinates//////////////
                    antenna_delay = ANTENNA_DELAY_START;
                    for(int j = 0; j< MAX_ANCHORS; j++)
                    {
                      anchors[j].total_data = "end";
                      anchors[j].num_of_send_counter = 0;
                      anchors[j].distance_counter_max = DISTANCE_COUNTER_MIN;
                      anchors[j].done = true;
                      anchors[j].distance = 0;
                      anchors[j].distance_counter = 0;
                      button_send.pressed = false;
                    }
                    //delay(200);
                  }   
                  for(uint8_t i = 0; i<MAX_ANCHORS;i++)
                  {
                      anchors[i].hulp_change_delay = false;                    
                  }
              }
            }
            #endif
            #endif
          if(button_backspace.pressed)
          {
            for(uint8_t i = 0; i < MAX_ANCHORS; i++)
              {
                anchors[i].total_data = "back";
                anchors[i].done = true;
                pos--;
              }
            #ifdef DEBUG_INTERRUPT
              Serial.print('B');
              i2cprint("B");
            #endif
            button_backspace.pressed = false;
          }
        //average_counter++;
          //#ifdef USE_SERIAL
            //outputDataJson();
            //average_counter = 0;
      #endif
        #ifdef USE_TIMER
          if(millis() - lastTimestamp > outputInterval)
          { 
            outputDataJson();
            lastTimestamp = millis();
          }

      #endif
    }

/////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
void setup() 
{
  Serial.begin(115200);//baud rate

  #ifdef I2C  //setup the ug2b lib //ug2b class is defined in i2c.cpp//
    _i2c.settings();
  #endif

  //uart_set_wakeup_threshold(UART_NUM_0, 0xFF);
  //esp_sleep_enable_uart_wakeup(ESP_SLEEP_WAKEUP_UART);
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
    interruptfunctions();
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
    _i2c.print("end of anchor setup!", true);
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
  DW1000Ranging.loop();
  #endif
  #ifdef TESTING_I2C
  testi2c();
  #endif
}


