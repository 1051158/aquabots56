#include <stdint.h>
#include <Arduino.h>
#include <DW1000Ranging.h>
#include "DW1000.h"
#include "Settings.cpp"
#include <stdint.h>
#include <string.h>

//#define HEIGHT_DIFFERENCE


// used to debug through wifi(see where this code is when python uses the getRequest function)
static uint8_t functionNumber = 0;

struct anchor
{
    uint16_t ID;
    float x;
    float y;
    #ifdef Z_TEST
    float z;
    #endif
    float distance;
    //unsigned long sendTime;
    //unsigned long lastSendTime;
    uint8_t distance_counter;
    bool active;
    bool done;
    String total_data;
    #ifndef RANGETEST
    float calibrationDistances[MAX_CAL_DIS];
    #endif
};

//bools controlled by the server functions(can be called with python code)
static bool _addAD = false;
static bool _subAD = false;
static bool _resetAD = false;
static bool _resetDCM = false;
static bool _addDCM = false;

static uint8_t distance_counter_max = DISTANCE_COUNTER_MIN;

#ifdef TYPE_TAG
static uint16_t antenna_delay = ANTENNA_DELAY_START;
#endif

#ifdef TYPE_ANCHOR
static uint16_t antenna_delay = ANTENNA_DELAY;
#endif


#ifdef X_Y_TEST
#ifndef Z_TEST
static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, false, false, "", {0,0,0,0,0,0} };
#endif
#ifdef Z_TEST
    static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0, 0.0, 0, false, false, "",{0,0,0}};
#endif
#endif

//when testing with 1 anchor the struct will change
#ifdef RANGETEST
    static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, 0, 0, 0, 0, false, false, false, ""};
#endif

static int anchorCount = 0;

#ifdef X_Y_Z_TEST
static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr, float ZCoordInMtr){
    if(anchorCount < MAX_ANCHORS){
        anchors[anchorCount].ID = ID;
        anchors[anchorCount].x = XCoordInMtr;
        anchors[anchorCount].y = YCoordInMtr;
        anchors[anchorCount].z = ZCoordInMtr;
        //Serial.print(anchors[anchorCount].ID);
        anchorCount++;
    }
}
#endif

#ifdef X_Y_TEST
#ifndef Z_TEST
static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr)
#endif

#ifdef Z_TEST
static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr, float ZCoordInMtr)
#endif
{
    if(anchorCount < MAX_ANCHORS)
    {
        anchors[anchorCount].ID = ID;
        anchors[anchorCount].x = XCoordInMtr;
        anchors[anchorCount].y = YCoordInMtr;
        #ifdef Z_TEST
        anchors[anchorCount].z = ZCoordInMtr;
        #endif
        //Serial.print(anchors[anchorCount].ID);
        anchorCount++;
    }
}
#endif
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

static void setAnchorActive(uint16_t ID, bool isActive)
{
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


static bool setDistanceIfRegisterdAnchor(uint16_t ID, double distance, uint8_t anchornumber)
{
    functionNumber = 0x06;
    if(_debugSerial)
        {
            Serial.print(functionNumber);
            Serial.printf("distance = %f", distance);
        }
    //filter to skip the wrong measurements
    if(distance <= -1 || distance>=30)
        return false;

    if(distance > LONGEST_RANGE)//if the measured value is bigger than the maximum range
        distance = LONGEST_RANGE;

    if(distance < 0)
        distance = 0.01;
    anchors[anchornumber].distance += distance;//add distance 
    //anchors[anchornumber].distance_counter++;//add 1 to distance_counter
    return true;

}

static bool generateDistanceAndTimer(uint8_t anchornumber)
{
    functionNumber = 0x07;
    if(_debugSerial)
        Serial.println(functionNumber);
    if(setDistanceIfRegisterdAnchor(DW1000Ranging.getDistantDevice()->getShortAddress(),DW1000Ranging.getDistantDevice()->getRange(), anchornumber)) 
    {
        return true;
    }
return false;
}

#ifdef USE_SERIAL//print the distances through serial
static void outputDataUart()
{
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
    }
#endif

/////////////////////synchronise all the anchors for integration with python///////////////////

//Synchronize the anchors. This is necessary for when less than MAX_ANCHORS has been send
static void synchronizeAnchors(void)
{
    functionNumber = 0x08;
    for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
        anchors[i].distance = 0;
        anchors[i].distance_counter = 0;
        anchors[i].total_data = "";
        anchors[i].done = false;
    }   
}

static bool _resetAnchors;
//function to reset all the anchors when a new sendRequest has been astablished
static void resetAnchors()
{
    functionNumber = 0x09;
    distance_counter_max = DISTANCE_COUNTER_MIN;
    Serial.print("reset");
    for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
        anchors[i].done = false;
        anchors[i].distance = 0;
        anchors[i].distance_counter = 0;
        anchors[i].total_data = "";
    }
    _resetAnchors = false;
}