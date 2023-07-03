#include "Wifi.cpp"
#include <vector>

static uint8_t cal_points_counter = 1;
static uint8_t anchors_to_calculate_counter = 0;
static uint8_t active_counter_1 = 0;
static unsigned long sendTime = 0;
static unsigned long sendTime_1 = 0;
// #define DEBUG_SYNCHRONIZE

static bool print = true;

/////////////////////////////////Function to send distances for the x-y calculation in python///////////////////////////////
#ifdef TYPE_TAG
#ifdef AD_TEST
static void resetAD()
{
  functionNumber = 0x0A;
  if (_debugSerial)
    Serial.println(functionNumber);
  /////Reset the anchor delay to minimal to redo the test at a different coördinate//////////////
  antenna_delay = ANTENNA_DELAY_START;

// When it changes the tester will notice on the i2c screen
#ifdef X_Y_TEST
  String AD = "";
  // The x_y_points are being converted to a string
  AD = AD + '(' + x_y_points[cal_points_counter][0] + ',' + x_y_points[cal_points_counter][1];
#ifdef Z_TEST
  AD = AD + ',' + x_y_points[cal_points_counter][2];
#endif
  AD = AD + ')';
  _i2c.print(AD.c_str(), true);
  // a counter is used to synchronise the right x_y_points array for the AD-string
  cal_points_counter++;
  // For reseting the cal_points_counter
  if (cal_points_counter >= MAX_CAL_DIS)
  {
    cal_points_counter = 0;
  }
#endif
  // the i2c menu interrupt bool will be set 0
  // bool to reset AD will be set false
  _resetAD = false;
  // put the new AD in the tag
  DW1000.setAntennaDelay(antenna_delay);
  // reset all the anchors
  _resetAnchors = true;
  // delay(200);
}

static void addAD()
{
  functionNumber = 0x0B;
  antenna_delay += ANTENNA_INTERVAL;
// When it changes the tester will notice on the i2c screen
#ifdef I2C
  String AD = "";
  AD = AD + antenna_delay;
  _i2c.print(AD.c_str(), true);
#endif
  // change the antenna delay on tag
  DW1000.setAntennaDelay(antenna_delay);
  // when all the antenna delays on the secified coördinates have been tested:
  _addAD = false;
}

static void subAD()
{
  functionNumber = 0x0C;
  antenna_delay -= ANTENNA_INTERVAL;
#ifdef I2C
  String AD = "";
  AD = AD + antenna_delay;
  _i2c.print(AD.c_str(), true);
#endif
  // change the antenna delay on tag
  DW1000.setAntennaDelay(antenna_delay);
  // when all the antenna delays on the secified coördinates have been tested:
  _subAD = false;
}
#endif

static void addDCM()
{
  functionNumber = 0x0D;
  _addDCM = false;
}

// when enough distances have been found for triliteration a bool will be set so the string will be send
static void Rdy2Send()
{
  uint8_t anchorCounter = 0;
  anchor helpAnchors[MAX_ANCHORS];
  functionNumber = 0x03;
  if (_debugSerial)
    Serial.println(functionNumber);
  anchors_to_calculate_counter = 0;
  rdy2send = true;
  for (uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    if (anchors[i].distance > 0)
    {
      helpAnchors[anchorCounter] = anchors[i];
      anchorCounter++;
      Serial.printf("helpAnchors[%u].distance = %f", anchorCounter, helpAnchors[anchorCounter].distance);
    }
  }
  Serial.print('\n');
  if (anchorCounter >= 3)
  {
    x_y_cal(helpAnchors[0], helpAnchors[1], helpAnchors[2]);
    if (i2cMenu[START_SEND].status && x > 0 && y > 0 && x < ANCHOR_X_4 + 3 && y < ANCHOR_Y_4 + 3)
    {
      // while(rdy2send && !_resetAnchors && i2cMenu[START_SEND].status)
      // checkMenuInterrupts();

      String Counter = "";
      sendTime = millis();
      sendTime = sendTime - sendTime_1;
      Counter = sendTime;
      Counter += "ms";
      _i2c.print(Counter.c_str(), true);
      _i2c.enter();
      Counter = '(' + String(x) + ',' + String(y) + ')';
      sendTime_1 = millis();
      _i2c.print(Counter.c_str(), false);
      int verstuur_x = x * 100;
      int verstuur_y = y * 100;
      int distances[MAX_ANCHORS] = {0};
      for (uint8_t i = 0; i < MAX_ANCHORS; i++)
      {
        distances[i] = anchors[i].distance * 100;
      }

      String Counter1 = "";
      Counter1 = '(' + String(verstuur_x) + ',' + String(verstuur_y) + ")\n";
      Serial.write(Counter1.c_str());
      Serial.print(Counter1);
    }
  }
// check menu when program is waiting for sending the data
// wait untill python asks for a getRequest
#ifdef AD_TEST
  if (_addAD)
    addAD();
  if (_resetAD)
    resetAD();
#endif
  if (_addDCM)
    addDCM();
  if (_resetDCM)
  {
    distance_counter_max = DISTANCE_COUNTER_MIN;
    _resetDCM = false;
  }
}

static void checkForDistances()
{
  functionNumber = 0x05;
  if (_debugSerial)
    Serial.print(functionNumber);

#ifdef DEBUG_INTERRUPT
  Serial.print('S');
  _i2c.print("S");
#endif
  if (!_resetAnchors)
  {
    // get ranges from all the anchors in a for statement
    for (uint8_t i = 0; i < MAX_ANCHORS; i++)
    {
      // check if the measured ID of the found distance is equal to the right anchor
      // also check if the found distance has been send already(for synchronising)
      if (anchors[i].distance == 0 && anchors[i].ID == DW1000Ranging.getDistantDevice()->getShortAddress() && !_resetAnchors && i2cMenu[START_SEND].status)
      {
        if (_debugSerial)
          Serial.print(functionNumber);
        // convert all the variables into a String to send over wifi
        if (generateDistanceAndTimer(i) && !_resetAnchors)
        {
          anchors[i].done = true;
          anchors_to_calculate_counter++;
          if (_debugSerial)
            Serial.printf("Anchor ID: %u \t registered\n");
        }
        else
        {
          anchors[i].total_data = "";
          anchors[i].distance = 0;
          if (_debugSerial)
            Serial.printf("Anchor ID: %u \tnot registered\n");
        }

        // when all the data of the antenna_delay for one anchor is done a counter is used for synchronisation
      }
      else
      {
        DW1000Ranging.getDistantDevice()->getRange();
      }
    }
    if (_debugSerial)
    {
      Serial.printf("anchorCount = %d", anchors_to_calculate_counter);
    }
    if (anchors_to_calculate_counter >= 3 && !_resetAnchors)
    {
      Rdy2Send();
      synchronizeAnchors();
    }
  }
}

static void endProgram(void)
{

#ifdef I2C
  _i2c.print("program ended", true);
#endif

  while (1) // keep program running until server interrupt has been handled(chip will be resetted there)
  {
  }
}

static void checkActivity()
{
  functionNumber = 0x04;
  if (i2cMenu[END_CODE].status)
  {
    endProgram();
  }

  // check first if there are at least 3 anchors
  active_counter = 0;
  for (uint8_t i = 0; i < MAX_ANCHORS; i++)
  {
    if (anchors[i].active)
    {
      active_counter++;
    }
  }
  checkForDistances();
  if (active_counter != active_counter_1)
  {
    String AC = "";
    AC = AC + "active: " + active_counter;
    _i2c.print(AC.c_str(), true);
    active_counter_1 = active_counter;
  }
}
#endif