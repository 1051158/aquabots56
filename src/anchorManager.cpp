#include <stdint.h>
#include <Arduino.h>
#include <DW1000Ranging.h>
#include "DW1000.h"


static bool hulp_bool;
static uint8_t min_send = 0;

static uint8_t active_counter = 0;
 //setting up u8g2 class from lib use static to use in other than main.cpp codes

////////////////////All defines underneath are neccesary for rangetests////////////////////////////////
#define X_Y_TEST

#define NUM_OF_SEND 4 //number of times the value is send for excel file

#define RESET_DISTANCE_COUNTER_MAX_VALUE 2 //value to reset distance counter max to DISTANCE_COUNTER_MIN
#define DISTANCE_COUNTER_MIN 1
#define DISTANCE_COUNTER_INTERVAL 1

///////////////////Variables needed for antenna delay range tests////////////////////////////////////////////

static uint8_t distance_counter_max = DISTANCE_COUNTER_MIN;
static uint8_t hulp = 0;

//Choose whether debug messages of the anchorManager should be printed
//#define DEBUG_ANCHOR_MANAGER
//choose which communication mode you want to use (multiple choises availible)
//#define USE_SERIAL//display the distance through uart

#define STRLEN 20 

#define LONGEST_RANGE 12

//#define HEIGHT_DIFFERENCE

///////////////////////anchor info for the tag(change for the right real-time situation)/////////////////////

#define ANCHOR_ID_1 4369
#define ANCHOR_X_1 10
#define ANCHOR_Y_1 0

#define ANCHOR_ID_2 8738
#define ANCHOR_X_2 0
#define ANCHOR_Y_2 5

#define ANCHOR_ID_3 13107
#define ANCHOR_X_3 10
#define ANCHOR_Y_3 5

#define ANCHOR_ID_4 17476
#define ANCHOR_X_4 10
#define ANCHOR_Y_4 10

#define ANCHOR_ID_5 21845
#define ANCHOR_X_5 10
#define ANCHOR_Y_5 15
/*
#define ANCHOR_ID_6 26214
#define ANCHOR_X_6 15
#define ANCHOR_Y_6 10
*/
#define X 0
#define Y 1

//Choose the amount of anchors supported
#ifndef X_Y_TEST
    #define MAX_RANGE_DIS 10
    #define MAX_ANCHORS 1
    static float range_points [MAX_RANGE_DIS] = {1,2,3,4,5,6,7,8,9,10};
#endif
#ifdef X_Y_TEST
    //GIVE THE NUMBER OF DISTANCES THAT ARE BEING USED FOR CALIBRATION
    #define MAX_CAL_DIS 6 
    #define MAX_ANCHORS 4
    static float x_y_points [MAX_CAL_DIS][2] = {{1.5,1},{3,1},{4.5,1},{6,1},{7.5,1},{9,1}};
#endif

struct anchor{
    uint16_t ID;
    float x;
    float y;
    float distance;
    unsigned long sendTime;
    unsigned long lastSendTime;
    uint8_t distance_counter;
    uint8_t distance_counter_max;
    uint8_t num_of_send_counter;
    bool active;
    bool hulp_change_delay;
    bool done;
    String total_data;
    #ifdef X_Y_TEST
    float calibrationDistances[MAX_CAL_DIS];
    #endif
};

#ifdef X_Y_TEST
static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, 0, 0, 0, 0, false, false, false, "",{0,0,0,0,0,0} };
#endif

//fill in the height difference between anchors and tag in meters
#ifdef HEIGHT_DIFFERENCE
static float height_difference = 0.40;
#endif

#ifndef X_Y_TEST
    static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, 0, 0, 0, 0, false, false, false, ""};
#endif
static int anchorCount = 0;


