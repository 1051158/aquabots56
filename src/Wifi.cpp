#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Buttons.cpp"

// bools to verify the string in the functions send_total_data_server() and
static bool rdy2send = false;
static bool done_send = false;

// bools to switch the ways of measuerement through wifi
static bool _accuracy = false;
static bool _lowpower = false;

static uint8_t active_counter = 0;

static bool start_test = false;

// IP's used by all devices

static AsyncWebServer anchorADserver(81);
static AsyncWebServer measureServer(88);
// belong to "IP:80/caldis" link
#ifdef TYPE_TAG

// IP's only used by tag
static AsyncWebServer sendDistancesServer(80);
static AsyncWebServer DCMserver(82);
static AsyncWebServer sendMenuServer(83);
static AsyncWebServer startServer(84);
static AsyncWebServer cladisServer(85);
static AsyncWebServer restartServer(86);

// a huge string with all the info will be sent to the python script
static String makeCaldisPackage()
{
  String total_data_cal;
#ifdef RANGETEST
  for (uint8_t i = 0; i < MAX_RANGE_DIS; i++)
    total_data_cal = total_data_cal + range_points[i] + ';';
#endif
#ifdef X_Y_TEST
  if (MAX_ANCHORS >= 2)
    for (uint8_t i = 0; i < MAX_CAL_DIS; i++)
    {
      total_data_cal = total_data_cal + '(' + x_y_points[i][X] + ',' + x_y_points[i][Y];
#ifdef Z_TEST
      total_data_cal = total_data_cal + ',' + x_y_points[i][Z];
#endif
      total_data_cal = total_data_cal + ");";
    }
  else
  {
    total_data_cal = "wrong anchornumber in tag program";
    return total_data_cal;
  }
#endif
  total_data_cal = total_data_cal + "\t\n" + MAX_ANCHORS + "max" + ANTENNA_DELAY_START + 'S' + ANTENNA_DELAY_END + 'E' + ANTENNA_INTERVAL + "I" + "\t\n";
#ifdef PYTHON_CONTROL
  total_data_cal = total_data_cal + NUM_OF_SEND + "nos" + DISTANCE_COUNTER_MIN + '-' + RESET_DISTANCE_COUNTER_MAX_VALUE + "r+" + DISTANCE_COUNTER_INTERVAL + "in" + "\t\n";
#endif
#ifdef X_Y_TEST
  for (uint8_t j = 0; j < MAX_ANCHORS; j++)
  {
    // send the coordinates of the anchors and the distances at every measurepoint
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

// belong to "/anchors" server(sends the distances through wifi)
static String send_total_data_server()
{
  //////////////check how many anchors are done with measuring//////////////////////////////////////////////
  String total_data_1;
  uint8_t dataCounter = 0;
  //////////////send signal if 3 out of MAX_ANCHORS anchors are done sending/////////////////////////////////
  for (uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    // check if the anchor have a found distance
    if (anchors[i].done && anchors[i].distance > 0)
    {
      total_data_1 = total_data_1 + anchors[i].total_data + i + "ID" + anchors[i].distance + 'd' + "\t";
      // store the data of every anchor in one total string to send
      dataCounter++;
    }

    // only send if more then 3 distances have been found
    if (dataCounter >= 3)
    {
      total_data_1 = total_data_1 + '\n';
      return total_data_1;
    }
  }

  // Send the value of the anchors that sended the data if debug is necessary
  total_data_1 = total_data_1 + "dC" + dataCounter;
  return total_data_1;
  ////////send "end" to pythonCode to go to next worksheet an calibrate the next point in the pool//////
}
#endif

#ifdef WIFI_EXTERN_ON

///////////////////When the ESP32 is not an acces point(AP) use this function in void setup() of main.cpp////////////////////

static void WiFiSettingsExtern(void)
{
  uint8_t wifiCounter = 0;
  bool wifiTimer = false;
  // Serial.println(ssid);

  const char *ssid = "Tesla IoT";
  const char *psswrd = "fsL6HgjN";

  ////////////log in into the router for extern wifi connection//////////////
  WiFi.begin(ssid, psswrd);
  while (WiFi.status() != WL_CONNECTED && !wifiTimer)
  {
    if (wifiCounter >= 10)
    {
      wifiTimer = true;
      Serial.println("connection failed!");
    }
    delay(1000);

#ifdef SERIAL_DEBUG
    Serial.println("Connecting to WiFi..");
#endif

    wifiCounter++;
  }
  if (!wifiTimer)
  {

///////////////////////////////////////////////////All the links for the tag//////////////////////////////////////
#ifdef TYPE_TAG
    sendDistancesServer.on("/anchors", HTTP_GET, [](AsyncWebServerRequest *request)
                           {
    if(active_counter < 3 && active_counter > 0)
    {
      request->send(200, "text/plain", "notActive");
      return;
    }
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
      request->send(200, "text/plain", functionName.c_str()); });

#ifdef SERVER_CONTROLLER
    // addAD when the python script has reached its DCM

    // server interrupt function to reset DCM
    DCMserver.on("/resetDCM", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
  request->send(200, "text/plain", "2");
  _resetDCM = true; });

    // server interrupt function to add to average counter
    DCMserver.on("/addDCM", HTTP_GET, [](AsyncWebServerRequest *request)
                 {
  request->send(200, "text/plain", "2");
  _addDCM = true; });

    startServer.on("/start", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
  request->send(200, "text/plain", "4");
  i2cMenu[START_SEND].status = true; });

#endif

    cladisServer.on("/caldis", HTTP_GET, [](AsyncWebServerRequest *request)
                    {
    String send;
    send = makeCaldisPackage().c_str();
    request->send(200, "text/plain", send); });

    // to control the python code with the interruptbuttons
    sendMenuServer.on("/sendMenu", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
    //when the button is pressed and the pythoncode has all measurements send the arraynumber of START_SEND
    if(i2cMenu[START_SEND].status)
    {  
      request->send(200, "text/plain", "0");
      resetAnchors();
      _resetAnchors = true;
    }

    //when this button is pressed and the pythoncode has all measurements send the arraynumber of END_CODE
    if(i2cMenu[END_CODE].status)
    {
      request->send(200, "text/plain", "3");
      esp_restart();
    }

    request->send(200, "text/plain", "n"); });
    restartServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
                     {
    request->send(200, "text/plain", "Restart");
    delay(1000);
    esp_restart(); });

    anchorADserver.on("/addAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  uint16_t AD_2_send = antenna_delay;
  String AD_send = "";
  AD_send = AD_send + AD_2_send;
  request->send(200, "text/plain", AD_send);
  _addAD = true; });

    // resetAD when the excel file has reached its max of the AD_test
    anchorADserver.on("/resetAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  uint16_t AD_2_send = antenna_delay;
  i2cMenu[START_SEND].status = false;
  request->send(200, "text/plain", "1");
  _resetAD = true; });

    anchorADserver.on("/subAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  uint16_t AD_2_send = antenna_delay - 5;
  String AD_send = "";
  AD_send = AD_send + AD_2_send;
  request->send(200, "text/plain", AD_send);
  _subAD = true; });

