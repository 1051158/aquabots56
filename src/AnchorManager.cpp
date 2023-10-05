#include <stdint.h>
#include <Arduino.h>
#include <DW1000Ranging.h>
#include "DW1000.h"
#include "Settings.cpp"
#include <stdint.h>
#include <string.h>

// Define a variable to track the current function number (used for debugging)
static uint8_t functionNumber = 0;

// Define a data structure 'anchor' to represent anchor information
struct anchor
{
    uint16_t ID; // Anchor ID
    float x;     // X-coordinate
    float y;     // Y-coordinate
#ifdef Z_TEST
    float z; // Z-coordinate (only if Z_TEST is defined)
#endif
    float distance;           // Measured distance
    uint8_t distance_counter; // Counter for distance measurements
    bool active;              // Flag indicating if the anchor is active
    bool done;                // Flag indicating if the anchor data is complete
    String total_data;        // Total data collected from the anchor
#ifndef RANGETEST
    float calibrationDistances[MAX_CAL_DIS]; // Array for calibration distances (not used in RANGETEST)
#endif
};

// Bools controlled by server functions (can be called with Python code)
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
// Initialize anchor array for X-Y test (without Z coordinate)
static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, false, false, "", {0, 0, 0, 0, 0, 0}};
#endif
#ifdef Z_TEST
// Initialize anchor array for X-Y-Z test (with Z coordinate)
static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0, 0.0, 0, false, false, "", {0, 0, 0}};
#endif
#endif

// When testing with 1 anchor, the struct will change
#ifdef RANGETEST
static anchor anchors[MAX_ANCHORS] = {0, 0, 0, 0.0, 0, 0, 0, 0, 0, false, false, false, ""};
#endif

static int anchorCount = 0;

#ifdef X_Y_Z_TEST
// Function to add an anchor with X, Y, and Z coordinates
static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr, float ZCoordInMtr)
{
    if (anchorCount < MAX_ANCHORS)
    {
        anchors[anchorCount].ID = ID;
        anchors[anchorCount].x = XCoordInMtr;
        anchors[anchorCount].y = YCoordInMtr;
        anchors[anchorCount].z = ZCoordInMtr;
        anchorCount++;
    }
}

// Function to add an anchor with X and Y coordinates (without Z)
static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr)
#endif

#ifdef Z_TEST
    // Function to add an anchor with X, Y, and Z coordinates
    static void addAnchor(uint16_t ID, float XCoordInMtr, float YCoordInMtr, float ZCoordInMtr)
#endif
{
    if (anchorCount < MAX_ANCHORS)
    {
        anchors[anchorCount].ID = ID;
        anchors[anchorCount].x = XCoordInMtr;
        anchors[anchorCount].y = YCoordInMtr;
#ifdef Z_TEST
        anchors[anchorCount].z = ZCoordInMtr;
#endif
        anchorCount++;
    }
}

// Uncomment in the main code to check which anchors are connected (commented for performance)
static void printAnchorArray()
{
    Serial.println("\n\nInitialized anchors:");
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID != 0)
        {
            Serial.println(anchors[i].ID);
        }
    }
    Serial.println("\n\n");
}

// Function to set an anchor as active or inactive
static void setAnchorActive(uint16_t ID, bool isActive)
{
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID == ID)
        {
            anchors[i].active = isActive;
#ifdef DEBUG_ANCHOR_MANAGER
            if (isActive)
            {
                Serial.printf("Activated anchor %u\n", ID);
            }
            else
            {
                Serial.printf("Deactivated anchor %u\n", ID);
            }
            return;
#endif
        }
    }
#ifdef DEBUG_ANCHOR_MANAGER
    Serial.printf("Anchor %u not registered\n", ID);
#endif
}

// Function to set the distance if the anchor is registered
static bool setDistanceIfRegisteredAnchor(uint16_t ID, double distance, uint8_t anchorNumber)
{
    functionNumber = 0x06;
    if (_debugSerial)
    {
        Serial.print(functionNumber);
        Serial.printf("Distance = %f", distance);
    }
    // Filter to skip incorrect measurements
    if (distance <= -1 || distance >= 30)
    {
        return false;
    }

    if (distance > LONGEST_RANGE)
    {
        distance = LONGEST_RANGE;
    }

    if (distance < 0)
    {
        distance = 0.01;
    }
    anchors[anchorNumber].distance += distance; // Add distance
    return true;
}

// Function to generate distance and timer data for an anchor
static bool generateDistanceAndTimer(uint8_t anchorNumber)
{
    functionNumber = 0x07;
    if (_debugSerial)
    {
        Serial.println(functionNumber);
    }
    if (setDistanceIfRegisteredAnchor(DW1000Ranging.getDistantDevice()->getShortAddress(),
                                      DW1000Ranging.getDistantDevice()->getRange(), anchorNumber))
    {
        return true;
    }
    return false;
}

// Function to output data through UART (serial)
static void outputDataUart()
{
    Serial.print("[");
    for (int i = 0; i < 3; i++)
    {
        if (anchors[i].active)
        {
            if (i > 0)
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

// Synchronize all the anchors for integration with Python
static void synchronizeAnchors(void)
{
    functionNumber = 0x08;
    for (uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
        anchors[i].distance = 0;
        anchors[i].distance_counter = 0;
        anchors[i].total_data = "";
        anchors[i].done = false;
    }
}

static bool _resetAnchors;

// Function to reset all the anchors when a new sendRequest has been established
static void resetAnchors()
{
    functionNumber = 0x09;
    distance_counter_max = DISTANCE_COUNTER_MIN;
    Serial.print("reset");
    for (uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
        anchors[i].done = false;
        anchors[i].distance = 0;
        anchors[i].distance_counter = 0;
        anchors[i].total_data = "";
    }
    _resetAnchors = false;
}
