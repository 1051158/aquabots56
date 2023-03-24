#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include "menu.cpp"

#define DEBOUNCE_BUTTON_TIME 250

//struct that is used for all the interrupt functions
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
//interruptfunction to go to change the menuNumber to menuNumber+1
static void IRAM_ATTR upCode(void)
{
  button_time = millis();
  //if-statement to prevent debouncing of the button and pressing a second button while the first is still processing
  if (button_time - last_button_time > DEBOUNCE_BUTTON_TIME && !button_down.pressed && !button_enter.pressed)
  {
    //set the button bool to true 
    button_up.pressed = true;   
  }
  last_button_time = button_time;

}

//interrupt code to go change the menuNumber to menuNumber-1
static void IRAM_ATTR downCode(void)
{
    button_time = millis();
if (button_time - last_button_time > DEBOUNCE_BUTTON_TIME && !button_up.pressed && !button_enter.pressed)
{
  //set the button bool to true
    button_down.pressed = true;
}
//set the millis interval to a variable to measure the next interval(>250)
  last_button_time = button_time;
}

//interrupt function to press the number of menuNumber where it is currently at
static void IRAM_ATTR enterCode(void)
{
  button_time = millis();
  if (button_time - last_button_time > DEBOUNCE_BUTTON_TIME && !button_up.pressed && !button_down.pressed)//to prevent debouncing of the button
  {
    //set the button bool to true
    button_enter.pressed = true; 
  }
}
#endif

static void interruptfunctions(void)
{
  #ifdef TYPE_TAG
  //enable interrupt to switch the status of menuNumber
  pinMode(button_enter.interrupt_pin, INPUT_PULLUP);
  attachInterrupt(button_enter.interrupt_pin, enterCode, FALLING);
  pinMode(button_down.interrupt_pin, INPUT_PULLUP);//enable interrupt to use 'backspace' in python
  attachInterrupt(button_down.interrupt_pin, downCode, FALLING);
  pinMode(button_up.interrupt_pin, INPUT_PULLUP);//enable interrupt to end code when red button is pressed
  attachInterrupt(button_up.interrupt_pin, upCode, FALLING);
  #endif
}

static int i2c_menuNumber = 0;

/*check which button has been pressed to scroll into i2c menu and switch the status of the 
menuNumber if enter is pressed the reason it's done here instead of the interrupt itself is 
that the measurements of the position needs to be done before changing settings*/
static void checkMenuInterrupts(void)
{
  //press enter
  if(button_enter.pressed && !button_down.pressed && !button_up.pressed)
  {
    functionNumber = 0x04;
    //when enter is pressed on the interruptButtons change status to true if status was false
    if(!i2cMenu[i2c_menuNumber].status)
      i2cMenu[i2c_menuNumber].status = true;

    //if the status is true set is back to false
    //if(i2cMenu[i2c_menuNumber].status)
      //i2cMenu[i2c_menuNumber].status = false;

    //if the startSend menu has been entered reset the anchors for synchronisation
    if(i2c_menuNumber == START_SEND)
    {
      //send a string in reset for debugging to show in python that the code is on this piece of the code
      _resetAnchors = true;
    }
  }
  //go down in the i2c number menu
  if(button_down.pressed && !button_up.pressed && !button_enter.pressed)
  {
    i2c_menuNumber--;
    if(i2c_menuNumber < 0)
      i2c_menuNumber = 3;
  }
  //go up in the i2c number menu
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