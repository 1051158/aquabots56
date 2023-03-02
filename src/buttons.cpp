#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include "menu.cpp"



struct button
{
  const uint8_t interrupt_pin;
  bool pressed;
};

//#define DEBUG_INTERRUPT

////////////////Choose type of device///////////////////////////////////////////////////////////////

#ifdef I2C

static button button_up = {15,false};//interrupt button to send data to pyhonscript
static button button_enter{14, false};//interrupt to skip latest uart value
static button button_down = {13,false};//interrupt to stop python script


////////////////////////millis() variables//////////////////////////
static unsigned long button_time = 0;  
static unsigned long last_button_time = 0; 

#ifdef TYPE_TAG
static void IRAM_ATTR upCode(void)//interruptfunction to go 1 row up in excel
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    button_up.pressed = true;   
    last_button_time = button_time;
  }
}

static void IRAM_ATTR downCode(void)//interrupt code to refresh the http page
{
    button_time = millis();
if (button_time - last_button_time > 250)
{
    button_down.pressed = true;
    last_button_time = button_time;
}
}

static void IRAM_ATTR enterCode(void)//interrupt function to stop the code of the tag
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  {
    button_enter.pressed = true; // ToDo change pin number hardware
  }
}
#endif

static void interruptfunctions(void)
{
  #ifdef TYPE_TAG
  pinMode(button_enter.interrupt_pin, INPUT_PULLUP);//enable interrupt to send data when green button is pressed
  attachInterrupt(button_enter.interrupt_pin, enterCode, FALLING);
  pinMode(button_down.interrupt_pin, INPUT_PULLUP);//enable interrupt to use 'backspace' in python
  attachInterrupt(button_down.interrupt_pin, downCode, FALLING);
  pinMode(button_up.interrupt_pin, INPUT_PULLUP);//enable interrupt to end code when red button is pressed
  attachInterrupt(button_up.interrupt_pin, upCode, FALLING);
  #endif
}

static int i2c_menuNumber = 0;

//check which button has been pressed to scroll into i2c menu
static void checkMenuInterrupts(void)
{
  //go up
  if(button_enter.pressed && !button_down.pressed && !button_up.pressed)
  {
    if(!i2cMenu[i2c_menuNumber].status)
      i2cMenu[i2c_menuNumber].status = true;
    if(i2c_menuNumber == START_SEND)
    {
      resetAnchors();
    }
  }
  //go down
  if(button_down.pressed && !button_up.pressed && !button_enter.pressed)
  {
    i2c_menuNumber--;
    if(i2c_menuNumber < 0)
      i2c_menuNumber = 3;
  }
  //press enter
  if(button_up.pressed && !button_enter.pressed && !button_down.pressed)
  {
    i2c_menuNumber++;
    if(i2c_menuNumber > 3)
      i2c_menuNumber = 0;
  }
  if(button_down.pressed || button_up.pressed || button_enter.pressed)
  {
    String help = "";
    if(i2cMenu[i2c_menuNumber].status)
    {
      help = help + i2cMenu[i2c_menuNumber].menuName + " = on";
      _i2c.print(help.c_str(), true);
    }
    else
    {
      help = help + i2cMenu[i2c_menuNumber].menuName + " = off";
      _i2c.print(help.c_str(), true);
    }
    button_down.pressed = false;
    button_up.pressed = false;
    button_enter.pressed = false;
  }
}
#endif