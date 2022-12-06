# python implementation of:
# https://math.stackexchange.com/questions/884807/find-x-location-using-3-known-x-y-location-using-trilateration
import numpy as np

global max_anchors
max_anchors = 3


def getCoordinates(anchor1, anchor2, anchor3):

    # get needed calculation values
    A = (-2*anchor1["x"]) + (2*anchor2["x"])
    B = (-2*anchor1["y"]) + (2*anchor2["y"])
    C = (anchor1["distance"]**2) - (anchor2["distance"]**2) - (anchor1["x"]**2) + (anchor2["x"]**2) - (anchor1["y"]**2) + (anchor2["y"]**2)
    D = (-2*anchor2["x"]) + (2*anchor3["x"])
    E = (-2*anchor2["y"]) + (2*anchor3["y"])
    F = anchor2["distance"]**2 - anchor3["distance"]**2 - anchor2["x"]**2 + anchor3["x"]**2 - anchor2["y"]**2 + anchor3["y"]**2

    # plug numbers for solution
    x = ((C*E) - (F*B))/((E*A) - (B*D))
    y = ((C*D) - (A*F))/((B*D) - (A*E))

    return x, y

def getCoordinates1(x, y, d_2, d_1, d):
    # get needed calculation values
    j = 0

    for j in range(max_anchors):
        x[j] = float(x[j])
        y[j] = float(y[j])
    #print(x)
    #print(y)
    d = float(d)
    d_1 = float(d_1)
    d_2 = float(d_2)
    A = (-2*x[0]) + (2*x[1])
    #print(A)
    B = (-2*y[0]) + (2*y[1])
    #print(B)
    C = (d**2) - (d_1**2) - (x[0]**2) + (x[1]**2) - (y[0]**2) + (y[1]**2)
    #print(C)
    D = (-2*x[1]) + (2*x[2])
    #print(D)
    E = (-2*y[1]) + (2*y[2])
    #print(E)
    F = (d_1**2) - (d_2**2) - (x[1]**2) + (x[2]**2) - (y[1]**2) + (y[2]**2)
    #print(F)
    # plug nfumbers for solution
    x = ((C * E) - (F * B)) / ((E * A) - (B * D))
    y = ((C * D) - (A * F)) / ((B * D) - (A * E))
    print(x, y)

    return x, y
