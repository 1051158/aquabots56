#include "U8g2lib.h"
#include <string.h>
#include <Arduino.h>
#include <stdio.h>

#define I2C
#define STRLEN 20 
#define MAX_ANCHORS 3
//#define DEBUG_I2C_PRINT

#ifdef I2C
static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

class i2c
{
    public:
        static void settings(void)
        {
            u8g2.begin();
            u8g2.setFontPosTop();
            Serial.print("i2c");
            //init the configuration
        }
        static void print(const char* total_data)
        {
            char* write_total_data = (char*)malloc(strlen(total_data)*sizeof(char));
            strcpy(write_total_data, total_data);            
            u8g2.setFont(u8g2_font_fancypixels_tf);
            u8g2.drawStr(0,0,write_total_data);
            u8g2.sendBuffer();
            u8g2.clearBuffer();
        }
};
