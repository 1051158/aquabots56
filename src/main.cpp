#include "anchorManager.cpp"
#include "calibration.cpp"
#include "Wifi.cpp"
#include <driver/uart.h>

struct button
{
  const uint8_t interrupt_pin;
  bool pressed;
};
////////////////////////interrupt buttons////////////////////////////

bool sendAntennaInfo = true;
bool bool_nummer = false;

#define ANTENNA_DELAY

#ifdef ANTENNA_DELAY
  #define NOV 14
  uint16_t antenna_delay = 16500;
  uint16_t antenna_delay_start = 16500;
  uint16_t antenna_delay_end = 16650;
  uint8_t antenna_interval = 10;//will be sended as ain(to prevent confusion of the meaning of AI which is not used here)
  uint8_t number = 0;
#endif

button button_send = {16,false};//interrupt button to send data to pyhonscript
button button_end = {5,false};//interrupt to stop python script
button button_backspace{17, false};//interrupt to skip latest uart value

////////////////////////millis() variables//////////////////////////
unsigned long lastTimestamp = millis();
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

static void IRAM_ATTR backCode(void)//interruptfunction to go 1 row up in excel
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    button_backspace.pressed = true;
    last_button_time = button_time;
  }
}

static void IRAM_ATTR refresh(void)//interrupt code to refresh the http page
{
    button_time = millis();
if (button_time - last_button_time > 250)
{
    button_send.pressed = true;
    last_button_time = button_time;
}
}

static void IRAM_ATTR endCode(void)//interrupt function to stop the code of the tag
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    button_end.pressed = true;
  }
}

//#define USE_TIMER////Choose if Serial output is required and interval in microseconds of output
int outputInterval = 1000;////(comment out if not needed)

//#define DEBUG_MAIN ////Choose whether debug output should be printed

////Choose type of device
#define TYPE_TAG
//#define TYPE_ANCHOR

////Choose measure mode
#define LOWPOWER
//#define ACCURACY

//#define USE_RANGE_FILTERING ////Enable the filter to smooth the distance (default off)

////////////////////////anchor antenna_delays//////////////////////////////////////////
#ifdef TYPE_ANCHOR
//#define ANCHOR_CALIBRATION //choose to measure(comment) or calibrate the anchor(uncomment)
//choose which anchor u want to flash
#define ANCHOR_1
//#define ANCHOR_2
//#define ANCHOR_3
//#define ANCHOR_4
//value calibrated antenna delays for the anchors and adresses to get detected by tag
#ifdef ANCHOR_1
#define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #1
#define UNIQUE_ADRESS "11:11:5B:D5:A9:9A:E2:9C"
#endif
#ifdef ANCHOR_2
#define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #2
#define UNIQUE_ADRESS "22:22:5B:D5:A9:9A:E2:9C"
#endif
#ifdef ANCHOR_3
#define UNIQUE_ADRESS "33:33:5B:D5:A9:9A:E2:9C"
#define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #3
#endif
#ifdef ANCHOR_4
#define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #4
#define UNIQUE_ADRESS "44:44:5B:D5:A9:9A:E2:9C"
#endif
// choose which anchor u want to flash/calibrate keep one uncommented even if u use tag for compile errors
#endif

#ifdef TYPE_TAG
  #define WIFI_ON
    #define UNIQUE_ADRESS "7D:00:22:EA:82:60:3B:9C" // (default tag) 
    static void initializeAnchors(){
      //Note addAnchor takes the decimal representation of the first four hex characters of the UNIQUE_ADRESS
      addAnchor(4369, 2, 0);
      addAnchor(8738, 0, 7);
      addAnchor(13107, 2, 7);
      //addAnchor(17476, 10.0, 10.0);
    // keep adding anchors this way to your likinG
  }
#endif

/////////////////////////////End of configuration////////////////////////////////////////////

//define pins
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4
 
