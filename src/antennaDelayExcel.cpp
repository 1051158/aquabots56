#include "Wifi.cpp"
#include "i2c.cpp"

static uint8_t change_delay_counter = 0;
static uint8_t cal_points_counter = 1;
static uint8_t anchors_to_calculate_counter = 0;

#define DEBUG_SYNCHRONISE


/////////////////////////////////Function to send distances for the x-y calculation in python///////////////////////////////

static void changeAD()
{
  antenna_delay += ANTENNA_INTERVAL;
  //When it changes the tester will notice on the i2c screen
  String AD = "";
  AD = AD + antenna_delay;
  _i2c.print(AD.c_str(), true);
  //change the antenna delay on tag
  DW1000.setAntennaDelay(antenna_delay);
  //when all the antenna delays on the secified coördinates have been tested:
  if(antenna_delay > ANTENNA_DELAY_END)
  {
    /////Reset the anchor delay to minimal to redo the test at a different coördinate//////////////
    antenna_delay = ANTENNA_DELAY_START;
    //When it changes the tester will notice on the i2c screen
    #ifdef X_Y_TEST
    AD = "";
    AD = AD + '(' + x_y_points[cal_points_counter][0] + ',' + x_y_points[cal_points_counter][1] + ')';
    _i2c.print(AD.c_str(), true);      
    //a counter is used to synchronise the integration with python
    cal_points_counter++;
    if(cal_points_counter >= MAX_CAL_DIS)
    { 
      cal_points_counter = 0;
    }
    #endif
    //the String in every struct from the array will send "end" for the integration with python
    i2cMenu[1].status = false;
    end_done = true;
    for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
    anchors[i].hulp_change_delay = false;
    anchors[i].done = true;
    }
    while(end_done)
    {
      #ifdef DEBUG_SYNCHRONIZE
      Serial.print('1');
      #endif
    }
    //delay(200);
  }
}

static void SendDistancesAD()
{
  //counter to make sure all anchors who send distances are synchronised  
  uint8_t synchronise = 0;
  change_delay_counter = 0;
  anchors_to_calculate_counter = 0;
  //check if python code has started yet if not skip measuring distance with making the interrupt bool false
  if(!start_test)
  {
    _i2c.print("not connected", true);
    i2cMenu[1].status = false;
  }
  #ifdef DEBUG_INTERRUPT
    Serial.print('S');
    _i2c.print("S");
  #endif

  #ifndef DEBUG_INTERRPUT
  //get ranges from all the anchors in a for statement
  for(uint8_t i = 0; i < MAX_ANCHORS;i++)
  {
  //check if the measured ID of the found distance is equal to the right anchor
  //also check if the found distance has been send already(for synchronising)
    if(!anchors[i].done && anchors[i].ID == DW1000Ranging.getDistantDevice()->getShortAddress())
    {
      //convert all the variables into a String to send over wifi
      anchors[i].total_data = generateWiFiString(i);
      //Serial.print(anchors[i].total_data);
      //when the function has not been succeeded empty the string in anchor struct
      if(anchors[i].total_data == "not")
      {
        Serial.println('n');
        anchors[i].total_data = "";
      }
    }
    //when all the data of the antenna_delay for one anchor is done a counter is used for synchronisation
  }
  for(uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    if(anchors[i].hulp_change_delay)
    change_delay_counter++;
  }
  #ifdef DEBUG_SYNCHRONIZE
  Serial.print("delay");
  Serial.println(change_delay_counter);
  #endif
  //when for statement to check distances is done check if Antenna Delay needs to be changed
    if(change_delay_counter >= 3)
      changeAD();
     
//if the AD doesn't need to be changed there will be checked if the anchors are done with measuring
    uint8_t synchornise = 0;
    for (uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
    //check which anchor is done with measuring to synchonise with those who didn't
      if(anchors[i].done)
      {
        synchornise = i;
        break;
      }
    }
      for(uint8_t i = 0; i < MAX_ANCHORS; i++)
      {
        #ifdef DEBUG_SYNCHRONISE
        Serial.print("before:");
        Serial.print(anchors[i].num_of_send_counter);
        Serial.print("    ");
        Serial.println(anchors[i].distance_counter_max);
        #endif
        ///////////When more then three anchors are used but only 3 were measured set the anchors not measure equal for excel file//////////
        anchors[i].num_of_send_counter = anchors[synchornise].num_of_send_counter;
        anchors[i].distance_counter_max = anchors[synchornise].distance_counter_max;
        #ifdef DEBUG_SYNCHRONISE
        Serial.print("after:");
        Serial.print(anchors[i].num_of_send_counter);
        Serial.print("    ");
        Serial.println(anchors[i].distance_counter_max);
        #endif
    }
  
  #endif
}    
static void backspaceDistances()
{
  for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
      anchors[i].total_data = "back";
      anchors[i].done = true;
      cal_points_counter--;
    }
  #ifdef DEBUG_INTERRUPT
    Serial.print('B');
    i2cprint("B");
  #endif
  i2cMenu[1].status = false;
}

static void endProgram(void)
{
  _i2c.print("program ended", true);
  while(1)//keep program running until server interrupt has been handled(chip will be set to deepSleep there)
  {}
}