#endif
    measureServer.on("/accuracy", HTTP_GET, [](AsyncWebServerRequest *request)
                     { _accuracy = true; 
               request->send(200, "text/plain", "accuracy"); });

    measureServer.on("/lowpower", HTTP_GET, [](AsyncWebServerRequest *request)
                     { _lowpower = true; 
               request->send(200, "text/plain", "lowpower"); });

    ///////////////////////////////////////////////////All the links for the anchor//////////////////////////////////////

#ifdef TYPE_ANCHOR
    anchorADserver.on("/addAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  antenna_delay += 5;
  String AD_send = "";
  _addAD = true;
  AD_send = AD_send + antenna_delay;
  request->send(200, "text/plain", AD_send); });

    // resetAD when the excel file has reached its max of the AD_test
    anchorADserver.on("/resetAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  antenna_delay = ANTENNA_DELAY;
  String AD_send = "";
  AD_send = AD_send + antenna_delay;
  request->send(200, "text/plain", AD_send); });

    anchorADserver.on("/subAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  antenna_delay -= 5;
  String AD_send = "";
  AD_send = AD_send + antenna_delay;
  request->send(200, "text/plain", AD_send); });

    anchorADserver.on("/setAD", HTTP_GET, [](AsyncWebServerRequest *request)
                      {
  request->send(200, "text/plain", "succes!");
  DW1000.setAntennaDelay(antenna_delay); });
#endif
    IPAddress IP = WiFi.localIP();
    // check and print IPadress on the little screen to fill into the laptop
    String ip = "";
    for (uint8_t i = 0; i < 4; i++)
    {
      ip += String(IP[i]) + '.';
    }
    _i2c.print(ip.c_str(), true);
#ifdef TYPE_TAG
    sendDistancesServer.begin();
    DCMserver.begin();
    sendMenuServer.begin();
    startServer.begin();
    cladisServer.begin();
    restartServer.begin();
#endif
    measureServer.begin();
    anchorADserver.begin();

#endif
  }
}

#ifdef WIFI_AP_ON
///////////////////When the ESP32 is an acces point(AP) use this function in void setup() of main.cpp////////////////////
static void WiFiSettingsAP(void)
{
  const char *ssid = "ESP32-Access-Point";
  const char *psswrd = "123456789";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, psswrd);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(IP); // configure the wifi settings when
  sendDistancesServer.on("/anchors", HTTP_GET, [](AsyncWebServerRequest *request)
                         {
    String send;
    send = send_total_data_server().c_str();
    i2cprint(send.c_str());
    request->send(200, "text/plain", send); });
  sendDistancesServer.begin();
}
#endif
