import requests
import array
import sys
import XLS as xls
import Settings as settings
import time
from dataclasses import dataclass

# write the right IP in Visual Studio for the http link
# make sure the laptop has same WiFi-connection as tag-code in VSC


@dataclass
class receivedData:
    max_anchors: int
    AD_start: int
    AD_end: int
    AD_interval: int
    antenna_delay: int


def setADelays(Settings, anchornumber):
    antenna_delay = 16400
    whileBool = False
    if anchornumber == 1:
        while antenna_delay != Settings.ADelay_1 and not whileBool:
            if antenna_delay < Settings.ADelay_1:
                antenna_delay = int(getRequest(Settings.IP_A1 + ":81/addAD"))

            if antenna_delay > Settings.ADelay_1:
                antenna_delay = int(getRequest(Settings.IP_A1 + ":81/subAD"))

            if antenna_delay == Settings.ADelay_1:
                whileBool == True

    print(antenna_delay)
    antenna_delay = 16400
    whileBool = False

    if anchornumber == 3:
        while antenna_delay != Settings.ADelay_2 and not whileBool:
            if antenna_delay < Settings.ADelay_2:
                antenna_delay = int(getRequest(Settings.IP_A2 + ":81/addAD"))

            if antenna_delay > Settings.ADelay_2:
                antenna_delay = int(getRequest(Settings.IP_A2 + ":81/subAD"))

            if antenna_delay == Settings.ADelay_2:
                whileBool == True
    print(antenna_delay)
    antenna_delay = 16400
    whileBool = False

    if anchornumber == 3:
        while antenna_delay != Settings.ADelay_3 and not whileBool:
            if antenna_delay < Settings.ADelay_3:
                antenna_delay = int(getRequest(Settings.IP_A3 + ":81/addAD"))

            if antenna_delay > Settings.ADelay_3:
                antenna_delay = int(getRequest(Settings.IP_A3 + ":81/subAD"))

            if antenna_delay == Settings.ADelay_3:
                whileBool == True
    print(antenna_delay)
    antenna_delay = 16400
    whileBool = False

    if anchornumber == 4:
        while antenna_delay != Settings.ADelay_4 and not whileBool:
            if antenna_delay < Settings.ADelay_4:
                antenna_delay = int(getRequest(Settings.IP_A4 + ":81/addAD"))

            if antenna_delay > Settings.ADelay_4:
                antenna_delay = int(getRequest(Settings.IP_A4 + ":81/subAD"))

            if antenna_delay == Settings.ADelay_4:
                whileBool == True
    print(antenna_delay)
    antenna_delay = 16400
    whileBool = False


# put the link of the Server to get the info from right server
def getRequest(server_address):
    # combine the IPadress and Server address
    while True:
        try:
            response = requests.get(server_address, timeout=10)
            break
        except:
            print("connection error of ip:", server_address)
            time.sleep(0.1)
    time.sleep(0.01)
    return response.text


def getValues(WiFistring):
    tagString = WiFistring.split("\t\n")
    print(tagString)

    # sort the big string out in the variables(underneath this comment) and in the arrays
    hulp_coordinates = tagString.pop(0)
    hulp_coordinates = hulp_coordinates.split(";")
    Coordinates = hulp_coordinates

    # delete the last value(contains nothing)
    del Coordinates[len(hulp_coordinates) - 1]

    # get the Antenna delay settings info from and the send info string
    String_help = tagString[0]
    max_anchors = String_help.split("max")
    AD_start = max_anchors[1].split("S")
    AD_end = AD_start[1].split("E")
    AD_interval = AD_end[1].split("I")
    max_anchors = max_anchors.pop(0)
    AD_start = AD_start.pop(0)
    AD_end = AD_end.pop(0)
    AD_interval = AD_interval.pop(0)

    # when only 1 anchor is used the rest of info not necessary
    if max_anchors == 1:
        return max_anchors, Coordinates, AD_start, AD_end, AD_interval

    # start by making array variables to store the string in
    ID_array = array.array("i", [])
    x_array = array.array("f", [])
    y_array = array.array("f", [])
    z_array = array.array("f", [])

    # delete the first part of the tagString, because it has already been set into variables(see code above)

    # make a array which size depends on the
    del tagString[0]
    Distance_array = [[0] * len(Coordinates)] * (len(tagString) - 1)
    for i in range(len(tagString) - 1):
        help = tagString[i].split("i")
        print(help)
        help1 = help[1].split("x")
        help2 = help1[1].split("y")
        help3 = help2[1].split("z")
        help = help.pop(0)
        help1 = help1.pop(0)
        help2 = help2.pop(0)
        help3 = help3.pop(0)
        Distances = tagString[i].split("\t")
        del Distances[0]
        ID_array.append(int(help))
        x_array.append(float(help1))
        y_array.append(float(help2))
        z_array.append(float(help3))
        Distance_array[i] = Distances

    antenna_delay = int(AD_start)
    tagInfo = receivedData(
        int(max_anchors),
        int(AD_start),
        int(AD_end),
        int(AD_interval),
        int(antenna_delay),
    )
    print(tagInfo)
    print(Coordinates)
    print(x_array)
    print(y_array)
    print(z_array)
    return tagInfo, Coordinates, x_array, y_array, z_array, Distance_array


