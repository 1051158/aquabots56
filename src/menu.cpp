#include <Arduino.h>

#define MENUSIZE 4
#define MAX_MENU_STRLEN 20

#define START_SEND 0
#define BACKSPACE 1
#define EXCEL_MODE 2
#define END_CODE 3

struct menu
{
    int menuNumber;
    bool status;
    String menuName;
};

static menu i2cMenu[MENUSIZE] = {
                                {0, false, "start_send"},
                                {1, false, "back_send"},
                                {2, false, "excel_mode"},
                                {3, false, "end_code"}
                                };
