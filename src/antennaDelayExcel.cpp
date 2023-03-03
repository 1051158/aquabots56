#include "Wifi.cpp"

static uint8_t cal_points_counter = 1;
static uint8_t anchors_to_calculate_counter = 0;
static uint8_t active_counter = 0;

//#define DEBUG_SYNCHRONIZE


/////////////////////////////////Function to send distances for the x-y calculation in python///////////////////////////////

static void resetAD()
{
    functionName = "strt";

    /////Reset the anchor delay to minimal to redo the test at a different coördinate//////////////
    antenna_delay = ANTENNA_DELAY_START;

    //When it changes the tester will notice on the i2c screen
    #ifdef X_Y_TEST
    String AD = "";
    //The x_y_points are being converted to a string
    AD = AD + '(' + x_y_points[cal_points_counter][0] + ',' + x_y_points[cal_points_counter][1] + ')';
    _i2c.print(AD.c_str(), true);      
    //a counter is used to synchronise the right x_y_points array for the AD-string 
    cal_points_counter++;
    //For reseting the cal_points_counter
    if(cal_points_counter >= MAX_CAL_DIS)
    { 
      cal_points_counter = 0;
    }
    #endif
    //the String in every struct from the array will send "end" for the integration with python
    i2cMenu[0].status = false;
    _resetAD = false;
    resetAnchors();
    //delay(200);
}

static void addAD()
{
  functionName = "cAD";
  antenna_delay += ANTENNA_INTERVAL;
  //When it changes the tester will notice on the i2c screen
  #ifdef I2C
  String AD = "";
  AD = AD + antenna_delay;
  _i2c.print(AD.c_str(), true);
  #endif
  //change the antenna delay on tag
  DW1000.setAntennaDelay(antenna_delay);
  //when all the antenna delays on the secified coördinates have been tested:
  _addAD = false;
}

static void SendDistances()
{
  //counter to make sure all anchors who send distances are synchronised  
  //check if python code has started yet if not skip measuring distance with making the interrupt bool false
  if(!start_test)
  {
    #ifdef I2C
    _i2c.print("not connected", true);
    #endif
    i2cMenu[0].status = false;
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
      if(setDistanceIfRegisterdAnchor && generateWiFiString(i))
      { 
        anchors[i].done = true;
        anchors_to_calculate_counter++; 
      }
      else
      {
        //Serial.println('n');
        anchors[i].total_data = "";
        anchors[i].done = false;
        //Serial.print("not registered");
      }
      
      //When 3 anchors are found stop measuring(to increase speed)
      if(anchors_to_calculate_counter >= 3)
        break;
    //when all the data of the antenna_delay for one anchor is done a counter is used for synchronisation
    }
    Serial.println(anchors_to_calculate_counter);


  }
  if(anchors_to_calculate_counter >=3)
  {
    functionName = "atcc";
    anchors_to_calculate_counter = 0;
    uint8_t anchornumbers[3];
    uint8_t j = 0;
    if(anchors_to_calculate_counter >= 4)
    {
      Serial.println("4 anchors");
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
    //check menu when program is waiting for sending the data
    functionName = "rdy";
    //wait untill python asks for a getRequest
    while(rdy2send)
      checkMenuInterrupts();
    //after the getRequest is send(Server.on("/anchor") interrupt is triggered) check if any options need to be changed(python always sends a change before it asks 4 message)
    if(_addAD)
      addAD();
    if(_resetAD)
      resetAD();
    if(_addDCM)
      {
        distance_counter_max += DISTANCE_COUNTER_INTERVAL;
        Serial.println("addDCM");
        _addDCM = false;
      }
    if(_resetDCM)
    {
       distance_counter_max = DISTANCE_COUNTER_MIN;
       _resetDCM = false;
    }
    synchronizeAnchors();
  }
  
  #ifdef DEBUG_SYNCHRONIZE
  Serial.print("delay");
  Serial.println(change_delay_counter);
  #endif
  //when for statement to check distances is done check if Antenna Delay needs to be changed
     
//if the AD doesn't need to be changed there will be checked if the anchors are done with measuring
  #endif
  }


//to skip last excel 
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

  #ifdef I2C
  _i2c.print("program ended", true);
  #endif

  while(1)//keep program running until server interrupt has been handled(chip will be set to deepSleep there)
  {}
}


static void checkMenus()
{
    if(i2cMenu[END_CODE].status)
    {
      endProgram();
    }

  //check first if there are at least 3 anchors
  for(uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    if(anchors[i].active)
      {active_counter++;}
  }

  if(i2cMenu[EXCEL_MODE].status)
    {
      //when the start_send in menu is selected start sending
      if(i2cMenu[START_SEND].status && active_counter >= 3)
      { 
        //Serial.print('3');   
        SendDistances();
      }
      if(i2cMenu[BACKSPACE].status && active_counter >= 3)
      {
        backspaceDistances();
      }
        //#ifdef USE_SERIAL
          //outputDataUart();
      }
    if(i2cMenu[START_SEND].status && active_counter >= 3)//press the interrupt button to start measurement
    { 
      //Serial.print('3');
    }
    active_counter = 0;
}