unsigned long rangetime = millis();


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
      setDistanceIfRegisterdAnchor(DW1000Ranging.getDistantDevice()->getShortAddress(),DW1000Ranging.getDistantDevice()->getRange()); 

      if(button_end.pressed)
      {
        Serial.print("end");
        esp_deep_sleep_start();
      }
      if(button_send.pressed)//press the interrupt button to start measurement
      {
        for(uint8_t i = 0; i<MAX_ANCHORS;i++)
        {
          if(anchors[i].active)
            {
              if(hulp_send_bool)//Press again to start the measurement of range or x_y
              {
                outputDataJson();
                button_send.pressed = false;
                hulp_send_bool = false;
                break;
              }
              #ifdef RANGETEST
              if(hulp_change_delay)
              {
                antenna_delay += antenna_interval;
                DW1000.setAntennaDelay(antenna_delay);
                if(antenna_delay >= antenna_delay_end)
                {
                  antenna_delay = antenna_delay_start;
                  hulp_send_bool = true;
                  total_data = "end";
                  num_of_send_counter = 0;
                  distance_counter_max = 2;
                  delay(200);
                }
                hulp_change_delay = false;
                //outputDataJson();//send data to i2c or uart
              }
              total_data = updateDataWiFi();//send data through wifi to wifi-tag
              //Serial.print("main: ");
              Serial.println(total_data);
              break;
            }
          }
          #endif
        if(button_backspace.pressed)
        {
          Serial.print("back");
          button_backspace.pressed = false;
        }
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

////////////////////////////////////////////////////////////////////////

String send_total_data(void)
{
  String total_data1 = total_data;
  total_data = "";
  return total_data1;
}

/////////////////////////////////////////////////////////////////////////

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

/////////////////////////////////////////////////////////////////////////
void interruptfunctions(void)
{
  pinMode(button_send.interrupt_pin, INPUT_PULLUP);//enable interrupt to send data when green button is pressed
  attachInterrupt(button_send.interrupt_pin, refresh, FALLING);
  pinMode(button_backspace.interrupt_pin, INPUT_PULLUP);//enable interrupt to use 'backspace' in python
  attachInterrupt(button_backspace.interrupt_pin, backCode, FALLING);
  pinMode(button_end.interrupt_pin, INPUT_PULLUP);//enable interrupt to end code when red button is pressed
  attachInterrupt(button_end.interrupt_pin, endCode, FALLING);
}
//////////////////////////////////////////////////////////////////////////
void setup() 
{
  #ifdef I2C  //setup the ug2b lib //ug2b class is defined in anchormanager.cpp//
    u8g2.begin();
    u8g2.setFontPosTop();
    //init the configuration
  #endif
  //uart_set_wakeup_threshold(UART_NUM_0, 0xFF);
  //esp_sleep_enable_uart_wakeup(ESP_SLEEP_WAKEUP_UART);
  Serial.begin(115200);//baud rate
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  #ifdef TYPE_TAG
    interruptfunctions();
    #ifdef WIFI_ON
      const char* ssid = "ESP32-Access-Point";
      const char* psswrd = "123456789";
      WiFi.mode(WIFI_AP);
      WiFi.softAP(ssid, psswrd);
      IPAddress IP = WiFi.softAPIP();
      //Serial.print(IP);//configure the wifi settings when 
      Server.on("/anchor1", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        request->send(200, "text/plain", send_total_data().c_str());
      });
      /*Server.on("/anchor2", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        request->send(200, "text/plain", send_total_data().c_str());
      });*/
      Server.begin();
    #endif
    Serial.println("\n\nTAG starting");
    uint16_t antenna_delay = antenna_delay_start;
    DW1000.setAntennaDelay(antenna_delay);//set the defined antenna delay
    Serial.print(number);
    #ifndef RANGETEST
    DW1000.setAntennaDelay(ANTENNA_DELAY);
    #endif
    //initialize all anchors
    initializeAnchors();
    //printAnchorArray();//uncomment to check if all anchors are initilized
    DW1000Ranging.attachNewDevice(newDevice);
    #ifdef LOWPOWER
    DW1000Ranging.startAsTag(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    #endif
    #ifdef ACCURACY
      DW1000Ranging.startAsTag(UNIQUE_ADRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);
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
  #endif

  #ifdef USE_RANGE_FILTERING
    DW1000Ranging.useRangeFilter(true);
  #endif
}

/////////////////////////////////////////////////////////////////////////

void loop() 
{
  DW1000Ranging.loop();
}
