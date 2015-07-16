#! /usr/bin/env python
import threading
import serial
import time
from datetime import datetime, timedelta
from threading import Lock
from Queue import Queue


class Transport(object):

   def write(self, payload):
      raise NotImplementedError

   def read(self, length, timeout=0):
      raise NotImplementedError

   def fileno(self):
      raise NotImplementedError


class SerialTransport(Transport):

   def __init__(self, port, baudrate):
      self._serial = serial.Serial(port=port, baudrate=baudrate)
      self._lock = Lock()
      # you should handshake here rather than just waiting...
      time.sleep(5)
      Transport.__init__(self)

   def write(self, payload):
      with self._lock:
         return self._serial.write(payload)

   def read(self, length, timeout=0):
      if timeout != self._serial.timeout:
         self._serial.timeout = timeout
      return self._serial.read(length)

   def fileno(self):
      return self._serial.fileno()


class Device(object):

   def __init__(self, transport):
      self._transport = transport

   def _compileMessage(self, *args, **kwargs):
      raise NotImplementedError

   def write(self, *args, **kwargs):
      return self._transport.write(self._compileMessage(*args, **kwargs))

   def read(self, length, timeout=0):
      return self._transport.read(length, timeout)

   def fileno(self):
      return self._transport.fileno()


class BtLight(Device):

   OFF = 0
   SET = 1
   FADE_TO = 2

   class Color(object):
      NULL = 0
      WHITE = 1
      BLUE = 2
      TEAL = 3
      GREEN = 4
      YELLOW = 5
      ORANGE = 6
      RED = 7
      PINK = 8
      PURPLE = 9
      SURPRISE = 10

   def __init__(self, transport):
      Device.__init__(self, transport)

   def _compileMessage(self, action, color=None):
      if color is None:
         color = BtLight.Color.NULL
      return chr((int(color) << 4 | int(action)))

   def off(self):
      return self.write(BtLight.OFF)

   def set(self, color):
      return self.write(BtLight.SET, color)

   def fade_to(self, color):
      return self.write(BtLight.FADE_TO, color)


class LightPlayer(Device):

   OFF = 0
   ON = 1
   REVERSE = 2
   FADE_IN = 3
   FADE_OUT = 4
   SET_LOW = 5
   SET_MED = 6
   SET_HIGH = 7
   QUERY = 8
   EDGE = 9
   TOGGLE = 10

   def __init__(self, transport):
      self._sensors = {}
      Device.__init__(self, transport)

   def validChannel(self, channel):
      return 0 <= channel <= 15

   def addSensor(self, cls, channel):
      channel = int(channel)
      if not self.validChannel(channel):
         raise ValueError("Sensor must be bound to a valid channel")
      self._sensors[channel] = cls(self, channel)

   def register(self, channel, target, *args, **kwargs):
      channel = int(channel)
      if not self.validChannel(channel) or channel not in self._sensors:
         raise ValueError("Must register behavior to a valid sensor.  Tried {0}, available {1}".format(
             channel, str(self._sensors.keys())))
      return self._sensors[channel].register(target, *args, **kwargs)

   def register_and_wait(self, channel):
      channel = int(channel)
      event = threading.Event()
      self.register(channel, event.set)
      event.wait()
      event.clear()
      return "True"

   def poll(self, channel=None):
      if channel is not None:
         channel = int(channel)
         if not self.validChannel(channel):
            raise ValueError("Must poll a valid channel")
         if channel in self._sensors:
            return self._sensors[channel].poll()
      else:
         for sensor in self._sensors.values():
            sensor.poll()

   def _compileMessage(self, channel, action):
      # This makes it a little clearer what you're trying to accomplish
      # probably...
      action = int(action)
      channel = int(channel)
      if action >= (1 << 4) or action < 0:
         raise ValueError("Action overflow: action={0}".format(action))
      if not self.validChannel(channel):
         raise ValueError(
             "Valid channel required: channel={0}".format(channel))
      return chr((channel << 4) | action)

   def wait(self, seconds):
      limit = datetime.now() + timedelta(seconds=seconds)
      while datetime.now() < limit:
         pass

   def query(self, channel):
      self.write(channel, LightPlayer.QUERY)

   def update(self):
      result = self.read(1)
      if result:
         op = ord(result)
         if op < 3:
            self._sensors[op].update(LightPlayer.EDGE)
         else:
            op -= 3
            channel = (op / 100) + 1
            value = op - ((channel - 1) * 100)
            self._sensors[channel].update(value)

   def on(self, channel):
      return self.write(channel, LightPlayer.ON)

   def low(self, channel):
      return self.write(channel, LightPlayer.SET_LOW)

   def med(self, channel):
      return self.write(channel, LightPlayer.SET_MED)

   def high(self, channel):
      return self.write(channel, LightPlayer.SET_HIGH)

   def fade_in(self, channel):
      result = self.write(channel, LightPlayer.FADE_IN)
      time.sleep(2.5)
      return result

   def fade_out(self, channel):
      result = self.write(channel, LightPlayer.FADE_OUT)
      time.sleep(2.5)
      return result

   def off(self, channel):
      return self.write(channel, LightPlayer.OFF)

   def reverse(self, channel):
      return self.write(channel, LightPlayer.REVERSE)

   def toggle(self, channel):
      return self.write(channel, LightPlayer.TOGGLE)


class SerialLightPlayer(LightPlayer):

   def __init__(self, port, baudrate):
      LightPlayer.__init__(self, SerialTransport(port, baudrate))


class SerialBtLight(BtLight):

   def __init__(self, port, baudrate):
      BtLight.__init__(self, SerialTransport(port, baudrate))


class WifiLightPlayer(LightPlayer):
   pass
  # specify correct transport, expose proper options in __init__
  # def __init__(self, ip, port):
  #   pass

  # SEE!  now you can do all the TCP partial read/ partial write handling in
  # the transport specific read/write methods which are, as of yet, not
  # implemented

