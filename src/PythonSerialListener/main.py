import numpy as np
import serial
import trilateration
import sys
import time
import request
import xls

test_xyz = True

counter_bool = True

last_measured = 0

start_bool = False

start_pos_avg_counter = 1  # the first average counter posistion in if statement of the VSC

interval_time_bool = True

print_check = True

# choose which way to end this program(so the excel file will be saved)
end_with_button = True
end_with_count = False
backspace_active = False

# define comport to which the tag is connected
comPort = "COM12"

def WifiReadLoop(onIncoming):
    #make xls settings before entering while loop
    wks_count = 0
    wks_bool = 0
    testName = '/x-y_test'  # name of the kind of test(choose between '/rangetest' or '/x_y-test')
    wbk = xls.make_new_wbk(testName)
    float_distance = 7
    back_bool = False
    cal_distances = request.getRequest(':81/caldis')
    end_count = 0
    max_anchors, Coordinates, AD_start, AD_end, AD_interval, ID_array, x_array, y_array, z_array, distance_array, nos, dcm, reset_dcm, dci = request.getValues(cal_distances)
    print(Coordinates)
    # print(AD_start)
    # print(AD_end)
    # print(AD_interval)
    # print(ID_array)
    # print(x_array)
    # print(y_array)
    # print(distance_array)
    print(dci)
    print(max_anchors)
    wks = xls.make_new_wks(Coordinates[end_count], wbk)
    position_x = 0
    if test_xyz == False:
        for i in range (6):
            r1 = distance_array[0][i]
            r2 = distance_array[1][i]
            r3 = distance_array[2][i]
            print(trilateration.triliterationnew3D(x_array, y_array, z_array, r1, r2, r3))
    position_y = 1
    aanstaan = False
    if max_anchors == '1':
        xls.first_rows_excel(position_x, wks, AD_interval, AD_start, AD_end, end_count, nos, dcm, reset_dcm, dci, max_anchors)
    else:
        xls.first_rows_excel_multiple(position_x, wks, AD_interval, AD_start, AD_end, end_count, nos, dcm, reset_dcm, dci, distance_array, max_anchors)
    position_x = 0
    position_y = 1
    WiFiString1 = 'a'
    while True:
        time.sleep(0.050)
        WiFiString = request.getRequest(':80/anchors')
        #print(WiFiString)
        if 'stop' in WiFiString:
            wbk.close()
            sys.exit(0)
        if(WiFiString != 'not'):
            if 'ID' in WiFiString and 'd' in WiFiString:
                if end_with_button:
                    if 'end' in WiFiString:  # if string contains 'end', stop program when stop with button is used
                        end_count += 1
                        if end_count >= len(Coordinates):
                            wbk.close()
                            sys.exit(0)
                        #float_distance -= 0.5
                        if back_bool == False:
                            print(Coordinates[end_count])
                            wks = xls.make_new_wks(Coordinates[end_count], wbk)
                        else:
                            print(Coordinates[end_count])
                            wks = wbk.get_worksheet_by_name(Coordinates[end_count])
                            back_bool = False
                        position_y = 1
                        position_x = 0
                        if max_anchors == '1':
                            xls.first_rows_excel(position_x, wks, AD_interval, AD_start, AD_end, end_count, nos, dcm, reset_dcm, dci, max_anchors)
                        else:
                            xls.first_rows_excel_multiple(position_x, wks, AD_interval, AD_start, AD_end, end_count, nos, dcm, reset_dcm, dci, distance_array, max_anchors)
                        # write AD and time in ms for the anchors
                    elif 'back' in WiFiString and end_count > 0:
                        end_count -= 1
                        wks = wbk.get_worksheet_by_name(Coordinates[end_count])
                        back_bool = True
                        position_y = 1
                        position_x = 1
                        xls.first_rows_excel(position_x, wks, AD_interval, AD_start, AD_end, distance_array, end_count, nos, dcm, reset_dcm, dci, max_anchors)
                    else:
                        if backspace_active and end_count > 1 and 'back' in WiFiString:
                            position_y -= 1  # go colom back in excel after a wrong measurent
                            print(position_y)
                        try:
                            print('y')
                            print(position_y)
                            print('x')
                            print(position_x)
                            print(WiFiString)
                            if 'start' in WiFiString and aanstaan == False:
                                position_x = 1
                                position_y = 0
                                aanstaan = True
                            elif 'a' in WiFiString:
                                WiFiString = WiFiString.split('\n')
                                WiFiString = WiFiString.pop(0)
                                WiFiString = WiFiString.split('a\t')
                                print(WiFiString)
                                onIncoming(WiFiString, position_y, position_x, wks, x_array, y_array, z_array, nos, max_anchors)
                                position_y += 3
                                position_x = 1

                            elif 'e' in WiFiString:
                                WiFiString = WiFiString.split('\n')
                                WiFiString = WiFiString.pop(0)
                                WiFiString = WiFiString.split('e\t')
                                print(WiFiString)
                                onIncoming(WiFiString, position_y, position_x, wks, x_array, y_array, z_array, nos, max_anchors)
                                position_y += 2
                                position_x = 1
                            else:
                                WiFiString = WiFiString.split('\n')
                                WiFiString = WiFiString.pop(0)
                                WiFiString = WiFiString.split('\t')
                                print(WiFiString)
                                onIncoming(WiFiString, position_y, position_x, wks, x_array, y_array, z_array,  nos, max_anchors)
                                position_x += 1
                        except:
                            pass
                #print("not")
            else:
                if(WiFiString1 == WiFiString):
                    print(WiFiString)
                WiFiString1 = WiFiString

                #print(WiFiString)


# z = 80.5

def parseJson(x, y, d, ID):
    if len(ID) > 3:  # if there are more then 3 anchors the three shortest distances will be used for the calculation
        low_arr_num = min(d)
        min_index = d.index(low_arr_num)
        x = np.delete(x, min_index)
        y = np.delete(y, min_index)
        d = np.delete(d, min_index)
        ID = np.delete(ID, min_index)
        #print(ID)
        #print(x)
        #print(y)
        #print(d)
    if (len(ID == 3)):  # when there are 3 they will be used for the calculation
        valueX, valueY = trilateration.getCoordinates1(x, y, d)
    return valueX, valueY

def xyzTest():
    P1 = np.array([2,0,1])
    P2 = np.array([0,2,1])
    P3 = np.array([2,2,1])
    r1 = 1.92
    r2 = 1.92
    r3 = 2.59
    print(trilateration.trilateration3D(P1,P2,P3,r1,r2,r3))

# start eventLoop
WifiReadLoop(xls.put_in_xls)