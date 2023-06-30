import xlsxwriter
from xlsxwriter.utility import xl_rowcol_to_cell
import trilateration
import draw
import array
import numpy as np

def put_in_xls(WiFiString, wks, x_array, y_array, z_array, tagInfo, fig, Settings, timer, ax, plt):
    #split the string into array's
    WiFiString = WiFiString.split('\n')
    WiFiString = WiFiString.pop(0)
    WiFiString = WiFiString.split('\t')
    anchor = WiFiString

    ID_array = array.array('i', [])
    D_array = array.array('f', [])

    #print for debugging
    if Settings.dbgXls:
        print(anchor)

    for i in range((tagInfo.max_anchors)-1):
        #split the array into subarray's.

        # split ID from the string
        ID = anchor[i].split('ID')  

        # split distance from the string
        D = ID[1].split('d')  

        #split measureTime from string(if activated) #todo
        #ms = D[1].split('ms')

        #take the first value of every subarray.
        ID = ID.pop(0)
        D = D.pop(0)
        #ms = ms.pop(0)

        ID_array.append(int(ID))
        D_array.append(float(D))

        #print only when debugging.
        if Settings.dbgXls:
            print(ID)
            #print(ms)

            if i == 2:
                print(ID_array)
                print(D_array)

        #check the ID number to make sure the values come in the right spot in excel...

        if tagInfo.AD_start != tagInfo.AD_end:
            print('ID: ', ID)
            xpos = Settings.num_of_send_counter + 2 + 2 * int(ID) + (Settings.nos + 1) * int(ID)
            print('xpos: ', xpos)
            wks.write(xl_rowcol_to_cell(Settings.position_y, xpos), D)
        else:
            wks.write(xl_rowcol_to_cell(Settings.position_y + 2*int(ID), Settings.num_of_send_counter), D)

        if i == 2:
            if Settings.xyzTest:
                Coordinate1, Coordinate2 = trilateration.triliterationnew3D(x_array, y_array, z_array, D_array[0],
                                                                            D_array[1], D_array[2], ID_array, Settings)
            else:
                x, y = trilateration.getCoordinates1(x_array, y_array, D_array[0],
                                                                        D_array[1], D_array[2])

            if Settings.dbgXls:
                if Settings.xyzTest:
                    print('Coordinate1: ', Coordinate1)
                    print('Coordinate2: ', Coordinate2)
                else:
                    print('x:', x)
                    print('y:', y)

            # put the x_y_z values in excel
            if tagInfo.AD_start == tagInfo.AD_end:
                if Settings.xyzTest:
                    for j in range(tagInfo.max_anchors - 1):
                        wks.write(xl_rowcol_to_cell((tagInfo.max_anchors + 1) * 2, Settings.num_of_send_counter + 2),
                                  Coordinate1[j])
                        wks.write(xl_rowcol_to_cell((tagInfo.max_anchors + 1) * 2, + Settings.num_of_send_counter + 2),
                                  Coordinate2[j])
                else:
                    wks.write(xl_rowcol_to_cell(10, Settings.num_of_send_counter + 2), x)
                    wks.write(xl_rowcol_to_cell(11, Settings.num_of_send_counter + 2), y)

            #put the x_y_z values in excel
            if tagInfo.AD_start != tagInfo.AD_end:
                if Settings.xyzTest:
                    for j in range(tagInfo.max_anchors - 1):
                        xpos_xyz1 = (Settings.nos + 3) * (tagInfo.max_anchors) + Settings.num_of_send_counter + (
                                    Settings.nos + 1) * j
                        xpos_xyz2 = (Settings.nos + 3) * (tagInfo.max_anchors) + Settings.num_of_send_counter + (
                                    Settings.nos + 1) * j + (Settings.nos + 2) * 3
                        print('xyz1: ', xpos_xyz1)
                        print('xyz2: ', xpos_xyz2)
                        wks.write(xl_rowcol_to_cell(Settings.position_y,
                                                    (Settings.nos + 3) * (
                                                        tagInfo.max_anchors) + Settings.num_of_send_counter + (
                                                                Settings.nos + 1) * j + j), Coordinate1[j])
                        wks.write(xl_rowcol_to_cell(Settings.position_y, (Settings.nos + 2) * (
                            tagInfo.max_anchors) + Settings.num_of_send_counter + Settings.nos * j + (
                                                                Settings.nos + 1) * 3 + 3 + j), Coordinate2[j])
                    if Coordinate1[2] > 0:
                        fig, ax, plt = draw.animate_func(Coordinate1, fig, ax, plt, Settings.xyzTest)
                else:
                    wks.write(xl_rowcol_to_cell(Settings.position_y,(Settings.nos + 3) * (
                                                    tagInfo.max_anchors) + Settings.num_of_send_counter + (
                                                Settings.nos + 1)), x)
                    wks.write(xl_rowcol_to_cell(Settings.position_y, (Settings.nos + 3) * (
                        tagInfo.max_anchors) + Settings.num_of_send_counter + (
                                                        Settings.nos + 1) * 2 + 2), y)
                    Coordinate2 = np.array([x, y])
                    fig, ax, plt = draw.animate_func(Coordinate2, fig, ax, plt, Settings.xyzTest)
            # draw the measured point with a positive Z-coordinate

    timer.stop()
    timer.start()
    return timer
