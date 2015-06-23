#!/usr/bin/env python
import serial
import time
import threading

# This code uses simple protocol for encoding a command and a channel number in a single byte sent from Python to Arduino:
# The lower nibble is  the command type: on=0, off=1, reverse=2, 8 = report digital sensor level 9 = report analog sensor value
# high nibble is channel (channels 0-7 are outputs, channel 8-15 are inputs)
# Our demo will use, channel 0 for "all" outputs, channels 1-7 for individual outputs (demo uses only channels 1-4)
# and channels 8-15  for  sensors  (demo uses only channel 8)

# open the serial port to talk to Arduino at 9600 baud

s = serial.Serial(port='/dev/tty.usbmodem1411', baudrate=9600)


# in the next three defs n is the channel number, multiplying by 16 shifts
# this number to the upper nibble
def off(n):
   s.write(chr(16 * n))


def on(n):
   print n
   s.write(chr(16 * n + 1))


def reverse(n):
   s.write(chr(16 * n + 2))

   # report digital sensor *level*,  edge detection is done in TriggerThread below
   # since demo has a lone sensor channel, no need for a channel input here


def connection():
   s.write(chr(0x88))  # request a sensor value
   # read one byte response from Arduino, which will be '0' if button is
   # pressed and '1'
   x = s.read(1)
   # confusing point: the Arduino is actually sending binary reps of 0 and 1,
   # but serial.read expects ascii
   if ord(x) == 1:  # ord function converts from ASCII code back to integer
      return False  # if button is not pressed
   else:
      return True  # if button pressed

# wait for n seconds


def wait(n):
   time.sleep(n)

# turn on all the lights for n seconds


def flash(n):
   on(0)
   wait(n)
   off(0)

# trigger code on sensor edge,


class TriggerThread(threading.Thread):

   def run(self):
      oldsensor = connection()
      while True:
         newsensor = connection()
         if (newsensor != oldsensor) and (newsensor == True):
            # insert code that runs on trigger here
            reverse(2)
         oldsensor = newsensor

# This trigger can be started from the command window by typing
# "trigger.start()"
trigger = TriggerThread()


# I think the serial init resets the Arduino, so wait here for the Arduino
# to restart
wait(5)

# flash all the actuators on to say hello
flash(0.2)

