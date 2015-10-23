#! /usr/bin/env python
import threading
import serial
import time
from datetime import datetime, timedelta
from threading import Lock
from Queue import Queue

import os
import time
from multiprocessing import Pipe, Process


class Transport(object):

   def write(self, payload):
      raise NotImplementedError

   def read(self, length, timeout=0):
      raise NotImplementedError

   def fileno(self):
      raise NotImplementedError


class BlockingMidi(Transport):

   def __init__(self, midi_in_id):
      self.pipe, child_pipe = Pipe()
      self.process = Process(
          target=self.__run_polling_proc, args=(child_pipe, midi_in_id))
      self.process.start()

   def read(self, length, timeout=0):
      recvd = []
      while len(recvd) < length:
         recvd += self.pipe.recv()
      print recvd
      return recvd

   def fileno(self):
      return self.pipe.fileno()

   def close(self):
      self.pipe.send(0)
      while True:
         self.process.join(.1)

   def __del__(self):
      self.close()

   def __run_polling_proc(self, child_pipe, midi_id):
      import pygame.midi as midi
      midi.init()
      midi_dev = None
      for i in xrange(midi.get_count()):
         info = midi.get_device_info(i)
         if info[1] == midi_id and info[2]:
            midi_dev = midi.Input(i)
            break
      while midi_dev:
         # TODO performance tune to find acceptable wait length
         if midi_dev.poll():
            while midi_dev.poll():
               child_pipe.send(midi_dev.read(1))


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


class SensingDevice(Device):

   def __init__(self, transport):
      self._sensors = {}
      Device.__init__(self, transport)

   def addSensor(self, sensor_cls, sensor_id):
      if not self.validChannel(sensor_id):
         raise RuntimeError("Sensor must be bound using valid sensor id")
      if sensor_id not in self._sensors:
         self._sensors[sensor_id] = []
      self._sensors[sensor_id].append(sensor_cls(self, sensor_id))

   def _update_sensor(self, sensor_id, data):
      if not self.validChannel(sensor_id):
         raise RuntimeError("Update sent to unregistered sensor id")
      if sensor_id in self._sensors:
         for sensor in self._sensors[sensor_id]:
            sensor.update(data)

   def update(self):
      raise NotImplementedError

   def validChannel(self, channel):
      return True

   def register(self, channel, target, *args, **kwargs):
      channel = int(channel)
      if not self.validChannel(channel) or channel not in self._sensors:
         raise ValueError("Must register behavior to a valid sensor.  Tried {0}, available {1}".format(
             channel, str(self._sensors.keys())))
      for sensor in self._sensors[channel]:
         sensor.register(target, *args, **kwargs)

   def register_and_wait(self, channel):
      channel = int(channel)
      event = threading.Event()
      self.register(channel, event.set)
      event.wait()
      event.clear()
      return "True"

   def register_and_wait_for_state(self, channel):
      channel = int(channel)
      event = threading.Event()
      self.register(channel, event.set)
      event.wait()
      event.clear()
      return str(self._sensors[channel].state())


class MidiDevice(SensingDevice):

   def __init__(self, device_name):
      transport = BlockingMidi(device_name)
      SensingDevice.__init__(self, transport)

   def update(self):
      message = self.read(1)[0]
      self._update_sensor(message[0][1], message)


class LightPlayer(SensingDevice):
   LIGHT = 0
   SET = 1
   FADE_TO = 2
   OTHER = 3
   MOTOR = 4

   class Color(object):
      RED = 0
      ORANGE = 1
      YELLOW = 2
      GREEN = 3
      BLUE = 4
      PURPLE = 5
      WHITE = 6
      SURPRISE = 7

   class Speed(object):
      SLOW = 0
      FASTER = 1
      FASTEST = 2

   class LightCommand(object):
      ON = 0
      OFF = 1
      FADE_IN = 2
      FADE_OUT = 3
      TOGGLE = 4
      SET_LOW = 5
      SET_MED = 6
      SET_HIGH = 7

   class MotorCommand(object):
      ON = 0
      OFF = 1
      REVERSE = 2
      TOGGLE = 3
      SET_LOW = 4
      SET_MED = 5
      SET_HIGH = 6

   def __init__(self, transport):
      self._sensors = {}
      SensingDevice.__init__(self, transport)

   def validChannel(self, channel):
      return 0 <= channel <= 15

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

   def _compileMessage(self, cmd_type, channel, action):
      # This makes it a little clearer what you're trying to accomplish
      # probably...
      action = int(action)
      channel = int(channel)
      cmd_type = int(cmd_type)
      if cmd_type > 7:
         raise ValueError("Command Type overflow: cmd={0}".format(cmd_type))
      if action > 7:
         raise ValueError("Action overflow: action={0}".format(action))
      if channel > 4:
         raise ValueError("Channel overflow: channel={0}".format(channel))
      return chr((cmd_type << 5) | (channel << 3) | action)

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
         channel = op / 2
         value = op % 2
         self._sensors[channel].update(value)

   def isLight(self, channel):
      return 0 <= channel < 4


# Color Commands
   def on(self, channel):
      if self.isLight(channel):
         return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.ON)
      else:
         return self.write(LightPlayer.MOTOR, 0, LightPlayer.MotorCommand.ON)

   def set(self, channel, color):
      return self.write(LightPlayer.SET, channel, color)

   def fade_to(self, channel, color):
      return self.write(LightPlayer.FADE_TO, channel, color)

   def fade_in(self, channel):
      return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.FADE_IN)

   def fade_slow(self, channel):
      return self.write(LightPlayer.OTHER, LightPlayer.Speed.SLOW, 0)

   def fade_faster(self, channel):
      return self.write(LightPlayer.OTHER, LightPlayer.Speed.FASTER, 0)

   def fade_fastest(self, channel):
      return self.write(LightPlayer.OTHER, LightPlayer.Speed.FASTEST, 0)

   def low(self, channel):
      if self.isLight(channel):
         return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.SET_LOW)
      else:
         return self.write(LightPlayer.MOTOR, 0, LightPlayer.LightCommand.SET_LOW)

   def med(self, channel):
      if self.isLight(channel):
         return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.SET_MED)
      else:
         return self.write(LightPlayer.MOTOR, 0, LightPlayer.MotorCommand.SET_MED)

   def high(self, channel):
      if self.isLight(channel):
         return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.SET_HIGH)
      else:
         return self.write(LightPlayer.MOTOR, 0, LightPlayer.LightCommand.SET_HIGH)

   def fade_out(self, channel):
      return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.FADE_OUT)

   def off(self, channel):
      if self.isLight(channel):
         return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.OFF)
      else:
         return self.write(LightPlayer.MOTOR, 0, LightPLayer.MotorCommand.OFF)

   def reverse(self, channel):
      if self.isLight(channel):
         raise RuntimeError("Invalid command for light: Reverse")
      return self.write(LightPlayer.MOTOR, 0, LightPlayer.MotorCommand.REVERSE)

   def toggle(self, channel):
      if self.isLight(channel):
         return self.write(LightPlayer.LIGHT, channel, LightPlayer.LightCommand.TOGGLE)
      else:
         return self.write(LightPlayer.MOTOR, 0, LightPlayer.MotorCommand.TOGGLE)


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

