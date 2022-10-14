#include <stdint.h>
#include <Arduino.h>
#include "U8g2lib.h"
#include "DW1000.h"
#include <Wire.h>
#include <WiFi.h>

 //setting up u8g2 class from lib use static to use in other than main.cpp codes

//#define X_Y_TEST
////////////////////All defines underneath are neccesary for rangetests////////////////////////////////
#define RANGETEST
#define ANTENNA_INTERVAL 10 //interval between 2 antenna delays

#define ANTENNA_DELAY_START 16500 //start value antenna delay
#define ANTENNA_DELAY_END 16650 //end value antenna delay

#define NUM_OF_SEND 4 //number of times the value is send for excel file\

#define RESET_DISTANCE_COUNTER_MAX_VALUE 2 //value to reset distance counter max to DISTANCE_COUNTER_MIN
#define DISTANCE_COUNTER_MIN 1


///////////////////Variables needed for antenna delay range tests////////////////////////////////////////////

static uint8_t distance_counter_max = 1;
static bool hulp_bool = true;
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
#define MAX_ANCHORS 3


///////////////////////anchor info for the tag(change for the right real-time situation)/////////////////////

#define ANCHOR_ID_1 4369
#define ANCHOR_X_1 10
#define ANCHOR_Y_1 0

#define ANCHOR_ID_2 8738
#define ANCHOR_X_2 0
#define ANCHOR_Y_2 10


#define ANCHOR_ID_3 13107
#define ANCHOR_X_3 10
#define ANCHOR_Y_3 10


/*
#define ANCHOR_ID_4 17476
#define ANCHOR_X_4 10
#define ANCHOR_Y_4 0*/    // ToDo when the chips arrive


struct anchor{
    uint16_t ID;
    uint8_t x;
    uint8_t y;
    uint16_t antenna_delay;
    float distance;
    uint8_t distance_counter;
    uint8_t distance_counter_max;
    uint8_t num_of_send_counter;
    bool active;
    bool hulp_change_delay;
    bool done;
    String total_data;
};

static anchor anchors[MAX_ANCHORS] = {0, 0, 0, ANTENNA_DELAY_START, 0.0, 0, 0, 0, false, false, false, ""};
static int anchorCount = 0;

static double longest_range = 11;


static void addAnchor(uint16_t ID, uint8_t XCoordInMtr, uint8_t YCoordInMtr){
    if(anchorCount < MAX_ANCHORS){
        anchors[anchorCount] = {ID, XCoordInMtr, YCoordInMtr, ANTENNA_DELAY_START, 0.0, 0, 0, 0, false, false, false, ""};
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

static String updateDataWiFi(uint8_t anchornumber)
{
    //Serial.println(hulp);
        String hulp_total_data = "";
            #ifdef RANGETEST
                if(anchors[anchornumber].distance_counter >= distance_counter_max)
                {
                    anchors[anchornumber].num_of_send_counter++;
                    hulp++;// for integration with WIFI_TAG.cpp to read every new value in the http-request
                    anchors[anchornumber].distance /= anchors[anchornumber].distance_counter;
                    String distance = ""; 
                    String ID = "";
                    ID = anchors[anchornumber].ID;
                    distance = anchors[anchornumber].distance;
                    if(anchors[anchornumber].num_of_send_counter >= NUM_OF_SEND)
                    //after the amount of outputs requested by de #define NUM_OF_SEND button needs to be pressed again
                    {
                        anchors[anchornumber].distance_counter_max++;
                        if(anchors[anchornumber].distance_counter_max > 2)
                        {
                            anchors[anchornumber].distance_counter_max = DISTANCE_COUNTER_MIN;
                            hulp_total_data = ID + "ID" + distance + 'd'+ hulp + 'a';
                            anchors[anchornumber].num_of_send_counter = 0;
                            hulp_bool = true;
                            anchors[anchornumber].hulp_change_delay = true;
                            anchors[anchornumber].distance = 0;
                            anchors[anchornumber].distance_counter = 0;
                            //Serial.println(hulp_total_data);
                            return hulp_total_data;
                        }
                        else
                        {
                            anchors[anchornumber].num_of_send_counter = 0;
                            hulp_bool = true;
                            anchors[anchornumber].hulp_change_delay = true;
                            hulp_total_data = ID + "ID" + distance + 'd'+ hulp + 'e';
                            //Serial.println(hulp_total_data);
                            anchors[anchornumber].distance = 0;
                            anchors[anchornumber].distance_counter = 0;
                            return hulp_total_data;
                        }                 
                    }
                    hulp_total_data = ID + "ID" + distance + 'd'+ hulp;
                    //Serial.println(hulp_total_data);
                    anchors[anchornumber].distance = 0;
                    anchors[anchornumber].distance_counter = 0;
                    return hulp_total_data;
                #endif
                #ifndef RANGETEST
                dataX = anchors[anchornumber].x;
                dataY = anchors[anchornumber].y;
                anchors[anchornumber].total_data = anchors[anchornumber].total_data + dataID + "ID" + dataDistance + dataCounter +"dc" +'\t';//todo
                #endif
                } 
            return "not";
            }