def callRightServer(
    Coordinates, wbk, wks, distance_array, tagInfo, Settings, t, t1, IPadress
):
    print("maxNOSreached")
    if Settings.dcm >= int(Settings.reset_dcm):
        Settings.dcm = 1
        print("resetDCM", getRequest(IPadress + ":82/resetDCM"))
        if tagInfo.AD_start == tagInfo.AD_end:
            Settings.wks_count += 1
            Settings.startSend = False
            getRequest(IPadress + ":81/resetAD")
            if Settings.wks_count >= len(Coordinates):
                wbk.close()
                sys.exit(0)
            # float_distance -= 0.5

            if Settings.back == False:
                print(Coordinates[Settings.wks_count])
                wks = xls.make_new_wks(Coordinates[Settings.wks_count], wbk)
                print(Coordinates[Settings.wks_count])
                wks = wbk.get_worksheet_by_name(Coordinates[Settings.wks_count])
                Settings.back = False

            if tagInfo.max_anchors == "1":
                xls.first_rows_excel(wks, tagInfo, distance_array, Settings)

            else:
                xls.first_rows_excel_multiple_anchor(
                    wks, tagInfo, distance_array, Settings
                )
            t1.stop()
        else:
            if int(tagInfo.antenna_delay) >= tagInfo.AD_end:
                Settings.wks_count += 1
                print("went in")
                if Settings.wks_count >= len(Coordinates):
                    wbk.close()
                    sys.exit(0)
                # float_distance -= 0.5

                if Settings.back == False:
                    print(Coordinates[Settings.wks_count])
                    wks = xls.make_new_wks(Coordinates[Settings.wks_count], wbk)

                else:
                    print(Coordinates[Settings.wks_count])
                    wks = wbk.get_worksheet_by_name(Coordinates[Settings.wks_count])
                    Settings.back = False

                if tagInfo.max_anchors == "1":
                    xls.first_rows_excel(wks, tagInfo, distance_array, Settings)

                else:
                    xls.first_rows_excel_multiple_AD(
                        wks, tagInfo, distance_array, Settings
                    )

                # reset the tag with the right link in the getRequest
                print("resetAD", getRequest(IPadress + ":81/resetAD"))
                Settings.startSend = False
                tagInfo.antenna_delay = tagInfo.AD_start
                Settings.position_y = 1
                print("time 1 coordinate:")
                t1.stop()
                sleepTime = 0.3
            else:
                t.start()
                print("addAD", getRequest(IPadress + ":81/addAD"))
                t.stop()
                tagInfo.antenna_delay += tagInfo.AD_interval
                Settings.position_y += 2
                sleepTime = 0.3

    else:
        Settings.dcm += Settings.dci

        if Settings.dbgReq:
            print("DCM= ")
            print(Settings.dcm)

        getRequest(IPadress + ":82/addDCM")
        Settings.position_y += 1
        sleepTime = 0.2
    return tagInfo, wks, Settings, t, t1
