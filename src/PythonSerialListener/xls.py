import xlsxwriter
from xlsxwriter.utility import xl_rowcol_to_cell
import draw
import trilateration
import array
import draw
from dataclasses import dataclass
import numpy as np
import request
distances_per_interval = 2

give_name = True

Bool_anchorFilter = False

bool_check_xls = True

@dataclass
class strctSet:
    dbgXls: bool
    dbgFirstRow: bool
    dbgReq: bool
    dbgInteg: bool
    xyz_test: bool
    nos: int
    dcm: int
    max_dcm: int
    reset_dcm: int
    dci: int
    startSend: bool = False
    back: bool = False
    wks_count: int = 0
    num_of_send_counter: int = 0
    position_y: int = 1

#enter all the lay out settings of excel here.
def excelSettings(Coordinates):

    #fill in the variables underneath the code will fill them in the struct some variables are private(can't be changed)
    dbgXls = True
    dbgFirstRow = False
    dbgReq = True
    dbgInteg = True
    nos = 5
    dcm = 1
    max_dcm = 1
    reset_dcm = 1
    dci = 1

    #set this false if you don't want to give a name each time the program starts
    give_name = True
    wbk, mapName = make_new_wbk(give_name)
    #xyz_test depends on the map the wbk comes in:
    if '/x-y-z_test' in mapName:
        xyz_test = True
    else:
        xyz_test = False
    Settings = strctSet(dbgXls, dbgFirstRow, dbgReq, dbgInteg, xyz_test, nos, dcm, max_dcm, reset_dcm, dci)
    print(Settings)
    # make a worksheet for excel
    wks = make_new_wks(Coordinates[Settings.wks_count], wbk)
    
    #open the figure that will show the coordinate
    fig = draw.start_figure()
    
    #return the workbook , worksheet, struct and figure
    return wbk, wks, Settings, fig

def put_in_xls(WiFiString, wks, x_array, y_array, z_array, tagInfo, fig, Settings):
    #split the string into array's
    WiFiString = WiFiString.split('\n')
    WiFiString = WiFiString.pop(0)
    WiFiString = WiFiString.split('\t')
    anchor = WiFiString

    #an anchorcounter to make sure three anchors with the right ID's are measured
    anchorCounter = 0
    anchorbool = np.array([0, 0, 0, 0])
    D_1 = 0
    D_2 = 0
    #print for debugging
    if Settings.dbgXls:
        print(anchor)

    Anchorfilter = 0

    for i in range((tagInfo.max_anchors)-1):
        #split the array into subarray's.
        ID = anchor[i].split('ID')  # split ID from the string
        D = ID[1].split('d')  # split distance from the string
        ms = D[1].split('ms')

        #take the first value of every subarray.
        ID = ID.pop(0)
        D = D.pop(0)
        ms = ms.pop(0)

        #print only when debugging.
        if Settings.dbgXls:
            print(ID)
            print(ms)

        #check the ID number to make sure the values come in the right spot in excel...

        #anchor 1
        if ID == '0':
            wks.write(xl_rowcol_to_cell(Settings.position_y, Settings.num_of_send_counter + 2), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(Settings.position_y + 1, Settings.num_of_send_counter + 2), ms)
            anchorCounter += 1

        #anchor 2
        if ID == '1':
            wks.write(xl_rowcol_to_cell(Settings.position_y, Settings.num_of_send_counter + Settings.nos + 6), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(Settings.position_y + 1, Settings.num_of_send_counter + Settings.nos + 6), ms)
            anchorCounter += 1

        #anchor 3
        if ID == '2':
            wks.write(xl_rowcol_to_cell(Settings.position_y, Settings.num_of_send_counter + Settings.nos + 12), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(Settings.position_y + 1, Settings.num_of_send_counter + Settings.nos + 12), ms)
            anchorCounter += 1

        #anchor 4
        if ID == '3':
            wks.write(xl_rowcol_to_cell(Settings.position_y, Settings.num_of_send_counter + Settings.nos + 18), D)  # write distances on the cells
            wks.write(xl_rowcol_to_cell(Settings.position_y + 1, Settings.num_of_send_counter + Settings.nos + 18), ms)
            anchorCounter += 1

        #check how far the anchorcounter is and put the distances in variables
        if i == 0:
            D_1 = float(D)

        if i == 1:
            D_2 = float(D_1)
            D_1 = float(D)

        if i == 2:
            if Settings.dbgXls:
                print(D)
                print(D_1)
                print(D_2)

            #calculate the coordinates
            Coordinate1, Coordinate2 = trilateration.triliterationnew3D(x_array, y_array, z_array, D_2, D_1, D)

            #draw the measured point with a positive Z-coordinate
            if Coordinate1[2] > 0:
                draw.animate_func(Coordinate1, fig)
            else:
                draw.animate_func(Coordinate2, fig)

            # write the calculated values with in the right row and colum of the AD_value
            wks.write(xl_rowcol_to_cell(Settings.position_y, Settings.num_of_send_counter + len(anchor)*(Settings.nos + 5)), Coordinate1[0])
            wks.write(xl_rowcol_to_cell(Settings.position_y, Settings.num_of_send_counter + len(anchor)*(Settings.nos + 5) + Settings.nos), Coordinate1[1])
            print('works')
            return

