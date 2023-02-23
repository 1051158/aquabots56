#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "buttons.cpp"
#include "calculations.cpp"


static bool end_done = false;
static bool send_done = false;


/////////////////////choose between AP or extern router(comment both to disable wifi on esp///////////////
//#define WIFI_AP_ON
#define WIFI_EXTERN_ON
//#define WIFI_TEST

static bool rdy2send = false;
static bool done_send = false;
static uint8_t syncNumber = 0;

//bool that will only let the tag measure the distances when the python program has started
static bool start_test = false;
//bool for synchronisation at start of the python program
static bool start_program = false;


#define HOTSPOT "Galaxy S20 FEA37E"
#define H_PSSWRD "cooa7104"


#define WIFI "Machelina"
#define W_PSSWRD "Donjer01"

#define ANTENNA_INTERVAL 5 //interval between 2 antenna delays

#define ANTENNA_DELAY_START 16540 //start value antenna delay
#define ANTENNA_DELAY_END 16560 //end value antenna delay

static uint16_t antenna_delay = ANTENNA_DELAY_START;

static AsyncWebServer Server(80);
static AsyncWebServer Server1(81);


static String sendCalibrationDistances()
{
  
    String total_data_cal;
    #ifdef RANGETEST
    for(uint8_t i = 0; i < MAX_RANGE_DIS; i++)
      total_data_cal = total_data_cal + range_points[i] + ';';
    #endif
    #ifdef  X_Y_Z_TEST
    if(MAX_ANCHORS >= 3)
      for(uint8_t i = 0; i < MAX_CAL_DIS; i++)
        total_data_cal = total_data_cal +  '(' + x_y_points[i][X] + ',' + x_y_points[i][Y] + ',' + x_y_points[i][Z] + ')' +';';
    #endif
    total_data_cal = total_data_cal + "\t\n"+ MAX_ANCHORS + "max" + ANTENNA_DELAY_START + 'S' + ANTENNA_DELAY_END + 'E' + ANTENNA_INTERVAL + "I";
    total_data_cal = total_data_cal + NUM_OF_SEND + "nos" + DISTANCE_COUNTER_MIN + '-' + RESET_DISTANCE_COUNTER_MAX_VALUE + "r+" + DISTANCE_COUNTER_INTERVAL + "in" + "\t\n";
    //Serial.print(total_data_cal);
    #ifdef X_Y_Z_TEST
    for(uint8_t j = 0; j < MAX_ANCHORS; j++)
    {
      //send the coordinates of the anchors and the distances at every measurepoint
        total_data_cal = total_data_cal + anchors[j].ID + 'i' + anchors[j].x + 'x' + anchors[j].y + "y" + anchors[j].z +"z\t";
        for (uint8_t i = 0; i < MAX_CAL_DIS; i++)
        {
            total_data_cal = total_data_cal + anchors[j].calibrationDistances[i] + "\t";
        }
        total_data_cal = total_data_cal + '\n'; // newline to send 
    }
    #endif
    start_test = true;
    //Serial.print(total_data_cal);
    return total_data_cal;
}

static String send_total_data_server()
{
  //////////////check how many anchors are done with measuring//////////////////////////////////////////////
  String total_data_1;
  uint8_t dataCounter = 0;
  //////////////send signal if 3 out of MAX_ANCHORS anchors are done sending/////////////////////////////////
  for(uint8_t i = 0; i<MAX_ANCHORS;i++)
  {
    if(anchors[i].done)
    {
      total_data_1 = total_data_1 + anchors[i].total_data;
      syncNumber = i;
      dataCounter++;
      if(dataCounter>=3)
        {
          total_data_1 = total_data_1 + '\n';
          for(uint8_t j = 0; j<MAX_ANCHORS;j++)
          {
            anchors[j].total_data = "";
            anchors[j].done = false;
          }
          return total_data_1;
    }
    } 
  }
    return "not";
    ////////send "end" to pythonCode to go to next worksheet an calibrate the next point in the pool//////
}

#ifdef WIFI_TEST

#endif

#ifdef WIFI_EXTERN_ON///////////////////When the ESP32 is not an acces point(AP) use this function in void setup() of main.cpp////////////////////
static void WiFiSettingsExtern(void)
{
  uint8_t wifiCounter = 0;
  //Serial.println(ssid);
  const char* ssid = HOTSPOT;
  const char* psswrd = H_PSSWRD;
  ////////////log in into the router for extern wifi connection///////////////
  WiFi.begin(ssid, psswrd);
  while (WiFi.status() != WL_CONNECTED) 
  {
    if(wifiCounter == 5)
      esp_deep_sleep(5);
    delay(1000);
    Serial.println("Connecting to WiFi..");
    wifiCounter++;
  }

  //Serial.println(WiFi.localIP());
  Server.on("/anchors", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    //variables to get the three closest anchors for x-y calculation
    bool cal_counter_bool = false;
      if(i2cMenu[3].status)
      {
        request->send(200, "text/plain", "stop");
        delay(3000);
        esp_deep_sleep_start();
      }
      if(!start_program)
        {
          for(uint8_t i = 0; i < MAX_ANCHORS; i++)
          {
            anchors[i].distance = 0;
            anchors[i].distance_counter = 0;
            anchors[i].distance_counter_max = DISTANCE_COUNTER_MIN;
            anchors[i].done = false;
          }
          start_program = true;
          request->send(200, "text/plain", "start");
          return;
        }
        if(end_done)
      {
      //button_send.pressed = false;
      for(uint8_t i = 0; i < MAX_ANCHORS; i++)
      {
        anchors[i].total_data = "";
        anchors[i].hulp_change_delay = false;
        anchors[i].done = false;
      }
      request->send(200, "text/plain", "end");
      end_done = false;
      rdy2send = false;
      return;
      }
      if(!i2cMenu[START_SEND].status)
      {
        request->send(200, "text/plain", "not");
        return;
      }
      if(rdy2send)
        {
          //x_y_cal(anchors[anchors_to_calculate[0]], anchors[anchors_to_calculate[1]], anchors[anchors_to_calculate[2]]);
          request->send(200, "text/plain", send_total_data_server().c_str()); 
          rdy2send = false;
          done_send = true;
          return;
        }
      #ifndef X_Y_TEST
        if(anchors[0].done)
        {
          String send;
          send = send_total_data_server().c_str();
          //i2cprint(send.c_str(), true);
          //Serial.println("send");
          //Serial.print(send);
          request->send(200, "text/plain", send);
          return;
        }
      #endif
    request->send(200, "text/plain", "not");
});
  Server1.on("/caldis", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    //Serial.println("send");
    String send;
    send = sendCalibrationDistances().c_str();
    //i2cprint(send.c_str(), false);
    //Serial.print(send);
    request->send(200, "text/plain", send);
  });
  //check and print IPadress to fill into the laptop
    IPAddress IP = WiFi.localIP();
    Serial.print(IP);
    Server.begin();
    Server1.begin();
  #endif
}

#ifdef WIFI_AP_ON
///////////////////When the ESP32 is an acces point(AP) use this function in void setup() of main.cpp////////////////////
static void WiFiSettingsAP(void)
{  
  const char* ssid = "ESP32-Access-Point";
  const char* psswrd = "123456789";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, psswrd);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(IP);//configure the wifi settings when 
  Server.on("/anchors", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String send;
    send = send_total_data_server().c_str();
    i2cprint(send.c_str());
    Serial.println("send");
    Serial.print(send);
    request->send(200, "text/plain", send);
  });    
Server.begin();
}
#endif
