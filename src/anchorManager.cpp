#include <stdint.h>
#include <Arduino.h>
#include "U8g2lib.h"
#include "DW1000.h"
#include <Wire.h>
#include <WiFi.h>

 //setting up u8g2 class from lib use static to use in other than main.cpp codes

static String total_data = "";

//Choose whether debug messages of the anchorManager should be printed
//#define DEBUG_ANCHOR_MANAGER
//choose which communication mode you want to use (multiple choises availible)
//#define I2C
//#define USE_SERIAL//display the distance through uart


#ifdef I2C
static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

//Choose the amount of anchors supported
#define MAX_ANCHORS 6


struct anchor{
    uint16_t ID;
    uint8_t x;
    uint8_t y;
    float distance;
    uint8_t distance_counter;
    bool active;
};

static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, false};
static int anchorCount = 0;

static void addAnchor(uint16_t ID, uint8_t XCoordInMtr, uint8_t YCoordInMtr){
    if(anchorCount < MAX_ANCHORS){
        anchors[anchorCount] = {ID, XCoordInMtr, YCoordInMtr, 0.0, false};
        anchorCount++;
    }
}
////uncomment in main to check which anchors are connected(commented for faster performance)
static void printAnchorArray(){
    Serial.println("\n\nInitialized anchors:");
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID != 0){
            Serial.println(anchors[i].ID);
        }
    }
    Serial.println("\n\n");
}

static void setAnchorActive(uint16_t ID, bool isActive){
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID == ID){
            anchors[i].active = isActive;
            #ifdef DEBUG_ANCHOR_MANAGER
                if (isActive){
                    Serial.printf("activated anchor %u\n", ID);
                }else{
                    Serial.printf("deactivated anchor %u\n", ID);
                }
                return;
            #endif
        }
    }
    #ifdef DEBUG_ANCHOR_MANAGER
        Serial.printf("anchor %u not registrerd\n", ID);
    #endif
}

static void setDistanceIfRegisterdAnchor(uint16_t ID, double distance)
{
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        
        if (anchors[i].ID == ID)
        {
            if(distance <=0)
            distance = 0;
            anchors[i].distance = distance;
            anchors[i].distance_counter++;
            #ifdef DEBUG_ANCHOR_MANAGER
                Serial.print("Set distance of ");
                Serial.print(ID);
                Serial.print(" to ");
                Serial.print(distance);
                Serial.print(" at rxPower ");
                Serial.print(rxPower);
                Serial.print("\n");
            #endif
            return;
        }
    }
    #ifdef DEBUG_ANCHOR_MANAGER
        Serial.printf("anchor %u not registered\n", ID);
    #endif
}

static void outputDataJson()
{
#ifdef USE_SERIAL//print the distances through serial
   Serial.print("[");
    for (int i = 0; i < 3; i++)
    {
        if (anchors[i].active){
            if(i > 0)
            {
                Serial.print(", ");
            }
            Serial.printf("{\"ID\": %u, \"x\": %u, \"y\": %u, \"distance\": %f, \"active\": %d}",
                         anchors[i].ID,
                         anchors[i].x,
                         anchors[i].y,
                         anchors[i].distance,
                         anchors[i].active);
        }
    }
    Serial.println("]\n");
    #endif
    #ifdef I2C
    int y = 0;
    for (int i = 0; i < MAX_ANCHORS; i++)// for statement to print the active anchors through i2c
    {
        if(anchors[i].active)
        {
        int IDlength = snprintf(NULL, 0, "%d", anchors[i].ID);
        char* ID = (char*)malloc(IDlength+1);
        snprintf(ID, IDlength+1, "%d", anchors[i].ID);
        int length = snprintf(NULL, 0, "%g", anchors[i].distance);
        char* convert = (char*)malloc(length+1);
        snprintf(convert, length+1, "%g", anchors[i].distance);
        u8g2.setFont(u8g2_font_fancypixels_tf);
        u8g2.drawStr(0,y, ID);
        u8g2.drawStr(50,y,convert);
        free(convert);
        free(ID);
        y += 10;
        }
        }
        u8g2.sendBuffer();
        u8g2.clearBuffer();
    #endif
}

static String updateDataWiFi()
{
    String dataDistance = "";
    String dataID = "";
    String dataX = "";
    String dataY = "";
    total_data = "";
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].active){
                //divide to get the average
                dataID = anchors[i].ID;
                dataX = anchors[i].x;
                dataY = anchors[i].y;
                //anchors[i].distance /= anchors[i].distance_counter;
                dataDistance = anchors[i].distance;
                total_data = total_data + dataID + "ID" + dataDistance + 'd' + dataX + 'x' + dataY + 'y' + '\t';
        } 
    }
    Serial.println(total_data);
    return total_data;
}
