#include <WiFi.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define WIFI_ON//comment if WiFi shoud be turned off

static AsyncWebServer Server(80);

static void WiFi_settings(void)
{
    const char* ssid = "ESP-32";
    const char* psswrd = "123456789";
    WiFi.softAP(ssid, psswrd);
    IPAddress IP = WiFi.softAPIP();
    Serial.print(IP);
    Server.begin();
}

