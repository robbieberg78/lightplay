import serial
import time
import threading

# This code uses simple protocol for encoding a command and a channel number in a single byte sent from Python to Arduino:
# The lower nibble is  the command type: on=0, off=1, reverse=2, 8 = report analog sensor level 9 = report analog sensor value
# high nibble is channel (channels 0-7 are outputs, channel 8-15 are inputs)
# Our demo will use, channel 0 for "all" outputs, channels 1-7 for individual outputs (demo uses only channels 1-4)
# and channels 8-15  for  sensors  (demo uses only channel 8)

# open the serial port to talk to Arduino at 9600 baud

s = serial.Serial(port='/dev/tty.usbmodem1421', baudrate=9600)


# in the next three defs n is the channel number, multiplying by 16 shifts this number to the upper nibble
def off(n):
    s.write(chr(16*n))

def on(n):
    s.write(chr(16*n+1))

def reverse(n):
       s.write(chr(16*n+2))

    # report digital sensor *level*,  edge detection is done in TriggerThread below
    # since demo has a lone sensor channel, no need for a channel input here
def sensor1():
    s.write(chr(0x18)) # request an analog value from sensor 1

def sensor2():
    s.write(chr(0x28)) # request an analog value from sensor 2

# wait for n seconds
def wait(n):
    time.sleep(n)

# turn on all the lights for n seconds
def flash(n):
    on(0)
    wait(n)
    off(0)

# trigger code on sensor edge,
class SensorThread(threading.Thread):
    def run(self):
        while True:
            x = s.read(1)
            if x:
                if (ord(x) == 1):
                    print "edge1"
                if (ord(x) == 2):
                    print "edge2"
                if (ord(x) >= 3) and (ord(x) <= 103):
                    print "The analog value of sensor 1 is {0}".format(ord(x) - 3)
                if (ord(x) >= 104) and (ord(x) <= 204):
                    print "The analog value of sensor 2 is {0}".format(ord(x) - 104)
            time.sleep(0.01)
        
 

# This trigger can be started from the command window by typing "sensor.start()"
sensor = SensorThread()


wait(5)  # I think the serial init resets the Arduino, so wait here for the Arduino to restart
        
## flash all the actuators on to say hello
flash(0.2)