def first_rows_excel_multiple_AD(wks, tagInfo, Distance_array, Settings):
    j = 0
    row = Settings.num_of_send_counter

    #calculate the number of AD intervals
    number_of_intervals = (tagInfo.AD_end - tagInfo.AD_start) / tagInfo.AD_interval

    nos = Settings.nos
    number_of_dcm = (Settings.reset_dcm - Settings.dcm)/int(Settings.dci)
    wks.write(xl_rowcol_to_cell(1, tagInfo.max_anchors*(nos+3)), 'x')
    wks.write(xl_rowcol_to_cell(1, (tagInfo.max_anchors*(nos+3)+5)), 'y')
    for k in range((tagInfo.max_anchors)):
        print('k', k)
        wks.write(xl_rowcol_to_cell(1 + j + 1, row), Distance_array[k][Settings.wks_count])
        for i in range(int(number_of_intervals) + 1):  # make sure the max anchors is correct with reality
            wks.write(xl_rowcol_to_cell(i + j, row), float(tagInfo.AD_start) + (float(tagInfo.AD_interval) * i))  # write on the cells defined by y row and the i of the for loop
            for l in range(int(number_of_dcm) + 1):
                wks.write(xl_rowcol_to_cell(i + j + 1, row + 1), 'avg_' + str(l + int(Settings.dcm)))
                j += 1
                wks.write(xl_rowcol_to_cell(i + j + 1, row + 1), 'time(ms)')
                j += 1
        j = 0
        row += Settings.nos + 3

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
    row += Settings.nos + 3

def make_new_wbk(give_name):
    if give_name == True:
        print('give filename: ')
        fileName = '/' + input() + '.xlsx' # name of the testfile
    else:
        fileName = '/integration.xlsx'
    directory = '/home/donny/Documents/Aqualab/pythonProject/xlsxFiles'  # name of this directory so the xsl lib works
    modeName = '/LOWPOWER_MODE'
    anchorName = ''  # name of the anchor to test the range with choose 1,2 or 3
    testName = '/x-y-z_test'  # name of the kind of test(choose between '/rangetest' or '/x_y-test')

    if '/x-y_test' or '/x-y-z_test' in testName:
        directory = directory + testName + modeName + fileName  # string combined
    else:
        directory = directory + testName + modeName + anchorName + fileName  # string combined
    wbk = xlsxwriter.Workbook(r'' + directory)
    return wbk, testName

def make_new_wks(coordinates, wbk):
    wks = wbk.add_worksheet(coordinates)
    return wks