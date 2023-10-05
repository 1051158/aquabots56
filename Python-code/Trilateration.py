# python implementation of:
# https://math.stackexchange.com/questions/884807/find-x-location-using-3-known-x-y-location-using-trilateration
import numpy as np

global max_anchors
max_anchors = 3


# P are the xyz coordinates of every anchor
def trilaterationtest():
    P1 = np.array([4.5, 0, 0.9])
    P2 = np.array([3, 5.6, 0.9])
    P3 = np.array([0.35, 0.73, 1.5])
    r1 = 3.35
    r2 = 3.32
    r3 = 2.69
    p1 = np.array([0, 0, 0])
    p2 = np.array([P2[0] - P1[0], P2[1] - P1[1], P2[2] - P1[2]])
    p3 = np.array([P3[0] - P1[0], P3[1] - P1[1], P3[2] - P1[2]])
    v1 = p2 - p1
    v2 = p3 - p1

    Xn = (v1) / np.linalg.norm(v1)

    tmp = np.cross(v1, v2)

    Zn = (tmp) / np.linalg.norm(tmp)

    Yn = np.cross(Xn, Zn)

    i = np.dot(Xn, v2)
    d = np.dot(Xn, v1)
    j = np.dot(Yn, v2)

    X = ((r1**2) - (r2**2) + (d**2)) / (2 * d)
    Y = (((r1**2) - (r3**2) + (i**2) + (j**2)) / (2 * j)) - ((i / j) * (X))
    Z1 = np.sqrt(max(0, r1**2 - X**2 - Y**2))
    Z2 = -Z1

    K1 = P1 + X * Xn + Y * Yn + Z1 * Zn
    K2 = P1 + X * Xn + Y * Yn + Z2 * Zn
    print(K1, K2)
    return K1, K2


def triliterationnew3D(x_array, y_array, z_array, r1, r2, r3, ID, Settings):
    if Settings.dbgTril:
        print("ID: ", ID)
    ID_0 = int(ID[0])
    ID_1 = int(ID[1])
    ID_2 = int(ID[2])
    P1 = np.array([x_array[ID_0], y_array[ID_0], z_array[ID_0]])
    P2 = np.array([x_array[ID_1], y_array[ID_1], z_array[ID_1]])
    P3 = np.array([x_array[ID_2], y_array[ID_2], z_array[ID_2]])
    if Settings.dbgTril:
        print("P1: ", P1)
        print("P2: ", P2)
        print("P3: ", P3)
    r1 = float(r1)
    r2 = float(r2)
    r3 = float(r3)
    print(r1, r2, r3)
    p1 = np.array([0, 0, 0])
    p2 = np.array([P2[0] - P1[0], P2[1] - P1[1], P2[2] - P1[2]])
    p3 = np.array([P3[0] - P1[0], P3[1] - P1[1], P3[2] - P1[2]])
    v1 = p2 - p1
    v2 = p3 - p1

    Xn = (v1) / np.linalg.norm(v1)

    tmp = np.cross(v1, v2)

    Zn = (tmp) / np.linalg.norm(tmp)

    Yn = np.cross(Xn, Zn)

    i = np.dot(Xn, v2)
    d = np.dot(Xn, v1)
    j = np.dot(Yn, v2)

    X = ((r1**2) - (r2**2) + (d**2)) / (2 * d)
    Y = (((r1**2) - (r3**2) + (i**2) + (j**2)) / (2 * j)) - ((i / j) * (X))
    Z1 = np.sqrt(max(0, r1**2 - X**2 - Y**2))
    Z2 = -Z1

    K1 = P1 + X * Xn + Y * Yn + Z1 * Zn
    K2 = P1 + X * Xn + Y * Yn + Z2 * Zn
    if K1[2] == "nan" or K2[2] == "nan":
        K1 = np.array([0, 0, 0])
        K2 = np.array([0, 0, 0])

    print(K1, K2)
    return K1, K2


def getCoordinates(anchor1, anchor2, anchor3):
    # get needed calculation values
    A = (-2 * anchor1["x"]) + (2 * anchor2["x"])
    B = (-2 * anchor1["y"]) + (2 * anchor2["y"])
    C = (
        (anchor1["distance"] ** 2)
        - (anchor2["distance"] ** 2)
        - (anchor1["x"] ** 2)
        + (anchor2["x"] ** 2)
        - (anchor1["y"] ** 2)
        + (anchor2["y"] ** 2)
    )
    D = (-2 * anchor2["x"]) + (2 * anchor3["x"])
    E = (-2 * anchor2["y"]) + (2 * anchor3["y"])
    F = (
        anchor2["distance"] ** 2
        - anchor3["distance"] ** 2
        - anchor2["x"] ** 2
        + anchor3["x"] ** 2
        - anchor2["y"] ** 2
        + anchor3["y"] ** 2
    )

    # plug numbers for solution
    x = ((C * E) - (F * B)) / ((E * A) - (B * D))
    y = ((C * D) - (A * F)) / ((B * D) - (A * E))

    return x, y


def getCoordinates1(x, y, d_2, d_1, d):
    # get needed calculation values
    j = 0

    for j in range(max_anchors):
        x[j] = float(x[j])
        y[j] = float(y[j])
    # print(x)
    # print(y)
    d = float(d)
    d_1 = float(d_1)
    d_2 = float(d_2)
    A = (-2 * x[0]) + (2 * x[1])
    # print(A)
    B = (-2 * y[0]) + (2 * y[1])
    # print(B)
    C = (d**2) - (d_1**2) - (x[0] ** 2) + (x[1] ** 2) - (y[0] ** 2) + (y[1] ** 2)
    # print(C)
    D = (-2 * x[1]) + (2 * x[2])
    # print(D)
    E = (-2 * y[1]) + (2 * y[2])
    # print(E)
    F = (d_1**2) - (d_2**2) - (x[1] ** 2) + (x[2] ** 2) - (y[1] ** 2) + (y[2] ** 2)
    # print(F)
    # plug nfumbers for solution
    x = ((C * E) - (F * B)) / ((E * A) - (B * D))
    y = ((C * D) - (A * F)) / ((B * D) - (A * E))
    print(x, y)

    return x, y
