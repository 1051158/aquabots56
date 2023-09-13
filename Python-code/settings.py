import xls  # Import a module named 'xls'
from dataclasses import dataclass  # Import the 'dataclass' decorator
import numpy as np  # Import the 'numpy' library and alias it as 'np'
import math as m  # Import the 'math' library and alias it as 'm'
import array  # Import the 'array' module

# Define a data class 'anchors' with x, y, and z attributes
@dataclass
class anchors:
    x: float  # X-coordinate
    y: float  # Y-coordinate
    z: float  # Z-coordinate

# Define another data class 'strctSet' with various attributes
@dataclass
class strctSet:
    # Public values that can be filled in using 'excelSettings()'
    dbgFirstRow: bool  # Debug option
    dbgInteg: bool     # Debug option
    dbgReq: bool       # Debug option
    dbgTril: bool      # Debug option
    dbgXls: bool       # Debug option
    xyzTest: bool      # XYZ test option
    speedTest: bool    # Speed test option
    draw: bool         # Draw option
    excel: bool        # Excel option
    nos: int           # Number of something (not clear from context)
    dcm: int           # Another integer value
    max_dcm: int       # Maximum dcm value
    reset_dcm: int     # Reset dcm value
    dci: int           # Yet another integer value
    startSend: bool = False         # Private value (default to False)
    back: bool = False              # Private value (default to False)
    wks_count: int = 0              # Private value (default to 0)
    num_of_send_counter: int = 0    # Private value (default to 0)
    position_y: int = 1             # Private value (default to 1)
    ADelay_1: int = 16550           # Private value (default to 16550)
    ADelay_2: int = 16550           # Private value (default to 16550)
    ADelay_3: int = 16550           # Private value (default to 16550)
    ADelay_4: int = 16550           # Private value (default to 16550)
    IP_A1: str = 'http://192.168.0.103'  # Private value (default IP address)
    IP_A2: str = 'http://192.168.0.104'  # Private value (default IP address)
    IP_A3: str = 'http://192.168.0.100'  # Private value (default IP address)
    IP_A4: str = 'http://192.168.0.102'  # Private value (default IP address)

# Function to enter all the layout settings of Excel
def allSettings(Coordinates):
    # Define and initialize various settings
    dbgFirstRow = False
    dbgInteg = False
    dbgReq = False
    dbgTril = False
    dbgXls = False
    speedTest = True
    draw = False
    excel = False
    nos = 5
    dcm = 1
    max_dcm = 1
    reset_dcm = 1
    dci = 1

    # Set this to False if you don't want to give a name each time the program starts
    give_name = True

    # Conditionally create a new workbook and worksheet
    if excel:
        wbk, mapName = xls.make_new_wbk(give_name)
    else:
        wbk, mapName = 0

    # Determine if 'xyzTest' is True based on the 'mapName'
    if '/x-y-z_test' in mapName:
        xyzTest = True
    else:
        xyzTest = False

    # Create a 'Settings' object of the 'strctSet' data class
    if not excel:
        Settings = strctSet(dbgFirstRow, dbgInteg, dbgReq, dbgTril, dbgXls, xyzTest, speedTest, draw, excel, nos, dcm, max_dcm,
                        reset_dcm, dci)
        return Settings

    # Create a new worksheet for Excel
    if Settings.excel:
        wks = xls.make_new_wks(Coordinates[Settings.wks_count], wbk)
    else:
        wks = 0

    y_coor = 2.4
    z_coor = 0.5

    # Open a figure that will show the coordinate
    if Settings.draw:
        fig, ax, plt = xls.draw.start_figure()

    # Define an array of coordinates
    coordinates = np.array([[2, y_coor, z_coor], [3.5, y_coor, z_coor], [5, y_coor, z_coor], [6.5, y_coor, z_coor], [8, y_coor, z_coor]])

    # Create anchor objects with specific coordinates
    anchor1 = anchors(4.5, 0, 0.9)
    anchor2 = anchors(3, 5.6, 0.9)
    anchor3 = anchors(0.4, 5, 1.1)
    anchor4 = anchors(0.35, 0.73, 1.5)

    # Calculate distances and store them in arrays
    d1, d2, d3, d4 = calculateDistances(anchor1, anchor2, anchor3, anchor4, coordinates)

    # Return workbook, worksheet, settings, figure, anchor objects, and distance arrays
    return wbk, wks, Settings, fig, ax, plt, coordinates, anchor1, anchor2, anchor3, anchor4, d1, d2, d3, d4

# Function to calculate distances between anchor points and coordinates
def calculateDistances(anchor1, anchor2, anchor3, anchor4, coordinates):
    d1 = array.array('f', [])
    d2 = array.array('f', [])
    d3 = array.array('f', [])
    d4 = array.array('f', [])
    for i in range(5):
        # Calculate Euclidean distances
        d1.append(m.sqrt(m.pow(anchor1.x - coordinates[i, 0], 2) + m.pow(anchor1.y - coordinates[i, 1], 2) + m.pow(anchor1.z - coordinates[i, 2], 2)))
        d3.append(m.sqrt(m.pow(anchor3.x - coordinates[i, 0], 2) + m.pow(anchor3.y - coordinates[i, 1], 2) + m.pow(anchor3.z - coordinates[i, 2], 2)))
        d4.append(m.sqrt(m.pow(anchor4.x - coordinates[i, 0], 2) + m.pow(anchor4.y - coordinates[i, 1], 2) + m.pow(anchor4.z - coordinates[i, 2], 2)))
        d2.append(m.sqrt(m.pow(anchor2.x - coordinates[i, 0], 2) + m.pow(anchor2.y - coordinates[i, 1], 2) + m.pow(anchor2.z - coordinates[i, 2], 2)))
    print(d1, d2, d3, d4)
    return d1, d2, d3, d4
