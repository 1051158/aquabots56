#include "Wifi.cpp"

static uint8_t cal_points_counter = 1;
static uint8_t anchors_to_calculate_counter = 0;
static uint8_t active_counter = 0;

//#define DEBUG_SYNCHRONIZE

/////////////////////////////////Function to send distances for the x-y calculation in python///////////////////////////////

static void resetAD()
{
    functionNumber = 0x10;

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
    _resetAnchors = true;
    //delay(200);
}

static void addAD()
{
  functionNumber = 0x11;
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

static void subAD()
{
  functionNumber = 0x12;
  antenna_delay -= ANTENNA_INTERVAL;
  #ifdef I2C
  String AD = "";
  AD = AD + antenna_delay;
  _i2c.print(AD.c_str(), true);
  #endif
  //change the antenna delay on tag
  DW1000.setAntennaDelay(antenna_delay);
  //when all the antenna delays on the secified coördinates have been tested:
  _subAD = false;
}

static void addDCM()
{
  functionNumber = 0x13;
  _addDCM = false;
}

//when enough distances have been found for triliteration a bool will be set so the string will be send
static void Rdy2Send()
{
  functionNumber = 0x08;
  anchors_to_calculate_counter = 0;
  rdy2send = true;
  //check menu when program is waiting for sending the data
  //wait untill python asks for a getRequest
  while(rdy2send && !_resetAnchors)
    checkMenuInterrupts();
  if(_addAD)
    addAD();
  if(_resetAD)
    resetAD();
  if(_addDCM)
  {
    addDCM();
  }
  if(_resetDCM)
  {
     distance_counter_max = DISTANCE_COUNTER_MIN;
     _resetDCM = false;
  }
}



static void checkForDistances()
{
  functionNumber = 0x05;
  //counter to make sure all anchors who send distances are synchronised  
  //check if python code has started yet if not skip measuring distance with making the interrupt bool false

  #ifdef DEBUG_INTERRUPT
    Serial.print('S');
    _i2c.print("S");
  #endif      
  if(!_resetAnchors)
  {
    #ifndef DEBUG_INTERRPUT
  //get ranges from all the anchors in a for statement
  for(uint8_t i = 0; i < MAX_ANCHORS;i++)
  {
  //check if the measured ID of the found distance is equal to the right anchor
  //also check if the found distance has been send already(for synchronising)
    if(!anchors[i].done && anchors[i].ID == DW1000Ranging.getDistantDevice()->getShortAddress() && !_resetAnchors)
    {
      Serial.print(functionNumber);
      //convert all the variables into a String to send over wifi
      if(setDistanceIfRegisterdAnchor && generateDistanceAndTimer(i) && !_resetAnchors)
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
      Serial.print("anchorcount" );
      Serial.println(anchors_to_calculate_counter);
      //When 3 anchors are found stop measuring(to increase speed)
      if(anchors_to_calculate_counter >= 3 && !_resetAnchors)
        break;
    //when all the data of the antenna_delay for one anchor is done a counter is used for synchronisation
    }
  }
  if(anchors_to_calculate_counter >=3 && !_resetAnchors)
  {
    Rdy2Send();
    synchronizeAnchors();
  }
    
    
    /*after the getRequest is send(Server.on("/anchor") interrupt is triggered) check if any options need to be changed.
    Python always sends a change before it asks 4 message so the interrupt bool to change the AD or DCM will be set true
    */ 
  #ifdef DEBUG_SYNCHRONIZE
  Serial.print("delay");
  Serial.println(change_delay_counter);
  #endif
  #endif
  }
}

static void endProgram(void)
{

  #ifdef I2C
  _i2c.print("program ended", true);
  #endif

  while(1)//keep program running until server interrupt has been handled(chip will be resetted there)
  {}
}


static void checkMenus()
{
  functionNumber = 0x04;
    if(i2cMenu[END_CODE].status)
    {
      endProgram();
    }

  //check first if there are at least 3 anchors
  for(uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    if(anchors[i].active)
      {
        active_counter++;
      }
  }          
    if(active_counter>=3 && !_resetAnchors)
        checkForDistances();
}