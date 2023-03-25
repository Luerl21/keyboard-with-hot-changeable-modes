import warnings
import serial
import serial.tools.list_ports
import json

def serial_ports():
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if 'Arduino' in p.description  # may need tweaking to match new arduinos
    ]
    if not arduino_ports:
        raise IOError("No Arduino found")
    if len(arduino_ports) > 1:
        warnings.warn('Multiple Arduinos found - using the first')
    return arduino_ports[0]

data = {"firstFrame": str(input("firstFrame text: "))}
if(len(data["firstFrame"]) > 12):
    print("max text len: 12")
else:
    ser = serial.Serial(serial_ports(), baudrate= 115200, timeout=0.5)
    data = json.dumps(data)
    if ser.isOpen(): 
        (ser.write(bytes(data, "utf-8")))
        try:
            incoming = ser.readline().decode("utf-8")
            print ((incoming))
        except Exception as e:
            print (e)
        ser.close()
    else:
        print ("opening error")