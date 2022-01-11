
#include <stdint.h>
#include <Arduino.h>

#define MAX_ANCHORS 32

struct anchor{
    uint16_t ID;
    double x;
    double y;
    double distance;
};

static anchor anchors[MAX_ANCHORS] = {0, 0.0, 0.0, 0.0};
static int anchorCount = 0;

static void addAnchor(uint16_t ID, double XCoordInMtr, double YCoordInMtr){
    if(anchorCount < MAX_ANCHORS){
        anchors[anchorCount] = {ID, XCoordInMtr, YCoordInMtr};
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

static void setDistanceIfRegisterdAnchor(uint16_t ID, double distance){
    for (int i = 0; i < MAX_ANCHORS; i++)
    {
        if (anchors[i].ID == ID){
            anchors[i].distance = distance;
            break;
        }
    }
    Serial.print("Set distance of ");
    Serial.print(ID);
    Serial.print(" to ");
    Serial.print(distance);
    Serial.print("\n");
}


