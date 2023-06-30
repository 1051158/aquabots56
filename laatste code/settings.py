import xls
from dataclasses import dataclass
import numpy as np
import math as m
import array

@dataclass
class anchors:
    x: float
    y: float
    z: float


@dataclass
class strctSet:
    # public values (can be filled in, in excelSettings())
    dbgFirstRow: bool
    dbgInteg: bool
    dbgReq: bool
    dbgTril: bool
    dbgXls: bool
    xyzTest: bool
    speedTest: bool
    draw: bool
    excel: bool
    nos: int
    dcm: int
    max_dcm: int
    reset_dcm: int
    dci: int
    startSend: bool = False  # private values of the class
    back: bool = False
    wks_count: int = 0
    num_of_send_counter: int = 0
    position_y: int = 1
    ADelay_1: int = 16550
    ADelay_2: int = 16550
    ADelay_3: int = 16550
    ADelay_4: int = 16550
    IP_A1: str = 'http://192.168.0.103'
    IP_A2: str = 'http://192.168.0.104'
    IP_A3: str = 'http://192.168.0.100'
    IP_A4: str = 'http://192.168.0.102'


# enter all the lay out settings of excel here.
def allSettings(Coordinates):
    # fill in the variables underneath the code will fill them in the struct some variables are private(can't be changed)

    dbgFirstRow = False
    dbgInteg = False
    dbgReq = False
    dbgTril = False
    dbgXls = False
    speedTest = True
    draw = True
    excel = True
    nos = 5
    dcm = 1
    max_dcm = 1
    reset_dcm = 1
    dci = 1

    # set this false if you don't want to give a name each time the program starts
    give_name = True
    if excel:
        wbk, mapName = xls.make_new_wbk(give_name)
    else:
        wbk, mapName = 0
    # xyz_test depends on the map the wbk comes in:
    if '/x-y-z_test' in mapName:
        xyzTest = True
    else:
        xyzTest = False
    Settings = strctSet(dbgFirstRow, dbgInteg, dbgReq, dbgTril, dbgXls, xyzTest, speedTest, draw, excel, nos, dcm, max_dcm,
                        reset_dcm, dci)
    # make a worksheet for excel
    if Settings.excel:
        wks = xls.make_new_wks(Coordinates[Settings.wks_count], wbk)
    else:
        wks = 0
    y_coor = 2.4
    z_coor = 0.5
    # open the figure that will show the coordinate
    if Settings.draw:
        fig, ax, plt = xls.draw.start_figure()

    coordinates = np.array([[2, y_coor, z_coor], [3.5, y_coor, z_coor], [5, y_coor, z_coor], [6.5, y_coor, z_coor], [8, y_coor, z_coor]])
    print(len(coordinates))
    anchor1 = anchors(4.5, 0, 0.9)
    anchor2 = anchors(3, 5.6, 0.9)
    anchor3 = anchors(0.4, 5, 1.1)
    anchor4 = anchors(0.35, 0.73, 1.5)
    d1, d2, d3, d4 = calculateDistances(anchor1, anchor2, anchor3, anchor4, coordinates)
    # return the workbook , worksheet, struct and figure
    return wbk, wks, Settings, fig, ax, plt, coordinates, anchor1, anchor2, anchor3, anchor4, d1, d2, d3, d4

def calculateDistances(anchor1, anchor2, anchor3, anchor4, coordinates):
    d1 = array.array('f', [])
    d2 = array.array('f', [])
    d3 = array.array('f', [])
    d4 = array.array('f', [])
    for i in range(5):
        d1.append(m.sqrt(m.pow(anchor1.x - coordinates[i, 0], 2) + m.pow(anchor1.y - coordinates[i, 1], 2) + m.pow(anchor1.z - coordinates[i, 2], 2)))
        d3.append(m.sqrt(m.pow(anchor3.x - coordinates[i, 0], 2) + m.pow(anchor3.y - coordinates[i, 1], 2) + m.pow(anchor3.z - coordinates[i, 2], 2)))
        d4.append(m.sqrt(m.pow(anchor4.x - coordinates[i, 0], 2) + m.pow(anchor4.y - coordinates[i, 1], 2) + m.pow(anchor4.z - coordinates[i, 2], 2)))
        d2.append(m.sqrt(m.pow(anchor2.x - coordinates[i, 0], 2) + m.pow(anchor2.y - coordinates[i, 1], 2) + m.pow(anchor2.z - coordinates[i, 2], 2)))
    print(d1, d2, d3, d4)
    return d1, d2, d3, d4