static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr){
    if(anchorCount < MAX_ANCHORS){
        anchors[anchorCount].ID = ID;
        anchors[anchorCount].x = XCoordInMtr;
        anchors[anchorCount].y = YCoordInMtr;
        //Serial.print(anchors[anchorCount].ID);
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

static void setDistanceIfRegisterdAnchor(uint16_t ID, double distance, uint8_t anchornumber)
{
            if(distance <= 0)
            distance = 0;

            //calculate real distance with pythagoras when there is a height difference
            #ifdef HEIGHT_DIFFERENCE
            distance = sqrt(pow(distance,2) - pow(height_difference, 2));
            Serial.print(distance);
            #endif
            if(distance > LONGEST_RANGE)//if the measured value is bigger than the maximum range
            {anchors[anchornumber].distance += LONGEST_RANGE;//set is to max range
            anchors[anchornumber].distance_counter++;
            return;}
            anchors[anchornumber].distance += distance;//add distance 
            anchors[anchornumber].distance_counter++;//add 1 to distance_counter
            #ifdef DEBUG_ANCHOR_MANAGER
                Serial.print(ID);
                Serial.print(" to ");
                Serial.print(distance);
                //Serial.print(" at rxPower ");
                //Serial.print(rxPower);
                Serial.print("\n");
            #endif
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

static void getDistances()
{
    
}

static String generateWiFiString(uint8_t anchornumber)
{
    //get the distance and add one up to the distanceCounter.
    setDistanceIfRegisterdAnchor(DW1000Ranging.getDistantDevice()->getShortAddress(),DW1000Ranging.getDistantDevice()->getRange(), anchornumber); 
    //Serial.println(hulp);
    #ifndef RANGETEST
    //when the distance counter max has been reached convert the right string will be constructed
        if(anchors[anchornumber].distance_counter >= distance_counter_max)
        {
            String hulp_total_data = "";
            anchors[anchornumber].num_of_send_counter++;
            anchors[anchornumber].distance /= anchors[anchornumber].distance_counter;
            anchors[anchornumber].sendTime = millis();
            anchors[anchornumber].sendTime = anchors[anchornumber].sendTime - anchors[anchornumber].lastSendTime;
            anchors[anchornumber].lastSendTime = millis();
            if(anchors[anchornumber].num_of_send_counter >= NUM_OF_SEND)
            //after the amount of outputs requested by de #define NUM_OF_SEND button needs to be pressed again
            {
                if(anchors[anchornumber].distance_counter_max >= RESET_DISTANCE_COUNTER_MAX_VALUE)
                {
                    anchors[anchornumber].distance_counter_max = DISTANCE_COUNTER_MIN;
                    hulp_total_data = hulp_total_data + anchors[anchornumber].ID + "ID" + anchors[anchornumber].distance + 'd' + anchors[anchornumber].sendTime + "ms" "a\t";
                    anchors[anchornumber].num_of_send_counter = 0;
                    hulp_bool = true;
                    anchors[anchornumber].hulp_change_delay = true;
                    anchors[anchornumber].distance = 0;
                    anchors[anchornumber].distance_counter = 0;
                    //Serial.println(hulp_total_data);
                    return hulp_total_data;
                }
                if (anchors[anchornumber].distance_counter_max < RESET_DISTANCE_COUNTER_MAX_VALUE)
                {
                    hulp_total_data = hulp_total_data + anchors[anchornumber].ID + "ID" + anchors[anchornumber].distance + 'd'+  anchors[anchornumber].sendTime + "ms" + "e\t";
                    hulp_bool = true;
                    anchors[anchornumber].distance = 0;
                    anchors[anchornumber].distance_counter = 0;
                    anchors[anchornumber].hulp_change_delay = false;
                    //Serial.println(hulp_total_data);
                    //Serial.println(hulp_total_data);
                    anchors[anchornumber].num_of_send_counter = 0;
                    anchors[anchornumber].distance_counter_max++;
                    return hulp_total_data;
                }                 
            }
            hulp_total_data = hulp_total_data + anchors[anchornumber].ID + "ID" + anchors[anchornumber].distance + 'd' + anchors[anchornumber].sendTime + "ms" + "\t";
            //Serial.println(hulp_total_data);
            anchors[anchornumber].distance = 0;
            anchors[anchornumber].distance_counter = 0;
            return hulp_total_data;
        #endif
    } 
return "not";
}