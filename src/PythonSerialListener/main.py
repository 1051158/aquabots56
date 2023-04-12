import numpy as np
import trilateration
import sys
import draw
import time
import request
import xls
import keyboard
from timer import Timer

# define comport to which the tag is connected
comPort = "COM12"



def WifiReadLoop():
    #ask for the array of data coming from the tag
    cal_distances = request.getRequest(':85/caldis')
    WiFiString_1 = ''
    tagInfo, Coordinates, x_array, y_array, z_array, distance_array = request.getValues(cal_distances)
    antenna_delay = int(tagInfo.AD_start)
    AD_interval = int(tagInfo.AD_interval)
    functionNameArray = np.array(['setup', 'newRange', 'Rdy2Send', 'checkMenus', 'checkForDistances', 'setDistanceIfRegisteredAnchor', 'generateDistanceAndTimer', \
                                  'synchornizeAnchors', 'resetAnchors', 'resetAD', 'addAD', 'subAD', 'addDCM'])

    #get the excel settings and make a workbook and worksheet
    wbk, wks, Settings, fig = xls.excelSettings(Coordinates)

    if Settings.xyz_test == False:
        for i in range (6):
            r1 = tagInfo.distance_array[0][i]
            r2 = tagInfo.distance_array[1][i]
            r3 = tagInfo.distance_array[2][i]
            print(trilateration.triliterationnew3D(x_array, y_array, z_array, r1, r2, r3))

    #use different lay out when using 1 anchor
    if tagInfo.max_anchors == '1':
        xls.first_rows_excel(wks, tagInfo, distance_array, Settings)

    #or no antenna delay test
    else:
        #make a worksheet for multiple anchors and with the option AD change
        xls.first_rows_excel_multiple_AD(wks, tagInfo, distance_array, Settings)

    if Settings.dbgFirstRow:
        print('ended FirstRowTest')
        endCode(wbk)
    # short delay to let the tag add the anchors without getting disturbed by server_interrupts

    #reset the AD of tag to make sure the right AD is set in tag for the test
    t = Timer()
    t1 = Timer()
    t2 = Timer()
    t.start()
    request.getRequest(':80/resetAD')
    request.getRequest(':80/anchors')
    print('code begins')
    t.stop()
    tijd = 0

    while True:
        t2.start()
        time.sleep(0.01)
        #check if the code had enough data to change position and antenna delay
        if Settings.num_of_send_counter > Settings.nos:
            Settings.num_of_send_counter = 1
            antenna_delay, wks, Settings, t1 = request.callRightServer(Coordinates, wbk, wks, distance_array, tagInfo, Settings, t, t1)
        #if that's not the case...

            # check is the program wnats new data. the bool can be set by pressing 's' key or by pressing 'start_send' on the tag
        if Settings.startSend:
            WiFiString = request.getRequest(':80/anchors')
            # print the incoming string if debug is desired
            if Settings.dbgInteg:
                if (WiFiString != WiFiString_1):
                    print(WiFiString)
                WiFiString_1 = WiFiString

            # check if there are 3 anchors that measured a distance(WiFistring should contain ID and d)
            if 'ID' in WiFiString and 'd' in WiFiString and 'dC' not in WiFiString:
                if Settings.dbgXls:
                    print('y', Settings.position_y)
                    print('x', Settings.num_of_send_counter)
                xls.put_in_xls(WiFiString, wks, x_array, y_array, z_array, tagInfo, fig, Settings)
                Settings.num_of_send_counter += 1

                if Settings.dbgXls:
                    print(Settings.num_of_send_counter)
                sleepTime = 0.2
            elif '2rst' in WiFiString:
                sleepTime = 0.5
            elif 'dC' in WiFiString:
                print(WiFiString)

            elif int(WiFiString) > 0x00 and int(WiFiString) < 0x0E:
                print(functionNameArray[int(WiFiString)])

            # if the code is not ready to send check if keyboard keys are pressed and any buttons on tag are pressed
        if not Settings.startSend:
            # ask to tag if a menubutton has been pressed yet
            WiFiString = request.getRequest(':83/sendMenu')
            # if awnser is '0' send button has been pressed
            if WiFiString == '0':
                Settings.startSend = True
                t1.start()
            if keyboard.is_pressed('s'):
                request.getRequest(':84/start')
                Settings.startSend = True
                t1.start()
            # if awnser of tag is '1' back button has been pressed

            if WiFiString == '1' or keyboard.is_pressed('b'):
                Settings.wks_count -= 1
                wks = wbk.get_worksheet_by_name(Coordinates[Settings.wks_count])
                Settings.position_y = 1
                Settings.num_of_send_counter = 0
                xls.first_rows_excel(wks, distance_array, tagInfo, Settings)

            # todo Wifistring  = 2 for draw

            # if awnser of tag is '3' end button has been pressed
            if WiFiString == '3' or keyboard.is_pressed('e'):
                print('end', request.getRequest(':86/restart'))
                endCode(wbk)
        t2.stop()
def endCode(wbk):
    wbk.close()
    sys.exit(0)

WifiReadLoop()
# start eventLoop
