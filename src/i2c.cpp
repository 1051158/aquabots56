#include "U8g2lib.h"
#include <string.h>
#include <Arduino.h>
#include <stdio.h>

#define MAX_STRLEN 120 
#define I2C

#define MAX_X_POS 128
#define MAX_Y_POS 32

//#define DEBUG_I2C_PRINT

#ifdef I2C
static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

static bool clear;
static uint8_t x_pos;
static uint8_t y_pos;

class i2c
{
    public:
    //all the necessary setting for the i2c screens
        static void settings(void)
        {
            u8g2.begin();
            u8g2.setFont(u8g2_font_ncenB08_tr);            
            u8g2.setFontPosTop();
            x_pos = 0;
            y_pos = 0;
            Serial.print("i2c");
            //init the configuration
            //_i2c.print("start");
        }
        static void print(const char* total_data, bool clear_screen)
        {
            //Serial.print("i2cprint");
            if(clear_screen)
                clear();
            uint16_t hulp_i = 0;
            char write_total_data[MAX_STRLEN];
            for(uint16_t i = 0; i<strlen(total_data);i++)
            {
                if(total_data[i] == '\n')
                {
                    for(uint8_t j = 0; j<i-hulp_i; j++)
                        write_total_data[j] = total_data[j + hulp_i];
                    write_total_data[i - hulp_i] = '\0';
                    hulp_i = i;
                    u8g2.setFont(u8g2_font_fancypixels_tf);
                    u8g2.drawStr(x_pos,y_pos,write_total_data);
                    u8g2.sendBuffer();
                    write_total_data[0] = '\0';
                    //
                    //u8g2.clearBuffer();
                    if(!enter())
                    {
                        break;
                    }
                }
                if(total_data[i] == '\t')
                {
                    for(uint8_t j = 0; j<i - hulp_i; j++)
                        write_total_data[j] = total_data[j + hulp_i];
                    write_total_data[i - hulp_i] = '\0';
                    u8g2.setFont(u8g2_font_fancypixels_tf);
                    u8g2.drawStr(x_pos,y_pos,write_total_data);
                    u8g2.sendBuffer();
                    write_total_data[0] = '\0';
                    //u8g2.clearBuffer();
                    //delay(1000);
                    if(!tab(i - hulp_i))
                    {
                        break;
                    }
                    hulp_i = i;
                }
                if(i == (strlen(total_data)-1))
                {
                    write_total_data[0] = '\0';
                    for(uint8_t j = 0; j<=i - hulp_i; j++)
                        write_total_data[j] = total_data[j + hulp_i];
                    write_total_data[i - hulp_i+1] = '\0';
                    u8g2.setFont(u8g2_font_fancypixels_tf);
                    u8g2.drawStr(x_pos,y_pos,write_total_data);
                    u8g2.sendBuffer();
                    x_pos = x_pos + ((i - hulp_i)*3);
                    if(x_pos >= MAX_X_POS)
                        enter();
                    //delay(1000);
                }
            }
        }
        static void clear(void)
        {
            x_pos = 0;
            y_pos = 0;
            u8g2.clearBuffer();
            u8g2.clearDisplay();
        }
        static bool tab(uint8_t pos)
        {
            x_pos = x_pos + 20 + pos*5;
            if(x_pos>=MAX_X_POS)
            {
                enter();
            }
            
            return true;
        }
        static bool enter(void)
        {
            y_pos+=9;
            x_pos = 0;
            if(y_pos>=MAX_Y_POS)
            {
                clear();
                return false;
            }
            return true;
        }
};
    static i2c _i2c;
