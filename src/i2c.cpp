#include "U8g2lib.h"
#include <string.h>
#include <Arduino.h>
#include <stdio.h>

#define I2C
#define MAX_STRLEN 50 
#define MAX_ANCHORS 3

#define MAX_X_POS 128
#define MAX_Y_POS 32

//#define DEBUG_I2C_PRINT

#ifdef I2C
static U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

static bool clear;
static uint8_t x;
static uint8_t y;

class i2c
{
    public:
        static void settings(void)
        {
            u8g2.begin();
            u8g2.setFont(u8g2_font_ncenB08_tr);            
            u8g2.setFontPosTop();
            x = 0;
            y = 0;
            Serial.print("i2c");
            //init the configuration
            //_i2c.print("start");
        }
        static void print(const char* total_data, bool clear_screen)
        {
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
                    u8g2.drawStr(x,y,write_total_data);
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
                    u8g2.drawStr(x,y,write_total_data);
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
                    u8g2.drawStr(x,y,write_total_data);
                    u8g2.sendBuffer();
                    x = x + ((i - hulp_i)*3);
                    if(x >= MAX_X_POS)
                        enter();
                    //delay(1000);
                }
            }
        }
        static void clear(void)
        {
            x = 0;
            y = 0;
            u8g2.clearBuffer();
            u8g2.clearDisplay();
        }
        static bool tab(uint8_t pos)
        {
            x = x + 20 + pos*5;
            if(x>=MAX_X_POS)
            {
                enter();
            }
            
            return true;
        }
        static bool enter(void)
        {
            y+=9;
            x = 0;
            if(y>=MAX_Y_POS)
            {
                clear();
                return false;
            }
            return true;
        }
};
    static i2c _i2c;
