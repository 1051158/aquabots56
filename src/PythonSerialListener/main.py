import serial
import json
import trilateration

# define comport to which the tag is connected
comPort = "COM15"

def serialReadLoop(targetPort, onIncoming):
    # initialize serial connection
    serialPort = serial.Serial(port=targetPort, baudrate=115200, bytesize=8, stopbits=serial.STOPBITS_ONE)
    while True:
        # Wait until there is data waiting in the serial buffer
        if serialPort.in_waiting > 0:

            # Read data out of the buffer until a new line is found
            serialString = serialPort.readline()
            # pass the incoming data to our parsing function
            try:
                onIncoming(serialString.decode("Ascii"))
            except:
                pass


def parseJson(jsonString):
    incomingList = json.loads(jsonString)
    activeAnchors = [anchor for anchor in incomingList if anchor["active"]]
    if len(activeAnchors) >= 3:
        print(trilateration.getCoordinates(activeAnchors[0], activeAnchors[1], activeAnchors[2]))



# start eventLoop
serialReadLoop(comPort, parseJson)
