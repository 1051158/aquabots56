#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "anchorManager.cpp"
#include "buttons.cpp"

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
  }
  else
  total_data_1 = "ignore";
  return total_data_1;
}

static void WiFiSettings(void)
{  
  const char* ssid = "ESP32-Access-Point";
  const char* psswrd = "123456789";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, psswrd);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(IP);//configure the wifi settings when 
  Server.on("/anchor1", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String send;
    send = send_total_data_server().c_str();
    i2cprint(send.c_str());
    Serial.println("send");
    Serial.print(send);
    request->send(200, "text/plain", send);
  });
  /*Server.on("/anchor3", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", send_total_data_server_3().c_str());
    if(server_bool[0],server_bool[1],server_bool[2])
    {
      server_bool[0] = false;
      server_bool[1] = false;
      server_bool[2] = false;
    }
  });*/      
Server.begin();
}