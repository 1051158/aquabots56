#include <WiFi.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//#define WIFI_ON//comment if WiFi shoud be turned off

static AsyncWebServer Server(80);

static void WiFi_settings(void)
{
    
    //Server.begin();
}

