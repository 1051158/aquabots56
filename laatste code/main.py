import numpy as np
import draw
import trilateration
import signal
import sys
import time
import request
import xls
import keyboard
import settings
from timer import Timer

# define comport to which the tag is connected
comPort = "COM12"

def handler(signum, frame):
    res = input("Ctrl-c was pressed. Do you really want to exit? y/n ")
    if res == 'y':
        sys.exit(0)

signal.signal(signal.SIGINT, handler)

def setDelays(anchornumber):
    Settings = settings.allSettings(Coordinate, anchornumber)
    request.setADelays(coordinates)

def setAccuracy(anchornumber):
    Settings = settings.allSettings(Coordinates)
    if anchornumber == 1:
        request.getRequest(Settings.IP_A1 + ':88/accuracy')

def setLowPower(anchornumber):
    Settings = settings.allSettings(Coordinates)


def WifiReadLoop():
    #restart the tag
    IPadress = 'http://172.31.83.43'
    #wait till restart is complete

    #ask for the tag info
    cal_distances = request.getRequest(IPadress + ':85/caldis')

    WiFiString_1 = ''
    firstSend = True
    #put the tagInfo in the tagInfo struct
    tagInfo, Coordinates, x_array, y_array, z_array, distance_array = request.getValues(cal_distances)

    #Array
    functionNameArray = np.array(['setup', 'newRange', 'Rdy2Send', 'checkMenus', 'checkForDistances', 'setDistanceIfRegisteredAnchor', 'generateDistanceAndTimer', \
                                  'synchornizeAnchors', 'resetAnchors', 'resetAD', 'addAD', 'subAD', 'addDCM'])

    #get the excel settings and make a workbook and worksheet
    wbk, wks, Settings, fig, ax, plt, coordinates, anchor1, anchor2, anchor3, anchor4, d1, d2, d3, d4 = settings.allSettings(Coordinates)
    #request.setADelays(Settings)

    if Settings.excel:
        # use different lay out when using 1 anchor
        if tagInfo.max_anchors == '1':
            xls.first_rows_excel(wks, tagInfo, distance_array, Settings)

        elif tagInfo.AD_start == tagInfo.AD_end:
            xls.first_rows_excel_multiple_anchor(wks, tagInfo, distance_array, Settings)
            # or no antenna delay test

        else:
            # make a worksheet for multiple anchors and with the option AD change
            xls.first_rows_excel_multiple_AD(wks, tagInfo, distance_array, Settings)
        if Settings.dbgFirstRow:
            print('ended FirstRowTest')
            endCode(wbk)
    # short delay to let the tag add the anchors without getting disturbed by server_interrupts

    #reset the AD of tag to make sure the right AD is set in tag for the test
    t = Timer()
    t1 = Timer()
    t2 = Timer()
    request.getRequest(IPadress + ':80/resetAD')
    request.getRequest(IPadress + ':80/anchors')
    t2.start()
    #main loop starting.
    print('code starts')
    while True:
        time.sleep(0.01)
        #check if the code had enough data to change position and antenna delay
        if Settings.num_of_send_counter > Settings.nos:
            Settings.num_of_send_counter = 0
            antenna_delay, wks, Settings, t, t1 = request.callRightServer(Coordinates, wbk, wks, distance_array, tagInfo, Settings, t, t1, IPadress)
        #if that's not the case...

            # check is the program wnats new data. the bool can be set by pressing 's' key or by pressing 'start_send' on the tag
        if Settings.startSend:
            WiFiString = request.getRequest(IPadress + ':80/anchors')
            # print the incoming string if debug is desired
            if WiFiString == 'notActive':
                print('not enough active!')
                time.sleep(1)
            else:
                if not Settings.excel:
                    print(WiFiString)
                # check if there are 3 anchors that measured a distance(WiFistring should contain ID and d)
                elif 'ID' in WiFiString and 'd' in WiFiString and 'dC' not in WiFiString and not firstSend and Settings.excel:
                    if Settings.dbgXls:
                        print('y', Settings.position_y)
                        print('x', Settings.num_of_send_counter)
                    D = xls.put_in_xls(WiFiString, wks, x_array, y_array, z_array, tagInfo, fig, Settings, t2, ax, plt)
                    Settings.num_of_send_counter += 1

                    if Settings.dbgXls:
                        print(Settings.num_of_send_counter)
                    sleepTime = 0.2


                elif '2rst' in WiFiString:
                    sleepTime = 0.5


                elif 'dC' in WiFiString:
                    print(WiFiString)


                elif Settings.dbgInteg:
                    if WiFiString != WiFiString_1:
                        t2.stop()
                        t2.start()
                        if int(WiFiString) > 0x00 and int(WiFiString) < 0x0E:
                            print(functionNameArray[int(WiFiString)])
                    WiFiString_1 = WiFiString

                # if-statement to reset nos_counter after the first value has been processed
                if firstSend:
                    firstSend = False
                    Settings.num_of_send_counter = 0
            # if the code is not ready to send check if keyboard keys are pressed and any buttons on tag are pressed
        if not Settings.startSend:
            # ask to tag if a menubutton has been pressed yet
            WiFiString = request.getRequest(IPadress + ':83/sendMenu')

            # if awnser is '0' send button has been pressed
            if WiFiString == '0':
                Settings.startSend = True
                firstSend = True
                request.getRequest(IPadress + ':80/anchors')
                t1.start()
            if keyboard.is_pressed('s'):
                request.getRequest(IPadress + ':84/start')
                request.getRequest(IPadress + ':80/anchors')
                Settings.startSend = True
                t1.start()
            # if answer of tag is '1' back button has been pressed

            if WiFiString == '1' or keyboard.is_pressed('b'):
                Settings.wks_count -= 1
                wks = wbk.get_worksheet_by_name(Coordinates[Settings.wks_count])
                Settings.position_y = 1
                Settings.num_of_send_counter = 0
                xls.first_rows_excel(wks, distance_array, tagInfo, Settings)

            if WiFiString == '2' or keyboard.is_pressed('d'):
                if not Settings.draw:
                    fig, ax, plt = draw.start_figure()
                    Settings.draw = True

            # if awnser of tag is '3' end button has been pressed
            if WiFiString == '3' or keyboard.is_pressed('e'):
                print('end', request.getRequest(IPadress + ':86/restart'))
                endCode(wbk)

def endCode(wbk):
    wbk.close()
    sys.exit(0)

#WifiReadLoop()# start eventLoop
setDelays()
setAccuracy()
setLowPower()