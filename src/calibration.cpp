#include <stdint.h>
#include <Arduino.h>
#include "DW1000Ranging.h"

static float last_delta = 0.0;
static uint8_t Adelay_delta = 100;
static uint16_t this_anchor_Adelay = 16500;//starting antenna delay;
static uint16_t this_anchor_target_distance = 7.00;

static void calibration(void)
{
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), DEC);
  float dist = 0;
  for (int i = 0; i < 100; i++) {
    // get and average 100 measurements
    dist += DW1000Ranging.getDistantDevice()->getRange();
  }
  dist /= 100.0;
  Serial.print(",");
  Serial.print(dist); 
  if (Adelay_delta < 3) {
    Serial.print(", final Adelay ");
    Serial.println(this_anchor_Adelay);
//    Serial.print("Check: stored Adelay = ");
//    Serial.println(DW1000.getAntennaDelay());
    while(1);  //done calibrating
  }
  float this_delta = dist - this_anchor_target_distance;  //error in measured distance

  if ( this_delta * last_delta < 0.0) Adelay_delta = Adelay_delta / 2; //sign changed, reduce step size
    last_delta = this_delta;
  
  if (this_delta > 0.0 ) this_anchor_Adelay += Adelay_delta; //new trial Adelay
  else this_anchor_Adelay -= Adelay_delta;
  
  Serial.print(", Adelay = ");
  Serial.println (this_anchor_Adelay);
  DW1000.setAntennaDelay(this_anchor_Adelay);
}