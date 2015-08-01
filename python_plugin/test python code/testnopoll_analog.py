import serial
import time
from random import randint

s = serial.Serial(port='/dev/tty.usbmodem1421', baudrate=9600, timeout = 0.01)

while True:
    x = s.read(1)
    if x:
        print ord(x)
    time.sleep(0.01)
    y = randint(1,200)
    if y == 1:
        s.write(chr(129))
    if y == 2:
        s.write(chr(130))
 
        
    
