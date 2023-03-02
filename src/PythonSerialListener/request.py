import requests
import array

import xls
import numpy as np
import time
from time import sleep

#write the wright IP in Visual Studio for the http link
IPaddress = 'http://192.168.2.53'


ssid = 'Machelina'
psswrd = 'Donjer01'

multiple_test = True

#put the link of the Server to get the info from
def getRequest(server_address):
    #combine the IPadress and Server address
    response = requests.get(IPaddress + server_address)
    #print('status code')
    #print(response.status_code)
    #print(response.text)
    return response.text

def getValues(WiFistring):
    anchorDistances = WiFistring.split('\t\n')
    #print(anchorDistances)

    #get the Coordinates array from the string
    Coordinates = array.array('i', [])
    hulp_coordinates = anchorDistances.pop(0)
    hulp_coordinates = hulp_coordinates.split(';')
    Coordinates = hulp_coordinates
    del Coordinates[len(hulp_coordinates)-1]
    print(Coordinates)
    #get the Antenna delay info from and the send info string
    ADS = anchorDistances[0]
    #print(ADS)
    max_anchors = ADS.split('max')
    AD_start = max_anchors[1].split('S')
    AD_end = AD_start[1].split('E')
    AD_interval = AD_end[1].split('I')
    NOS = AD_interval[1].split('nos')
    DCM = NOS[1].split('-')
    reset_dis_max = DCM[1].split('r+')
    DCI = reset_dis_max[1].split('in')
    max_anchors = max_anchors.pop(0)
    print(max_anchors)
    AD_start = AD_start.pop(0)
    AD_end = AD_end.pop(0)
    AD_interval = AD_interval.pop(0)
    NOS = NOS.pop(0)
    DCM = DCM.pop(0)
    DCI = DCI.pop(0)
    reset_dis_max = reset_dis_max.pop(0)
    print(AD_start, AD_end, AD_interval, NOS, DCM, reset_dis_max, DCI)
    if max_anchors == 1:
        return max_anchors, Coordinates, AD_start, AD_end, AD_interval, NOS, DCM, reset_dis_max, DCI
    #get the info of all enchors from the string
    ID_array = array.array('i', [])
    x_array = array.array('f', [])
    y_array = array.array('f', [])
    z_array = array.array('f', [])
    del anchorDistances[0]
    print(anchorDistances)
    Distance_array = [[0] * len(Coordinates)] * (len(anchorDistances)-1)

    for i in range(len(anchorDistances)-1):
        help = anchorDistances[i].split('i')
        #print(help)
        help1 = help[1].split('x')
        help2 = help1[1].split('y')
        help3 = help2[1].split('z')
        help = help.pop(0)
        help1 = help1.pop(0)
        help2 = help2.pop(0)
        help3 = help3.pop(0)
        Distances = anchorDistances[i].split('\t')
        del Distances[0]
        #print(Distances)
        ID_array.append(int(help))
        x_array.append(float(help1))
        y_array.append(float(help2))
        z_array.append(float(help3))
        Distance_array[i] = Distances

    print(Distance_array)
    #print(x_array)
    #print(y_array)
    #print(ID_array)

    return max_anchors, Coordinates, AD_start, AD_end, AD_interval, ID_array, x_array, y_array, z_array, Distance_array, NOS, DCM, reset_dis_max, DCI

