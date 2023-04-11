#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "buttons.cpp"

#ifdef TYPE_TAG

//bools to verify the string in the functions send_total_data_server() and
static bool rdy2send = false;
static bool done_send = false;

static bool start_test = false;

static AsyncWebServer Server(80);
static AsyncWebServer Server1(81);
static AsyncWebServer Server2(82);
static AsyncWebServer Server3(83);

//belong to "IP:80/caldis" link
static String makeCaldisPackage()
{
    String total_data_cal;
    #ifdef RANGETEST
    for(uint8_t i = 0; i < MAX_RANGE_DIS; i++)
      total_data_cal = total_data_cal + range_points[i] + ';';
    #endif
    #ifdef  X_Y_TEST
    if(MAX_ANCHORS >= 2)
      for(uint8_t i = 0; i < MAX_CAL_DIS; i++)
        {
        total_data_cal = total_data_cal +  '(' + x_y_points[i][X] + ',' + x_y_points[i][Y];
        #ifdef Z_TEST
        total_data_cal = total_data_cal + ',' + x_y_points[i][Z];
        #endif
        total_data_cal = total_data_cal + ");";
        }
      else
      {
        total_data_cal =  "wrong anchornumber in tag program";
        return total_data_cal;
      }
    #endif
    total_data_cal = total_data_cal + "\t\n"+ MAX_ANCHORS + "max" + ANTENNA_DELAY_START + 'S' + ANTENNA_DELAY_END + 'E' + ANTENNA_INTERVAL + "I" + "\t\n";
    #ifdef PYTHON_CONTROL
    total_data_cal = total_data_cal + NUM_OF_SEND + "nos" + DISTANCE_COUNTER_MIN + '-' + RESET_DISTANCE_COUNTER_MAX_VALUE + "r+" + DISTANCE_COUNTER_INTERVAL + "in" + "\t\n";
    #endif
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
    return total_data_cal;
}

//belong to "/anchors" server
static String send_total_data_server()
{
  //////////////check how many anchors are done with measuring//////////////////////////////////////////////
  String total_data_1;
  uint8_t dataCounter = 0;
  //////////////send signal if 3 out of MAX_ANCHORS anchors are done sending/////////////////////////////////
  for(uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    if(anchors[i].done)
    {
      total_data_1 = total_data_1 + anchors[i].total_data + i + "ID" + anchors[i].distance + 'd' + anchors[i].sendTime + "ms" + "\t";
      //store the data of every anchor in one total string to send 
      dataCounter++;
    }
      if(dataCounter >= 3)
      {          
        total_data_1 = total_data_1 + '\n';
        return total_data_1;
      }
  }
  //Send the value of the anchors that sended the data if debug is necessary
  total_data_1 = total_data_1 + "dC" + dataCounter;
  return total_data_1;
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
  const char* ssid = "TP-LINK_D712";
  const char* psswrd = "05428717";
  #endif
  ////////////log in into the router for extern wifi connection//////////////
  WiFi.begin(ssid, psswrd);
  while (WiFi.status() != WL_CONNECTED) 
  {
    if(wifiCounter == 5)
      esp_restart();
    delay(1000);
    #ifdef SERIAL_DEBUG
      Serial.println("Connecting to WiFi..");
    #endif

    wifiCounter++;
  }

  Server.on("/anchors", HTTP_GET, [](AsyncWebServerRequest *request)
  {

    //check if tag has been shutted off(through i2c menu) and send that to python so the excel sheet will be saved
      /*if(i2cMenu[END_CODE].status)
      {
        request->send(200, "text/plain", "stop");
        delay(3000);
        esp_deep_sleep_start();
      }*/
      if(_resetAnchors)
      {
        request->send(200, "text/plain", "2rst");
        return;
      }
      if(rdy2send)
      {
        //x_y_cal(anchors[anchors_to_calculate[0]], anchors[anchors_to_calculate[1]], anchors[anchors_to_calculate[2]]);
        request->send(200, "text/plain", send_total_data_server().c_str());
        rdy2send = false;
        return;
      }
      #ifndef X_Y_TEST
        if(anchors[0].done)
        {
          String send;
          send = send_total_data_server().c_str();
          request->send(200, "text/plain", send);
          return;
        }
      #endif
      String functionName;
      functionName = functionNumber;
      request->send(200, "text/plain", functionName.c_str());
});

#ifdef SERVER_CONTROLLER
//addAD when the python script has reached its DCM
Server1.on("/addAD", HTTP_GET, [](AsyncWebServerRequest *request)
{
  uint16_t AD_2_send = antenna_delay;
  String AD_send = "";
  AD_send = AD_send + AD_2_send;
  request->send(200, "text/plain", AD_send);
  _addAD = true;
});

//resetAD when the excel file has reached its max of the AD_test
Server1.on("/resetAD", HTTP_GET, [](AsyncWebServerRequest *request)
{
  uint16_t AD_2_send = antenna_delay;
  String AD_send = "";
  AD_send = AD_send + AD_2_send;
  request->send(200, "text/plain", AD_send);
  _resetAD = true;
}
);

//subADD when the python script and VSC are not synchronized
Server1.on("/subAD", HTTP_GET, [](AsyncWebServerRequest *request)
{
  uint16_t AD_2_send = antenna_delay -= ANTENNA_INTERVAL;
  String AD_send = "";
  AD_send = AD_send + AD_2_send;
  request->send(200, "text/plain", AD_send);
  _resetAD = true;
}
);
//server to reset and start measurements from tag

//server interrupt function to reset DCM
Server2.on("/resetDCM", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  _resetDCM = true;
} );
//server interrupt function to add to average counter
Server2.on("/addDCM", HTTP_GET, [](AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", "OK");
  _addDCM = true;
});

#endif

  Server.on("/caldis", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String send;
    send = makeCaldisPackage().c_str();
    request->send(200, "text/plain", send);
  });
  //to control the python code with the interruptbuttons
  Server3.on("/sendMenu", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    //when the button is pressed and the pythoncode has all measurements send the arraynumber of START_SEND
    if(i2cMenu[START_SEND].status)
    {  
      request->send(200, "text/plain", "0");
      i2cMenu[START_SEND].status = false;
      resetAnchors();
      _resetAnchors = true;
    }
    //when the interrupt in tag has been detected by python
    if(i2cMenu[BACKSPACE].status)
    {  
      request->send(200, "text/plain", "1");
      i2cMenu[BACKSPACE].status = false;
      resetAnchors();
      _resetAnchors = true;
    }
    if(i2cMenu[EXCEL_MODE].status)
    {  
      request->send(200, "text/plain", "2");
      i2cMenu[EXCEL_MODE].status = false;
    }
    //when this button is pressed and the pythoncode has all measurements send the arraynumber of END_CODE
    if(i2cMenu[END_CODE].status)
    {
      request->send(200, "text/plain", "3");
      esp_restart();
    }
    request->send(200, "text/plain", "n");
  });
Server3.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain", "Restart");
    delay(1000);
    esp_restart();
  });

  //check and print IPadress to fill into the laptop
    IPAddress IP = WiFi.localIP();
    Serial.print(IP);
    Server.begin();
    Server1.begin();
    Server2.begin();
    Server3.begin();
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
    request->send(200, "text/plain", send);
  });    
Server.begin();
}
#endif
#endif