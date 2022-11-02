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

static void i2cSettings(void)
{
    u8g2.begin();
    u8g2.setFontPosTop();
    Serial.print("i2c");
    //init the configuration
}

static void i2cprint(const char* total_data, bool unit_test)
{
    /*#define DEBUG_I2C_print
    if(!unit_test)
    {
        uint8_t y_axis = 0;
        uint8_t x_axis = 0;
        #ifdef DEBUG_I2C_PRINT
        Serial.print('1');
        #endif
        char* hulp_total_data = (char*)malloc(strlen(total_data)*sizeof(char));
        #ifdef DEBUG_I2C_PRINT
        Serial.print('2');
        #endif    
        strcpy(hulp_total_data, total_data);
        #ifdef DEBUG_I2C_PRINT
        Serial.print('3');
        #endif
        const char s[2] = "\t";
        char* splitted;
        #ifdef DEBUG_I2C_PRINT
        Serial.print('4');
        #endif
        splitted = strtok(hulp_total_data, s);
        //char array to print the chars that were put into the function
        #ifdef DEBUG_I2C_PRINT
        Serial.print('5');
        #endif
        while(splitted != NULL && (y_axis < 40*MAX_ANCHORS || x_axis < 150))
        {
            u8g2.setFont(u8g2_font_fancypixels_tf);
            u8g2.drawStr(0,y_axis, splitted);
            Serial.println(splitted);
            splitted = strtok(NULL, s);
            y_axis+=40;
            #ifdef DEBUG_I2C_PRINT
            Serial.print(x_axis);
            Serial.print(y_axis);
            #endif
            if(y_axis>=200)
            {
                x_axis += 50;
            }
        }
        free(hulp_total_data);
    }
    else
    {*/
        //Serial.print("in");
        u8g2.setFont(u8g2_font_fancypixels_tf);
        u8g2.drawStr(0,0,total_data);
        u8g2.sendBuffer();
        u8g2.clearBuffer();
}