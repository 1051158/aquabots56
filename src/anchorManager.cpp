#include <stdint.h>
#include <Arduino.h>
#include "U8g2lib.h"
#include "DW1000.h"
#include <Wire.h>
#include <WiFi.h>

 //setting up u8g2 class from lib use static to use in other than main.cpp codes

static String total_data = "";

//#define X_Y_TEST

#define RANGETEST

#define NUM_OF_SEND 4

static uint8_t output_counter = 0;
static uint8_t distance_counter_max = 2;
static bool hulp_bool = true;
static bool hulp_send_bool = false;
static bool hulp_change_delay = false;
static uint8_t hulp = 0;

//Choose whether debug messages of the anchorManager should be printed
//#define DEBUG_ANCHOR_MANAGER
//choose which communication mode you want to use (multiple choises availible)
//#define I2C
//#define USE_SERIAL//display the distance through uart


#ifdef I2C
static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

//Choose the amount of anchors supported
#define MAX_ANCHORS 1


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

static double longest_range = 11;


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
            if(distance>longest_range)//if the measured value is bigger than the maximum range
            {anchors[i].distance += longest_range;//set is to max range
            anchors[i].distance_counter++;
            return;}
            anchors[i].distance += distance;//add distance 
            anchors[i].distance_counter++;//add 1 to distance_counter
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
        anchors[i].distance = anchors[i].distance/anchors[i].distance_counter;
        Serial.print(anchors[i].distance_counter);
        int IDlength = snprintf(NULL, 0, "%d", anchors[i].ID);
        char* ID = (char*)malloc(IDlength+1);
        snprintf(ID, IDlength+1, "%d", anchors[i].ID);
        int length = snprintf(NULL, 0, "%g", anchors[i].distance);
        char* convert = (char*)malloc(length+1);
        snprintf(convert, length+1, "%g", anchors[i].distance);
        int counterLength = snprintf(NULL, 0, "%d", anchors[i].distance_counter);
        char* counter = (char*)malloc(length+1);
        snprintf(counter, counterLength+1, "%d", anchors[i].distance_counter);
        u8g2.setFont(u8g2_font_fancypixels_tf);
        u8g2.drawStr(0,y, ID);
        u8g2.drawStr(50,y,convert);
        u8g2.drawStr(90,y, counter);
        free(convert);
        free(ID);
        free(counter);
        y += 10;
        }
        }
        u8g2.sendBuffer();
        u8g2.clearBuffer();
    #endif
}

static String updateDataWiFi()
{
    //Serial.println(hulp);
    for (int i = 0; i < 1; i++)
    {
        if (anchors[i].active)
        {
            #ifdef RANGETEST
                if(anchors[i].distance_counter>=distance_counter_max)
                {
                    output_counter++;
                    hulp++;
                    anchors[i].distance /= anchors[i].distance_counter;
                    if(output_counter >= NUM_OF_SEND)
                    //after the amount of outputs requested by de #define NUM_OF_SEND button needs to be pressed again
                    {
                        distance_counter_max += 3;
                        if(distance_counter_max > 5)
                        {
                            distance_counter_max = 2;
                            total_data = total_data + anchors[i].ID + "ID" + anchors[i].distance + 'd'+ hulp + 'a';
                            output_counter = 0;
                            hulp_bool = true;
                            hulp_change_delay = true;
                            anchors[i].distance = 0;
                            anchors[i].distance_counter = 0;
                            //Serial.println(total_data);
                            break;
                        }
                        else
                        {
                            output_counter = 0;
                            hulp_bool = true;
                            total_data = total_data + anchors[i].ID + "ID" + anchors[i].distance + 'd'+ hulp + 'e';
                            //Serial.println(total_data);
                            anchors[i].distance = 0;
                            anchors[i].distance_counter = 0;
                            break;
                        }                 
                    }
                    total_data = total_data + anchors[i].ID + "ID" + anchors[i].distance + 'd'+ hulp;
                    Serial.println(total_data);
                    anchors[i].distance = 0;
                    anchors[i].distance_counter = 0;
                #endif
                #ifndef RANGETEST
                dataX = anchors[i].x;
                dataY = anchors[i].y;
                total_data = total_data + dataID + "ID" + dataDistance + dataCounter +"dc" +'\t';//todo
                #endif
            } 
        }
    }
    //Serial.println(total_data);
    return total_data;
}