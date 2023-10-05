#include <Arduino.h>
#include "I2C.cpp"

struct menu
{
    bool status;
    String menuName;
};
//to add a menu fill in a new breacket in the i2cMenu struct ,{bool status, String menuName}
static menu i2cMenu[] = {
                                {false, "start_send"},
                                {false, "Show_ADelay"},
                                {false, "showIP"},
                                {false, "end_code"}
                                };