def first_rows_excel_multiple_AD(wks, tagInfo, Distance_array, Settings):
    j = 0
    row = Settings.num_of_send_counter

    # calculate the number of AD intervals
    number_of_intervals = (tagInfo.AD_end - tagInfo.AD_start) / tagInfo.AD_interval

    number_of_dcm = (Settings.reset_dcm - Settings.dcm) / int(Settings.dci)
    wks.write(xl_rowcol_to_cell(0, (Settings.nos + 2) * tagInfo.max_anchors + 1), 'x1')
    wks.write(xl_rowcol_to_cell(0, (Settings.nos + 2) * (tagInfo.max_anchors + 1) + 1), 'y1')
    wks.write(xl_rowcol_to_cell(0, (Settings.nos + 2) * (tagInfo.max_anchors + 2) + 2), 'z1')
    wks.write(xl_rowcol_to_cell(0, (Settings.nos + 2) * (tagInfo.max_anchors + 3) + 3), 'x2')
    wks.write(xl_rowcol_to_cell(0, (Settings.nos + 2) * (tagInfo.max_anchors + 4) + 4), 'y2')
    wks.write(xl_rowcol_to_cell(0, (Settings.nos + 2) * (tagInfo.max_anchors + 5) + 5), 'z2')

    for k in range((tagInfo.max_anchors)):
        print('k', k)
        wks.write(xl_rowcol_to_cell(3 + j, row), Distance_array[k][Settings.wks_count])
        for i in range(int(number_of_intervals) + 1):  # make sure the max anchors is correct with reality
            wks.write(xl_rowcol_to_cell(i + j, row), float(tagInfo.AD_start) + (float(
                tagInfo.AD_interval) * i))  # write on the cells defined by y row and the i of the for loop
            for l in range(int(number_of_dcm) + 1):
                wks.write(xl_rowcol_to_cell(i + j + 1, row + 1), 'avg_' + str(l + int(Settings.dcm)))
                j += 1
        j = 0
        row += Settings.nos + 3

def first_rows_excel_multiple_anchor(wks, tagInfo, Distance_array, Settings):
    j = 0
    row = 0
    for i in range(Settings.nos):
        wks.write(xl_rowcol_to_cell(0, i + 2), i)

    number_of_dcm = (Settings.reset_dcm - Settings.dcm) / int(Settings.dci)
    if Settings.xyzTest:
        wks.write(xl_rowcol_to_cell(10, 0), 'x1')
        wks.write(xl_rowcol_to_cell(11, 0), 'y1')
        wks.write(xl_rowcol_to_cell(12, 0), 'z1')
        wks.write(xl_rowcol_to_cell(14, 0), 'x2')
        wks.write(xl_rowcol_to_cell(15, 0), 'y2')
        wks.write(xl_rowcol_to_cell(16, 0), 'z2')

    else:
        wks.write(xl_rowcol_to_cell(10, 0), 'x')
        wks.write(xl_rowcol_to_cell(11, 0), 'y')

    for k in range((tagInfo.max_anchors)):
        print('k', k)
        wks.write(xl_rowcol_to_cell(2 * k , row), Distance_array[k][Settings.wks_count])
        for l in range(int(number_of_dcm) + 1):
            wks.write(xl_rowcol_to_cell(2*k + l + 1, row + 1), 'avg_' + str(l + int(Settings.dcm)))

def first_rows_excel(wks, tagInfo, distance_array, Settings):
    j = 0
    print(Settings.dci)
    row = Settings.num_of_send_counter
    number_of_intervals = (float(tagInfo.AD_end) - float(tagInfo.AD_start)) / float(tagInfo.AD_interval)
    number_of_dcm = (int(Settings.reset_dcm) - int(Settings.dcm)) / int(Settings.dci)

    for i in range(int(number_of_intervals) + 1):  # make sure the max anchors is correct with reality
        Settings.wks.write(xl_rowcol_to_cell(i + j, row), float(tagInfo.AD_start) + (float(tagInfo.AD_interval) * i))
        # write on the cells defined by y row and the i of the for loop
        for l in range(int(number_of_dcm) + 1):
            wks.write(xl_rowcol_to_cell(i + j, row + 1), 'avg_' + str(l + int(Settings.dcm)))
            j += 1
            wks.write(xl_rowcol_to_cell(i + j, row + 1), 'time(ms)')
            j += 1
    j = 0
    row += Settings.nos + 2

def make_new_wbk(give_name):
    if give_name == True:
        print('give filename: ')
        fileName = '/' + input() + '.xlsx' # name of the testfile
    else:
        fileName = '/integration.xlsx'
    directory = '/home/donny/Aqualab/Acces_tag_python/xlsxFiles'  # name of this directory so the xsl lib works
    modeName = '/LOWPOWER_MODE'
    anchorName = ''  # name of the anchor to test the range with choose 1,2 or 3
    testName = '/x-y_test'  # name of the kind of test(choose between '/rangetest' or '/x_y-test')

    if '/x-y_test' or '/x-y-z_test' in testName:
        directory = directory + testName + modeName + fileName  # string combined
    else:
        directory = directory + testName + modeName + anchorName + fileName  # string combined
    wbk = xlsxwriter.Workbook(r'' + directory)
    return wbk, testName

def make_new_wks(coordinates, wbk):
    wks = wbk.add_worksheet(coordinates)
    return wks