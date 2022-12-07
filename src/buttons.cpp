#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>

struct button
{
  const uint8_t interrupt_pin;
  bool pressed;
};

//#define DEBUG_INTERRUPT

////////////////Choose type of device///////////////////////////////////////////////////////////////

#define TYPE_TAG
//#define TYPE_ANCHOR

#ifdef TYPE_TAG
static button button_send = {17,false};//interrupt button to send data to pyhonscript
static button button_end = {5,false};//interrupt to stop python script
static button button_backspace{16, false};//interrupt to skip latest uart value
#endif
static button ACCURACY{15, false};
static button LOWPOWER{14, false};

////////////////////////millis() variables//////////////////////////
static unsigned long button_time = 0;  
static unsigned long last_button_time = 0; 

#ifdef TYPE_TAG
static void IRAM_ATTR backCode(void)//interruptfunction to go 1 row up in excel
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    button_backspace.pressed = true;
    last_button_time = button_time;
  }
}

static void IRAM_ATTR refresh(void)//interrupt code to refresh the http page
{
    button_time = millis();
if (button_time - last_button_time > 250)
{
    button_send.pressed = true;
    last_button_time = button_time;
}
}

static void IRAM_ATTR endCode(void)//interrupt function to stop the code of the tag
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    button_end.pressed = true; // ToDo change pin number hardware
  }
}
#endif

static void IRAM_ATTR LowPower(void)
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    LOWPOWER.pressed = true; // ToDo change pin number hardware
  }
} 

static void IRAM_ATTR Accuracy(void)
{
  button_time = millis();
  if (button_time - last_button_time > 250)//to prevent debouncing of the button
  { 
    ACCURACY.pressed = true; // ToDo change pin number hardware
  }
} 


static void interruptfunctions(void)
{
  #ifdef TYPE_TAG
  pinMode(button_send.interrupt_pin, INPUT_PULLUP);//enable interrupt to send data when green button is pressed
  attachInterrupt(button_send.interrupt_pin, refresh, FALLING);
  pinMode(button_backspace.interrupt_pin, INPUT_PULLUP);//enable interrupt to use 'backspace' in python
  attachInterrupt(button_backspace.interrupt_pin, backCode, FALLING);
  pinMode(button_end.interrupt_pin, INPUT_PULLUP);//enable interrupt to end code when red button is pressed
  attachInterrupt(button_end.interrupt_pin, endCode, FALLING);
  #endif
  pinMode(LOWPOWER.interrupt_pin, INPUT_PULLUP);
  attachInterrupt(LOWPOWER.interrupt_pin, LowPower, FALLING);
  pinMode(ACCURACY.interrupt_pin, INPUT_PULLUP);
  attachInterrupt(ACCURACY.interrupt_pin, Accuracy, FALLING);
}
