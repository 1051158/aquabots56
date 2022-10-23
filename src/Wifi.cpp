#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "anchorManager.cpp"
#include "buttons.cpp"

/////////////////////choose between AP or extern router(comment both to disable wifi on esp///////////////
//#define WIFI_AP_ON
#define WIFI_EXTERN_ON
//#define WIFI_TEST

#define SSID "Machelina"
#define PSSWRD "Donjer01"

static AsyncWebServer Server(80);

static String send_total_data_server()
{
  String total_data_1; // use hulp string to store the array of strings of every anchor in
  if(anchors[0].done && anchors[1].done && anchors[2].done)
  {
    for(int i = 0; i<MAX_ANCHORS;i++)
    {
      total_data_1 = total_data_1 + anchors[i].total_data;
      anchors[i].done = false;
      if(anchors[i].total_data == "end")
      {
        button_send.pressed = false;
        anchors[i].total_data = "";
        anchors[i].done = false;
      }
    }
    total_data_1 = total_data_1 + '\n';
  }
  else
  {
    total_data_1 = "ignore";
  }
  return total_data_1;
}

#ifdef WIFI_TEST

#endif

#ifdef WIFI_EXTERN_ON///////////////////When the ESP32 is not an acces point(AP) use this function in void setup() of main.cpp////////////////////
static void WiFiSettingsExtern(void)
{
  const char* ssid = SSID;
  const char* psswrd = PSSWRD;
  ////////////log in into the router for extern wifi connection///////////////
  WiFi.begin(ssid, psswrd);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
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
      i2cprint(send.c_str(), true);
      Serial.println("send");
      Serial.print(send);
      request->send(200, "text/plain", send);
      }
    else
      request->send(200, "text/plain", "not");
    #endif
    });
    IPAddress IP = WiFi.localIP();
    Serial.print(IP);
    Server.begin();
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
