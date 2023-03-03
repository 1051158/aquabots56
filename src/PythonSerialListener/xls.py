import xlsxwriter
from xlsxwriter.utility import xl_rowcol_to_cell
import trilateration
import array
import draw
import numpy as np
import request
distances_per_interval = 2

give_name = True

Bool_anchorFilter = False

def make_new_wbk(testName):
    if give_name == True:
        fileName = '/l.xlsx' # name of the testfile
    else:
        fileName = '/integration.xlsx'
    directory = '/home/donny/Documents/UWB-Location_total_code/localization-uwb-esp32/src/PythonSerialListener'  # name of this directory so the xsl lib works
    modeName = '/LOWPOWER_MODE'
    anchorName = '/anchor1'  # name of the anchor to test the range with choose 1,2 or 3
    if '/x-y_test' in testName:
        directory = directory + testName + modeName + fileName  # string combined
    else:
        directory = directory + testName + modeName + anchorName + fileName  # string combined
    wbk = xlsxwriter.Workbook(r'' + directory)
    return wbk



def put_in_xls(jsonString, y_row, x_row, wks, x_array, y_array, z_array, nos, max_anchors):
    print(z_array)
    nos = int(nos)
    anchor = jsonString
    #print(anchor)
    max_anchors = int(max_anchors)
    Anchorfilter = 0
    python_test_bool = False
    for i in range(max_anchors):
        #print(anchor[i])
        ID = anchor[i].split('ID')  # split ID from the string
        #print(ID)
        D = ID[1].split('d')  # split distance from the string
        #print(D)
        #dc = D[1].split('Dc')
        ms = D[1].split('ms')
        #print(ms)
        # counter[i] = y[i][1].split('dc')
        ID = ID.pop(0)  # remove the rest of the array except the split
        #print(ID)
        D = D.pop(0)

        ms = ms.pop(0)
        # if (print_check == True and ID[i] != 0):
        #print(ms)

        if ID == '0':
            wks.write(xl_rowcol_to_cell(y_row, x_row + 2), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(y_row + 1, x_row + 2), ms)
            if Bool_anchorFilter == True:
                Anchorfilter -=1
        if ID == '1':
            wks.write(xl_rowcol_to_cell(y_row, x_row + nos + 6), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(y_row + 1, x_row + nos + 6), ms)
            if Bool_anchorFilter == True:
                Anchorfilter -= 2
        if ID == '2':
            wks.write(xl_rowcol_to_cell(y_row, x_row + nos + 12), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(y_row + 1, x_row + nos + 12), ms)
            if Bool_anchorFilter == True:
                Anchorfilter -= 3
        if ID == '3':
            wks.write(xl_rowcol_to_cell(y_row, x_row + nos + 18), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(y_row + 1, x_row + nos + 18), ms)
            if Bool_anchorFilter == True:
                Anchorfilter -= 4
        if i == 0:
            D_1 = float(D)
        if i == 1:
            D_2 = float(D_1)
            D_1 = float(D)
        if i == 2:
            print(D)
            print(D_1)
            print(D_2)
            if Bool_anchorFilter == True:
                Anchorfilter += 9
                del x_array[Anchorfilter]
                del y_array[Anchorfilter]
            print(trilateration.triliterationnew3D(x_array, y_array, z_array, D_2, D_1, D))
            #wks.write(xl_rowcol_to_cell(y_row, x_row + len(anchor)*(nos + 5)), Xvalue)
            #wks.write(xl_rowcol_to_cell(y_row, x_row + len(anchor)*(nos + 5)+nos), Yvalue)
            # write the calculated values with in the right row and colum of the AD_value
        #print(D_array)






def first_rows_excel_multiple(row, wks, AD_interval, AD_start, AD_end, end_count, nos, dcm, reset_dcm, dci, Distance_array, max_anchors):
    j = 0
    print(dci)
    number_of_intervals = (float(AD_end) - float(AD_start)) / float(AD_interval)
    # i = last_measured
    max_anchors = int(max_anchors)
    nos = int(nos)
    number_of_dcm = (int(reset_dcm) - int(dcm))/int(dci)
    distances_per_interval = int(reset_dcm)/int(dcm)
    wks.write(xl_rowcol_to_cell(1, max_anchors*(nos+3)), 'x')
    wks.write(xl_rowcol_to_cell(1, (max_anchors*(nos+3)+5)), 'y')
    for k in range(max_anchors):
        wks.write(xl_rowcol_to_cell(1 + j + 1, row), Distance_array[k][end_count])
        for i in range(int(number_of_intervals) + 1):  # make sure the max anchors is correct with reality
            wks.write(xl_rowcol_to_cell(i + j, row), float(AD_start) + (float(AD_interval) * i))  # write on the cells defined by y row and the i of the for loop
            for l in range(int(number_of_dcm) + 1):
                wks.write(xl_rowcol_to_cell(i + j + 1, row + 1), 'avg_' + str(l + int(dcm)))
                j += 1
                wks.write(xl_rowcol_to_cell(i + j + 1, row + 1), 'time(ms)')
                j += 1
        row += nos + 3
        j = 0

        # last_measured = j
        # print(last_measured)
        # wbk.close()

def first_rows_excel(row, wks, AD_interval, AD_start, AD_end, end_count, nos, dcm, reset_dcm, dci, max_anchors):
    j = 0
    print(dci)
    number_of_intervals = (float(AD_end) - float(AD_start)) / float(AD_interval)
    # i = last_measured
    number_of_dcm = (int(reset_dcm) - int(dcm)) / int(dci)
    distances_per_interval = int(reset_dcm) / int(dcm)
    for i in range(int(number_of_intervals) + 1):  # make sure the max anchors is correct with reality
        wks.write(xl_rowcol_to_cell(i + j, row), float(AD_start) + (float(AD_interval) * i))  # write on the cells defined by y row and the i of the for loop
        for l in range(int(number_of_dcm) + 1):
            wks.write(xl_rowcol_to_cell(i + j, row + 1), 'avg_' + str(l + int(dcm)))
            j += 1
            wks.write(xl_rowcol_to_cell(i + j, row + 1), 'time(ms)')
            j += 1
    j = 0

def make_new_wks(coordinates, wbk):
    wks = wbk.add_worksheet(coordinates)
    return wks