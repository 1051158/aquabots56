# python implementation of:
# https://math.stackexchange.com/questions/884807/find-x-location-using-3-known-x-y-location-using-trilateration

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

