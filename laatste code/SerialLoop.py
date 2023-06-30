import Serial

def serialReadLoop(targetPort, onIncoming):
    # if the file already exists give a new name if ypu don't want to overwrite the file
    # initialize serial connection
    wks_count = 0
    wks_bool = 0
    testName = '/x-y_test'  # name of the kind of test(choose between '/rangetest' or '/x_y-test')
    wbk = xls.make_new_wbk(testName)
    float_distance = 7
    wks_names = np.array(['(1,1)', '(9,1)', '(5,5)', '(1,9)', '(9,9)'])
    wks = xls.make_new_wks(wks_names[wks_count], wbk)
    aanstaan = False
    end_count = 0
    #  distance = 7
    position_x = 0
    position_y = 1
    distances_per_interval = 2  # number of measured intervals to test accuracy
    serialPort = serial.Serial(port=targetPort, baudrate=115200, bytesize=8, stopbits=serial.STOPBITS_ONE)
    xls.first_rows_excel(position_x, wks, distances_per_interval, testName)
    # uncomment to test if first row is succeeded
    # wbk.close()
    # sys.exit(0)
    while True:
        if serialPort.in_waiting > 0:  # if serial buffer is empty nothing happens
            # Read data out of the buffer until a new line is found
            serialString = serialPort.readline()  # read the serial line
            serialString1 = serialString.decode('Ascii')
            print(serialString)  # check the incoming string

            if end_with_button:
                if 'end' in serialString1:  # if string contains 'end', stop program when stop with button is used
                    end_count += 1
                    if end_count >= 2:
                        wbk.close()
                        sys.exit(0)
                    float_distance -= 0.5
                    wks = xls.make_new_wks(wks_names[end_count], wbk)
                    position_y = 1
                    position_x = 0
                    wks_count += 1
                    xls.first_rows_excel(position_x, wks, distances_per_interval, testName)#write AD and time in ms for the anchors
                    position_x = 1
                else:
                    if backspace_active and position_y >= 2 and 'back' in serialString1:
                        position_y -= 1  # go colom back in excel after a wrong measurent
                        print(position_y)

                    if '192.168.4.2' in serialString1 and aanstaan == False:
                        position_x = 1
                        position_y = 1
                        aanstaan = True
                    try:
                        print('y')
                        print(position_y)
                        print('x')
                        print(position_x)
                        if 'a' in serialString1:
                            position_y += 3
                            position_x = 1
                        elif 'e' in serialString1:
                            position_y += 2
                            position_x = 1
                        else:
                            position_x += 1
                            onIncoming(serialString1, position_y, position_x, wks, testName)  # check

                    except:
                        pass