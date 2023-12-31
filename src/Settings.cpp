/////Type of test////////////

////////////////////Choose between X_Y_(Z) Test or Rangetest(with one anchor)//////////////////////////
#define X_Y_TEST
// #define RANGETEST

////////////////////comment Z test if the map is 2D////////////////////////////////////////////////////
#ifdef X_Y_TEST
#define Z_TEST
#endif

///////////////////For debugging/////////////////////////////
#define FUNCTIONNAME_DEBUG
static bool _debugSerial = false;

/////Anchor or tag////////////

#define TYPE_TAG
// #define TYPE_ANCHOR

#define AD_TEST

///////////////////////anchor info for the tag(change coordinates for the right real-time situation)/////////////////////
// give the coordinates and ID's of the anchors

// ID of Anchor #1 (Do not change!!)
#define ANCHOR_ID_1 4369

// coordinates of anchor #1
#define ANCHOR_X_1 0
#define ANCHOR_Y_1 0
#define ANCHOR_Z_1 0

// ID of Anchor #2 (Do not change!!)
#define ANCHOR_ID_2 8738

// coordinates of anchor #2
#define ANCHOR_X_2 10
#define ANCHOR_Y_2 0
#define ANCHOR_Z_2 0

// ID of Anchor #3 (Do not change!!)
#define ANCHOR_ID_3 13107

// coordinates of anchor #3
#define ANCHOR_X_3 0
#define ANCHOR_Y_3 12.7
#define ANCHOR_Z_3 0

// ID of Anchor #4 (Do not change!!)
#define ANCHOR_ID_4 17476

// coordinates of anchor #4
#define ANCHOR_X_4 10
#define ANCHOR_Y_4 14
#define ANCHOR_Z_4 0

#define X 0
#define Y 1
#define Z 2

// longest range measureable in the area(to filter wrong measurements).
#define LONGEST_RANGE 22.3

// Choose the amount of anchors supported
#ifdef RANGETEST
#define MAX_RANGE_DIS 10
#define MAX_ANCHORS 1
static float range_points[MAX_RANGE_DIS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
#endif

#ifdef X_Y_TEST
#define MAX_ANCHORS 4
// GIVE THE NUMBER OF DISTANCES THAT ARE BEING USED FOR CALIBRATION
#ifndef Z_TEST
#define MAX_CAL_DIS 6
static float x_y_points[MAX_CAL_DIS][2] = {{0.25, 0.25}, {0.5, 0.5}, {0.75, 0.75}, {1, 1}, {1.25, 1.25}, {1.5, 1.5}};
#endif
#endif

// uncomment if the tag should be controlled by servers
#define SERVER_CONTROLLER

#define Y_COOR 2
#define Z_COOR 0.84

#ifdef Z_TEST
#define MAX_CAL_DIS 3
static float x_y_points[MAX_CAL_DIS][3] = {{5, 10, Z_COOR}, {5, 15, Z_COOR}, {5, 5, Z_COOR}};
#endif

///////////////////////to program the right anchor///////////////////////////////////////////////////////////
#ifdef TYPE_ANCHOR
// #define ANCHOR_1
// // valeus for the right anchor for the void setup() function
// #ifdef ANCHOR_1
// #define ANTENNA_DELAY 16550 // BEST ANTENNA DELAY ANCHOR #1
// #define UNIQUE_ADRESS "11:11:5B:D5:A9:9A:E2:9C"
// #endif

// #define ANCHOR_2
// // valeus for the right anchor for the void setup() function
// #ifdef ANCHOR_2
// #define ANTENNA_DELAY 16550 // BEST ANTENNA DELAY ANCHOR #2
// #define UNIQUE_ADRESS "22:22:5B:D5:A9:9A:E2:9C"
// #endif

// #define ANCHOR_3
// // valeus for the right anchor for the void setup() function
// #ifdef ANCHOR_3
// #define UNIQUE_ADRESS "33:33:5B:D5:A9:9A:E2:9C"
// #define ANTENNA_DELAY 16550 // BEST ANTENNA DELAY ANCHOR #3
// #endif

// // #define ANCHOR_4
// // valeus for the right anchor for the void setup() function
// #ifdef ANCHOR_4
// #define ANTENNA_DELAY 16550 // BEST ANTENNA DELAY ANCHOR #4
// #define UNIQUE_ADRESS "44:44:5B:D5:A9:9A:E2:9C"
// #endif

// // #define ANCHOR_5
// #ifdef ANCHOR_5
// #define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #4
// #define UNIQUE_ADRESS "55:55:5B:D5:A9:9A:E2:9C"
// #endif

// #ifdef ANCHOR_6
// #define ANTENNA_DELAY 16384 // BEST ANTENNA DELAY ANCHOR #4
// #define UNIQUE_ADRESS "66:66:5B:D5:A9:9A:E2:9C"
// #endif
#endif

////Choose measure mode (has to be the same with the other chips!!)////
// #define LOWPOWER
#define ACCURACY

// define pins
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

////////////////////Antenna_delay options (will integrade immediatly with python)///////////////////////////////////////////

#define RESET_DISTANCE_COUNTER_MAX_VALUE 2 // value to reset distance counter max to DISTANCE_COUNTER_MIN
#define DISTANCE_COUNTER_MIN 1             // will reset variable distance counter max when RESET_DISTANCE_COUNTER_MAX_VALUE is reached
#define DISTANCE_COUNTER_INTERVAL 1        // will be added to variable distance counter max in the anchor struct

#define ANTENNA_INTERVAL 5 // interval between 2 antenna delays

#define ANTENNA_DELAY_START 16385 // start value antenna delay
#define ANTENNA_DELAY_END 16600   // end value antenna delay

///////////////////////////////////////I2C settings////////////////////////////////////

#define I2C
#define MAX_STRLEN 120

#define MAX_X_POS 128
#define MAX_Y_POS 32

//////////////////////////////////////Menu settings/////////////////////////////////////

// set the size of different menu options//////
#define MENUSIZE 4
#define MAX_MENU_STRLEN 20
////////Give every Menu number a define to make the code clearer
#define START_SEND 0
#define SHOW_ADELAY 1
// ToDo change excel_mode to drawMode
#define SHOW_IP 2
#define END_CODE 3

///////////////////////////////////////Wifi settings//////////////////////////////////////

// if there is no wifi use 2 esp32's to send and receive data and uncomment underneath define
// #define WIFI_AP_ON

// if there is wifi uncomment
#define WIFI_EXTERN_ON

// uncomment to start the wifi test
#define WIFI_TEST
