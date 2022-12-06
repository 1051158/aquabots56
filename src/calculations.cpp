#include <stdint.h>
#include <Arduino.h>
#include <stdio.h>
#include "anchorManager.cpp"


//////////////////uncomment to check the calculations of calibration distances/////////////////////////////
//#define DEBUG_CAL


#ifdef X_Y_TEST
//////////////////Calculate the distances at every X and Y point for the calibration of Antenna Delay///////
static void CalibrationDistances()
{
    for(uint8_t j = 0; j < MAX_ANCHORS; j++)
    {
        for (uint8_t i = 0; i < MAX_CAL_DIS; i++)
        {
            #ifdef DEBUG_CAL
            float verschil = anchors[j].y - x_y_points[i][Y];
            Serial.print("vy:\t");
            Serial.println(verschil);
            verschil = anchors[j].x - x_y_points[i][Y];
            Serial.print("vx:\t");
            Serial.println(verschil);
            Serial.print(j+1);
            Serial.print(":\t");
            Serial.print(i+1);
            Serial.print(":\t");
            #endif
            anchors[j].calibrationDistances[i] = sqrt(pow(anchors[j].x - x_y_points[i][X], 2) + pow(anchors[j].y - x_y_points[i][Y], 2));
            Serial.print("anchor:\t");
            Serial.print(j);
            Serial.print("cal_number:\t");
            Serial.print(i);
            Serial.println(anchors[j].calibrationDistances[i]); 
        }
    }
}
/*
static void x_y_cal(anchor anchor1, anchor anchor2, anchor anchor3)
{ 
    float A,B,C,D,E,F; 
    A = (-2*anchor1.x) + (2*anchor2.x);
    B = (-2*anchor1.y) + (2*anchor2.y);
    C = pow(anchor1.distance, 2) - pow(anchor2.distance, 2) - pow(anchor1.x, 2) + pow(anchor2.x, 2) - pow(anchor1.y, 2) + pow(anchor2.y, 2);
    D = -2*anchor2.x + 2*anchor3.x;
    E = -2*anchor2.y + 2*anchor3.y;
    F = pow(anchor2.distance,2) - pow(anchor3.distance, 2) - pow(anchor2.x, 2) + pow(anchor3.x, 2) - pow(anchor2.y, 2) + pow(anchor3.y, 2);
    
    x = ((C*E) - (F*B))/((E*A) - (B*D));
    y = ((C*D) - (A*F))/((B*D) - (A*E));
}*/
#endif