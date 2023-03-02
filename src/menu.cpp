#include <Arduino.h>
#include "i2c.cpp"

struct menu
{
    bool status;
    String menuName;
};

static menu i2cMenu[MENUSIZE] = {
                                {false, "start_send"},
                                {false, "back_send"},
                                {true, "excel_mode"},
                                {false, "end_code"}
                                };

