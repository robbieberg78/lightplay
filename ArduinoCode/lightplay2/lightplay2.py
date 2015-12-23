import serial
import time

s = serial.Serial(port='/dev/cu.usbserial-AI02KF7L', baudrate=9600, timeout = 0)
a=[32,33,34]

def soak():
    while True:
        s.write(chr(32))
        time.sleep(.1)
        x = s.readline()
        if x:
            print(x)
        s.write(chr(33))
        time.sleep(.1)
        x = s.readline()
        if x:
            print(x)
        
def tx(n):
    s.write(chr(n)) # send Arduino a byte,
    #Arduino will respond by sending 3 bytes (ubits, xbits,ybits)
    time.sleep(0.2)  # give buffer time to fill
    while True: 
        x = s.readline()
        if x:
            print(x)
        time.sleep(0.02)


def surprise():
    while True:
           s.write(chr(39))
           time.sleep(1.0)

def fadein(n):
    s.write(chr(32 + n))
    s.write(chr(2))

def fadeout(n):
    s.write(chr(32 + n))
    s.write(chr(3))

def inout(n):
    while True:
        s.write(chr(120))
        fadein(n)
        time.sleep(7)       
        fadeout(n)
        time.sleep(7)

def motoron():
    s.write(chr(128))

def motoroff():
    s.write(chr(129))

def reverse():
    s.write(chr(130))

def setcolor(n):
    s.write(chr(32 + n))

