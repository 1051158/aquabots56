#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "buttons.cpp"
#include "calibration.cpp"
#include "i2c.cpp"

/////////////////////choose between AP or extern router(comment both to disable wifi on esp///////////////
//#define WIFI_AP_ON
#define WIFI_EXTERN_ON
//#define WIFI_TEST

#define SSID "Machelina"
#define PSSWRD "Donjer01"

static AsyncWebServer Server(80);
static AsyncWebServer Server1(81);


static String sendCalibrationDistances()
{
    String total_data_cal;
    for(uint8_t i = 0; i < MAX_CAL_DIS; i++)
      total_data_cal = total_data_cal +  '(' + x_y_points[i][X] + ',' + x_y_points[i][Y] + ')' + ';';
    total_data_cal = total_data_cal + "\t\n" + ANTENNA_DELAY_START + 'S' + ANTENNA_DELAY_END + 'E' + ANTENNA_INTERVAL + "I\t\n";
    //Serial.print(total_data_cal);
    for(uint8_t j = 0; j < MAX_ANCHORS; j++)
    {
        total_data_cal = total_data_cal + anchors[j].ID + 'i' + anchors[j].x + 'x' + anchors[j].y + "y\t";
        for (uint8_t i = 0; i < MAX_CAL_DIS; i++)
        {
            total_data_cal = total_data_cal + anchors[j].calibrationDistances[i] + "\t";
        }
        total_data_cal = total_data_cal + '\n'; // newline to send 
    }
    Serial.print(total_data_cal);
    return total_data_cal;
}

static String send_total_data_server()
{
  uint8_t send_count = 0;
  String total_data_1; // use hulp string to store the array of strings of every anchor in

  //////////////check how many anchors are done with measuring//////////////////////////////////////////////

  //////////////send signal if 3 out of MAX_ANCHORS anchors are done sending/////////////////////////////////
  for(int i = 0; i<MAX_ANCHORS;i++)
  {
    total_data_1 = total_data_1 + anchors[i].total_data;
    anchors[i].done = false;
    ////////send "end" to pythonCode to go to next worksheet an calibrate the next point in the pool//////
    if(anchors[i].total_data == "end")
    {
      button_send.pressed = false;
      anchors[i].total_data = "";
      anchors[i].done = false;
    }
  }
  total_data_1 = total_data_1 + '\n';
  return total_data_1;
}

#ifdef WIFI_TEST

#endif

#ifdef WIFI_EXTERN_ON///////////////////When the ESP32 is not an acces point(AP) use this function in void setup() of main.cpp////////////////////
static void WiFiSettingsExtern(void)
{
  uint8_t wifiCounter = 0;
  const char* ssid = SSID;
  const char* psswrd = PSSWRD;
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
    #ifdef WIFI_TEST
      request->send(200, "text/plain", "werkt");
    #endif
    #ifndef WIFI_TEST
    if(anchors[0].done && anchors[1].done && anchors[2].done)
      {
      String send;
      send = send_total_data_server().c_str();
      //i2cprint(send.c_str(), true);
      //Serial.println("send");
      //Serial.print(send);
      request->send(200, "text/plain", send);
      }
    else
      //Serial.print("not");
      request->send(200, "text/plain", "not");
    #endif
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
    IPAddress IP = WiFi.localIP();
    Serial.print(IP);
    Server.begin();
    Server1.begin();
}
#endif

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
