#include <Arduino.h>
#include "i2c.cpp"

struct menu
{
    bool status;
    String menuName;
};

static menu i2cMenu[] = {
                                {false, "start_send"},
                                {false, "Show_ADelay"},
                                {false, "showIP"},
                                {false, "end_code"}
                                };

