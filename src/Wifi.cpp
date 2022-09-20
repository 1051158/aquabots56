#include <WiFi.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

static AsyncWebServer Server(80);

static void WiFi_settings(void)
{
    const char* ssid = "ESP32-Access-Point";
    const char* psswrd = "123456789";
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, psswrd);
    IPAddress IP = WiFi.softAPIP();
    Serial.print(IP);
}

