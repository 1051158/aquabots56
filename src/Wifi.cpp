#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "buttons.cpp"

//bools to send different string through WiFi
static bool end_done = false;
static bool send_done = false;
//bools to verify the string in the functions send_total_data_server() and
static bool rdy2send = false;
static bool done_send = false;

static bool start_test = false;
static bool start_program = false;

static uint8_t syncNumber = 0;

static AsyncWebServer Server(80);

//belang to "IP:80/caldis" link
static String sendCalibrationDistances()
{
    String total_data_cal;
    #ifdef RANGETEST
    for(uint8_t i = 0; i < MAX_RANGE_DIS; i++)
      total_data_cal = total_data_cal + range_points[i] + ';';
    #endif
    #ifdef  X_Y_TEST
    if(MAX_ANCHORS >= 3)
      for(uint8_t i = 0; i < MAX_CAL_DIS; i++)
        {
        total_data_cal = total_data_cal +  '(' + x_y_points[i][X] + ',' + x_y_points[i][Y];
        #ifdef Z_TEST
        total_data_cal = total_data_cal + ',' + x_y_points[i][Z];
        #endif
        total_data_cal = total_data_cal + ");";
        }
    #endif
    total_data_cal = total_data_cal + "\t\n"+ MAX_ANCHORS + "max" + ANTENNA_DELAY_START + 'S' + ANTENNA_DELAY_END + 'E' + ANTENNA_INTERVAL + "I";
    total_data_cal = total_data_cal + NUM_OF_SEND + "nos" + DISTANCE_COUNTER_MIN + '-' + RESET_DISTANCE_COUNTER_MAX_VALUE + "r+" + DISTANCE_COUNTER_INTERVAL + "in" + "\t\n";
    //Serial.print(total_data_cal);
    #ifdef X_Y_TEST
    for(uint8_t j = 0; j < MAX_ANCHORS; j++)
    {
      //send the coordinates of the anchors and the distances at every measurepoint
        total_data_cal = total_data_cal + j + 'i' + anchors[j].x + 'x' + anchors[j].y + "y";
        #ifdef Z_TEST
        total_data_cal = total_data_cal + anchors[j].z + "z\t";
  #endif
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

//belong to "/anchors" server
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
      //store the data of every anchor in one total string to send 
      syncNumber = i;
      dataCounter++;
      if(dataCounter>=3)
        {
          for(uint8_t i = 0; i < MAX_ANCHORS; i++)
            total_data_1 = total_data_1 + anchors[i].total_data;

          total_data_1 = total_data_1 + '\n';
          rdy2send = false;
          dataCounter = 0;

          return total_data_1;
        }
    } 
  }
    functionName = "dC" + dataCounter;
    return functionName;
    ////////send "end" to pythonCode to go to next worksheet an calibrate the next point in the pool//////
  }

#ifdef WIFI_EXTERN_ON///////////////////When the ESP32 is not an acces point(AP) use this function in void setup() of main.cpp////////////////////
static void WiFiSettingsExtern(void)
{
  uint8_t wifiCounter = 0;
  //Serial.println(ssid);
  #ifdef WIFI
  const char* ssid = "Machelina";
  const char* psswrd = "Donjer01";
  #endif
  #ifdef HOTSPOT
  const char* ssid = "Galaxy S20 FEA37E";
  const char* psswrd = "cooa7104";
  #endif
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
      if(end_done)
      {
        resetAnchors();
        request->send(200, "text/plain", "end");
        end_done = false;
        rdy2send = false;
        return;
      }
      if(!i2cMenu[START_SEND].status)
      {
        resetAnchors();
        rdy2send = false;
        request->send(200, "text/plain", "not");
        return;
      }
      if(rdy2send)
      {
        //x_y_cal(anchors[anchors_to_calculate[0]], anchors[anchors_to_calculate[1]], anchors[anchors_to_calculate[2]]);
        request->send(200, "text/plain", send_total_data_server().c_str());
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
    request->send(200, "text/plain", functionName.c_str());
});

Server.on("/addAD", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  _addAD = true;
});

Server.on("/resetAD", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  _resetAD = true;
}
);
//server to reset and start measurements from tag
Server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  resetAnchors();
  start_program = true;
});
//server interrupt function to reset DCM
Server.on("/resetDCM", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  _resetDCM = true;
} 
);

Server.on("/addDCM", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  _addDCM = true;
}
);

  Server.on("/caldis", HTTP_GET, [](AsyncWebServerRequest *request)
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
