import serial
import time

s = serial.Serial(port='/dev/tty.usbmodem1421', baudrate=9600, timeout = 0)

while True:
    x = s.read(1)
    if x:
        print ord(x)
    time.sleep(0.01)
