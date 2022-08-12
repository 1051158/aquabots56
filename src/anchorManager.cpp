#include <stdint.h>
#include <Arduino.h>
#include "U8g2lib.h"
#include <Wire.h>

//static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //setting up u8g2 class from lib use static to use in other than main.cpp codes


//Choose whether debug messages of the anchorManager should be printed
//#define DEBUG_ANCHOR_MANAGER
//choose which communication mode you want to use (multiple choises availible)
//#define I2C
#define USE_SERIAL
//#define TXT

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

static void setDistanceIfRegisterdAnchor(uint16_t ID, double distance, double rxPower){
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        
        if (anchors[i].ID == ID){
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

            /*if(anchors[i].distance_counter >= 20)//print distance after 20 measures
            {
            anchors[i].average_distance /= 20;
            Serial.print("average distance:");
            Serial.println(anchors[i].average_distance);
            anchors[i].average_distance = 0;
            anchors[i].distance_counter = 0;
            }*/
            return;
        }
    }
    #ifdef DEBUG_ANCHOR_MANAGER
        Serial.printf("anchor %u not registered\n", ID);
    #endif
}

static void outputDataJson(){
#ifdef USE_SERIAL
   Serial.print("[");
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID != 0){
            if(i > 0){
                Serial.print(", ");
            }
            Serial.printf("{\"ID\": %u, \"x\": %f, \"y\": %f, \"distance\": %f, \"active\": %d}",
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
    for (int i = 0; i < MAX_ANCHORS; i++)// for statement to print the active anchors
    {
        //bool checker = anchors[i].active;
        //if(checker == true)
        //{
            u8g2.println(anchors[i].distance);
        //}
    }
    #endif
}
