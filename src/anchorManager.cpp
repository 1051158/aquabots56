#include <stdint.h>
#include <Arduino.h>
#include <DW1000Ranging.h>
#include "DW1000.h"
#include "Defines.cpp"

//#define HEIGHT_DIFFERENCE

static uint16_t antenna_delay = ANTENNA_DELAY_START;


//Choose the amount of anchors supported
#ifdef RANGETEST
    #define MAX_RANGE_DIS 10
    #define MAX_ANCHORS 1
    static float range_points [MAX_RANGE_DIS] = {1,2,3,4,5,6,7,8,9,10};
#endif


#ifdef X_Y_TEST
    #define MAX_ANCHORS 4
    //GIVE THE NUMBER OF DISTANCES THAT ARE BEING USED FOR CALIBRATION
    #ifndef Z_TEST
    #define MAX_CAL_DIS 6 
    static float x_y_points [MAX_CAL_DIS][2] = {{0.25,0.25},{0.5,0.5},{0.75,0.75},{1,1},{1.25,1.25},{1.5,1.5}};
    #endif
#endif


#ifdef Z_TEST
    #define MAX_CAL_DIS 6 
    static float x_y_points [MAX_CAL_DIS][3] = {{8.5,1,0.83}, {7,1,0.83}, {5.5,1,0.83}, {4,1,0.83}, {2.5,1,0.83}, {1,1,0.83}};
#endif


struct anchor{
    uint16_t ID;
    float x;
    float y;
    #ifdef Z_TEST
    float z;
    #endif
    float distance;
    unsigned long sendTime;
    unsigned long lastSendTime;
    uint8_t distance_counter;
    bool active;
    bool done;
    String total_data;
    #ifndef RANGETEST
    float calibrationDistances[MAX_CAL_DIS];
    #endif
};

static bool hulp_change_delay;
static uint8_t num_of_send_counter = 0;
static uint8_t distance_counter_max = DISTANCE_COUNTER_MIN;

#ifdef X_Y_TEST
#ifndef Z_TEST
static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, 0, 0, false, false, "",{0,0,0,0,0,0} };
#endif
#ifdef Z_TEST
    static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0, 0.0, 0, 0, 0, false, false, "",{0,0,0,0,0,0} };
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
/*
static void resetAnchors()
{

}
*/
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
    //for debugging:
    //Serial.print("distance: ");
    //Serial.println(distance);
    //Serial.println(anchors[anchornumber].distance_counter);

    //filter to skip the wrong measurements
    //if the measured distance is smaller than -1 or bigger than sqrt(x² + y²)
            if(distance <= -1 || distance>=15)
            {
                return false;
            }
            if(distance > LONGEST_RANGE)//if the measured value is bigger than the maximum range
            {anchors[anchornumber].distance += LONGEST_RANGE;//set is to max range
            anchors[anchornumber].distance_counter++;
            return true;}
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
            return true;
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

static bool generateWiFiString(uint8_t anchornumber)
{
    //get the distance and add one up to the distanceCounter.
    if(setDistanceIfRegisterdAnchor(DW1000Ranging.getDistantDevice()->getShortAddress(),DW1000Ranging.getDistantDevice()->getRange(), anchornumber)) 
    {//Serial.println(hulp);
    #ifndef RANGETEST
    //when the distance counter max has been reached convert the right string will be constructed
        if(anchors[anchornumber].distance_counter >= distance_counter_max)
        {
            anchors[anchornumber].distance /= anchors[anchornumber].distance_counter;
            //Serial.print("divided distance: ");
            //Serial.println(anchors[anchornumber].distance);
            anchors[anchornumber].sendTime = millis();
            anchors[anchornumber].sendTime = anchors[anchornumber].sendTime - anchors[anchornumber].lastSendTime;
            anchors[anchornumber].lastSendTime = millis();
            if(num_of_send_counter >= NUM_OF_SEND)
            //after the amount of outputs requested by de #define NUM_OF_SEND the outputs get resetted for the next antenna delay
            {
                if(distance_counter_max >= RESET_DISTANCE_COUNTER_MAX_VALUE)
                {
                    anchors[anchornumber].total_data = anchors[anchornumber].total_data + "a\t";
                    Serial.println('a');
                    return true;
                }
                if (distance_counter_max < RESET_DISTANCE_COUNTER_MAX_VALUE)
                {
                    //make the string
                    anchors[anchornumber].total_data = anchors[anchornumber].total_data + "e\t";
                    Serial.println('e');
                    return true;
                }                 
            }
            anchors[anchornumber].total_data = anchors[anchornumber].total_data + anchornumber + "ID" + anchors[anchornumber].distance + 'd' + anchors[anchornumber].sendTime + "ms" + "\t";
            //Serial.println(hulp_total_data);
            return true;
        #endif
        }
    }
    //when the distance counter max hasn't been reached send back false to measure again untill the maximum has been reached
return false;
}

/////////////////////synchronise all the anchors for integration with python///////////////////

static void synchronizeAnchors(void)
{
    num_of_send_counter++;
    for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
        anchors[i].distance = 0;
        anchors[i].distance_counter = 0;
        anchors[i].total_data = "";
        anchors[i].done = false;
    }
    if(distance_counter_max >= RESET_DISTANCE_COUNTER_MAX_VALUE && num_of_send_counter > NUM_OF_SEND) 
    {
        distance_counter_max = DISTANCE_COUNTER_MIN;
        num_of_send_counter = 0;
        hulp_change_delay = true;
        return;
    }
    if(distance_counter_max < RESET_DISTANCE_COUNTER_MAX_VALUE && num_of_send_counter > NUM_OF_SEND)
    {
        hulp_change_delay = false;
        //Serial.println(hulp_total_data);        
        num_of_send_counter = 0;
        //add one to the distance counter max and measure again with more samples
        distance_counter_max += DISTANCE_COUNTER_INTERVAL;
        return;
    }    
    
}

static void resetAnchors(void)
{
    num_of_send_counter = 0;
    hulp_change_delay = false;
    distance_counter_max = DISTANCE_COUNTER_MIN;
    for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
        anchors[i].done = false;
        anchors[i].distance = 0;
        anchors[i].distance_counter = 0;
        anchors[i].total_data = "";
    }
}