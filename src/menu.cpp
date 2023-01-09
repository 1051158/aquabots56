#include <Arduino.h>

#define MENUSIZE 4
#define MAX_MENU_STRLEN 20

#define START_SEND 0
#define BACKSPACE 1
#define EXCEL_MODE 2
#define END_CODE 3

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
