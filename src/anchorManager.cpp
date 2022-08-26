#include <stdint.h>
#include <Arduino.h>
#include "U8g2lib.h"
#include <Wire.h>

 //setting up u8g2 class from lib use static to use in other than main.cpp codes


//Choose whether debug messages of the anchorManager should be printed
//#define DEBUG_ANCHOR_MANAGER
//choose which communication mode you want to use (multiple choises availible)
#define I2C
#define USE_SERIAL
//#define TXT

#ifdef I2C
static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

//Choose the amount of anchors supported
#define MAX_ANCHORS 6


struct anchor{
    uint16_t ID;
    double x;
    double y;
    double distance;
    double average_distance;
    uint8_t distance_counter;
    double rxPower;
    bool active;
};

static anchor anchors[MAX_ANCHORS] = {0, 0.0, 0.0, 0.0, 0.0, 0, 0.0, false};
static int anchorCount = 0;

static void addAnchor(uint16_t ID, double XCoordInMtr, double YCoordInMtr){
    if(anchorCount < MAX_ANCHORS){
        anchors[anchorCount] = {ID, XCoordInMtr, YCoordInMtr, 0.0, 0.0, false};
        anchorCount++;
    }
}

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

static void setDistanceIfRegisterdAnchor(uint16_t ID, double distance, double rxPower)
{
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        
        if (anchors[i].ID == ID)
        {
            if(distance <=0)
            distance = 0;
            anchors[i].distance = distance;
            anchors[i].average_distance += distance;
            anchors[i].distance_counter++;
            anchors[i].rxPower = rxPower;
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
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID != 0){
            if(i > 0)
            {
                Serial.print(", ");
            }
            if(anchors[i].active)
            {
                anchors[i].average_distance /= anchors[i].distance_counter;//divide to get the average
                Serial.printf("{\"ID\": %u, \"x\": %f, \"y\": %f, \"distance\": %f, \"active\": %d}",
                             anchors[i].ID,
                             anchors[i].x,
                             anchors[i].y,
                             anchors[i].average_distance,
                             anchors[i].active);
            }
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
        int length = snprintf(NULL, 0, "%g", anchors[i].average_distance);
        char* convert = (char*)malloc(length+1);
        snprintf(convert, length+1, "%g", anchors[i].distance);
        u8g2.setFont(u8g2_font_fancypixels_tf);
        u8g2.drawStr(0,y, ID);
        u8g2.drawStr(50,y,convert);
        free(convert);
        free(ID);
        y += 10;
        anchors[i].average_distance = 0;//set both values back to zero to get new average
        anchors[i].distance_counter = 0;
        }
        }
        u8g2.sendBuffer();
        u8g2.clearBuffer();
    #endif
}

