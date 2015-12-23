#! /usr/bin/env python

from threading import Event
from threading import Thread
from arduino import LightPlayer
import serial
import time
import struct
'''This is the most general outline for what a sensor should do.  All sensors should be able to register a new behaviour
which is slated to be run with some arguments.  Also, all sensors must be pollable.  That is, we can call some function
poll which updates the sensor state'''


class BaseSensor(object):

   def __init__(self):
      self.__on_update = Event()

   def register(self, target, *args, **kwargs):
      raise NotImplementedError

   def poll(self):
      raise NotImplementedError

   def update(self, value):
      raise NotImplementedError

   def wait(self):
      self.__on_update.wait()
      self.__on_update.clear()

   def on_update(self):
      self.__on_update.set()


'''Now, we'll start defining some actual sensor behavior.  This particular sensor keep a list of interested functions.  Registered listeners will be
started on a new thread once the event is raised.  NOTE, poll is not implemented, yet...'''


class EventSensor(BaseSensor):

   def __init__(self):
      self._listeners = []
      BaseSensor.__init__(self)

   def register(self, target, *args, **kwargs):
      self._listeners.append((target, args, kwargs))

   def raise_event(self):
      print "RAISED"
      for target, args, kwargs in self._listeners:
         t = Thread(target=target, args=args, kwargs=kwargs)
         t.start()

'''Now, with the underlying machinery out of the way, we can define some sensor behavior.  Namely, this edge triggered sensor has a
high and low state, and polling this sensor looks for changes in this state, raising events as neccessary'''


class EdgeTriggeredSensor(EventSensor):

   def __init__(self):
      EventSensor.__init__(self)
      self._state = False
      self._last_state = self._state

   def poll(self):
      if self._state != self._last_state:
         self._last_state = self._state
         self.raise_event()
      return self._state

   def high_state(self):
      self._state = True
      self.raise_event()

   def low_state(self):
      self._state = False
      self.raise_event()

   def state(self):
      return self._state


'''This will prompt the user for a state, then call the EdgeTriggeredSensor's poll() implementation to fire if appropriate'''


class PromptingEdgeTriggeredSensor(EdgeTriggeredSensor):

   def __init__(self, name):
      self._name = name
      EdgeTriggeredSensor.__init__(self)

   def poll(self):
      command = raw_input(
          "What state should {0} be in?  'H' for high, 'L' for low, any other key to not change: ".format(self._name))
      if command == "H":
         self.high_state()
      if command == "L":
         self.low_state()
      return EdgeTriggeredSensor.poll(self)


class ArduinoEdgeTriggeredSensor(EdgeTriggeredSensor):
   # you fill this in, but the idea is to save a reference to whatever polling thing you need, and then ask for an update in poll
   # similar to what was done above

   def __init__(self, arduino, channel):
      self._channel = channel
      self._arduino = arduino
      EdgeTriggeredSensor.__init__(self)

   def poll(self):
      result = self._arduino.query(self._channel)
      if result is not None:
         self.high_state() if result >= 50 else self.low_state()
      return EdgeTriggeredSensor.poll(self)

   def update(self, value):
      self.high_state() if not value else self.low_state()


class ArduinoAnalogSensor(EventSensor):

   def __init__(self, arduino, channel):
      self._channel = channel
      self._arduino = arduino
      self.__last_value = None
      EventSensor.__init__(self)

   def poll(self):
      self._arduino.query(self._channel)
      self.wait()
      return self.__last_value

   def update(self, value):
      if value != self.__last_value:
         self.__last_value = value
         self.raise_event()
      else:
         self.__last_value = value
         self.on_update()


class MidiEventSensor(EventSensor):

   def __init__(self, midi_device, midi_code):
      self._midi_code = midi_code
      self._midi_device = midi_device
      self.__last_value = None
      EventSensor.__init__(self)

   def poll(self):
      self.wait()
      return self.__last_value

   def update(self, value):
      if value:
         print "VALUE"
         self.__last_Value = value
         self.raise_event()

