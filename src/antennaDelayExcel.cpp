#include "Wifi.cpp"

static uint8_t change_delay_counter = 0;
static uint8_t cal_points_counter = 1;
static uint8_t anchors_to_calculate_counter = 0;


//#define DEBUG_SYNCHRONIZE


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
    i2cMenu[0].status = false;
    end_done = true;
    while(end_done)
    {
      checkInterrupts();
    }
    //delay(200);
  }
  for(uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
    anchors[i].hulp_change_delay = false;
    }
}

static void SendDistancesAD()
{
  //counter to make sure all anchors who send distances are synchronised  
  change_delay_counter = 0;
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
      //when the function has not been succeeded empty the string in anchor struct
      if(anchors[i].total_data == "not")
      {
        //Serial.println('n');
        anchors[i].total_data = "";
        anchors[i].done = false;
      }
      else
        {
          anchors[i].done = true;
          anchors_to_calculate_counter++;
          //Serial.print(anchors_to_calculate_counter);
        }
      if(anchors_to_calculate_counter >= 3)
    break;
    //when all the data of the antenna_delay for one anchor is done a counter is used for synchronisation
  }
  }
  if(anchors_to_calculate_counter >=3)
  {
    uint8_t anchornumbers[3];
    uint8_t j = 0;
    if(anchors_to_calculate_counter >= 4)
    {
      uint8_t highest_anchor_number;
      float highest_anchor_value = anchors[0].distance;
      for(uint8_t i = 0; i < MAX_ANCHORS; i++)
      {
        if(highest_anchor_value < anchors[i].distance)
        {
          highest_anchor_value = anchors[i].distance;
          highest_anchor_number = i;
        }
      }
    anchors[highest_anchor_number].done = false;
    Serial.print("highest anchor = ");
    Serial.println(highest_anchor_number);
    Serial.print("highest anchor value = ");
    Serial.println(highest_anchor_value);
    for(uint8_t i = 0; i<MAX_ANCHORS;i++)
    {
      if(anchors[i].done)
      {
        anchornumbers[j] = i;
        Serial.println(anchornumbers[j]);
        j++;
      }
    }
    }
    rdy2send = true;
    anchors_to_calculate_counter = 0;
    while(rdy2send)
    {checkInterrupts();}
    if(done_send)
    {
      for(int i = 0; i < MAX_ANCHORS; i++)
      {
        anchors[i].done = false;
        anchors[i].distance_counter = 0;
        anchors[i].distance = 0;
        if(syncNumber != i)
        {
          anchors[i].num_of_send_counter = anchors[syncNumber].num_of_send_counter;
          anchors[i].distance_counter_max = anchors[syncNumber].distance_counter_max; 
        }  
      }
    } 
    done_send = false;
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