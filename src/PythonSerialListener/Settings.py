import xls
from dataclasses import dataclass

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


# enter all the lay out settings of excel here.
def excelSettings(Coordinates):
    # fill in the variables underneath the code will fill them in the struct some variables are private(can't be changed)

    dbgFirstRow = False
    dbgInteg = False
    dbgReq = False
    dbgTril = True
    dbgXls = True
    speedTest = True
    draw = True
    excel = True
    nos = 7
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

    # open the figure that will show the coordinate
    if Settings.draw:
        fig, ax, plt = xls.draw.start_figure()

    # return the workbook , worksheet, struct and figure
    return wbk, wks, Settings, fig, ax, plt