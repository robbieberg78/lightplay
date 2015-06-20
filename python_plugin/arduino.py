#! /usr/bin/env python
import serial
import time
from datetime import datetime, timedelta
from Queue import Queue

class LightPlayer(object):

   OFF = 0
   ON = 1
   REVERSE = 2
   QUERY = 8

   def __init__(self):
     self._sensors = {}

   def write(self, payload):
     raise NotImplementedError

   def read(self, length, timeout=0):
      raise NotImplementedError

   def validChannel(self, channel):
      return 0 <= channel <= 15

   def addSensor(self, cls, channel):
      if not self.validChannel(channel):
         raise ValueError("Sensor must be bound to a valid channel")
      self._sensors[channel] = cls(self, channel)

   def register(self, channel, target, *args, **kwargs):
      if not self.validChannel(channel) or channel not in self._sensors:
         raise ValueError("Must register behavior to a valid sensor.  Tried {0}, available {1}".format(channel, str(self._sensors.keys())))
      return self._sensors[channel].register(target, *args, **kwargs)
 
   def poll(self, channel=None):
      if channel is not None:
         if not self.validChannel(channel):
            raise ValueError("Must poll a valid channel")
         if channel in self._sensors:
            self._sensors[channel].poll()
      else:
         for sensor in self._sensors.values():
            sensor.poll()

   def _compileMessage(self, channel, action):
      #  This makes it a little clearer what you're trying to accomplish probably...
      if action >= (1 << 4) or action < 0:
         raise ValueError("Action overflow: action={0}".format(action))
      if not self.validChannel(channel):
         raise ValueError("Valid channel required: channel={0}".format(channel))
      return chr((channel << 4) | action)

   def wait(self, seconds):
      limit = datetime.now() + timedelta(seconds=seconds)
      while datetime.now() < limit:
         pass

   def query(self, channel):
      self.write(self._compileMessage(channel, LightPlayer.QUERY))
      return ord(self.read(1)) == LightPlayer.ON 

   def on(self, channel):
      return self.write(self._compileMessage(channel, LightPlayer.ON))

   def off(self, channel):
      return self.write(self._compileMessage(channel, LightPlayer.OFF))

   def reverse(self, channel):
      return self.write(self._compileMessage(channel, LightPlayer.REVERSE))


class SerialLightPlayer(LightPlayer):

   def __init__(self, port, baudrate):
      self._transport = serial.Serial(port=port, baudrate=baudrate)
      #you should handshake here rather than just waiting...
      time.sleep(5)
      LightPlayer.__init__(self)

   def write(self, payload):
      return self._transport.write(payload)

   def read(self, length, timeout=0):
      if timeout != self._transport.timeout:
         self._transport.timeout = timeout
      return self._transport.read(length)


class TestLightPlayer(LightPlayer):

   def __init__(self):
      self._read_queue = Queue()
      self._responses = {"on": LightPlayer.ON, "off": LightPlayer.OFF}
      LightPlayer.__init__(self)

   def write(self, payload):
      channel = ord(payload) >> 4
      op_code = ord(payload) % (1 << 4)
      print "channel", channel, "got message", op_code
      print
      if op_code == LightPlayer.QUERY:
         response = raw_input("response to query on channel {0}> ".format(channel))
         if response:
            self._read_queue.put(self._responses[response])

   def read(self, length, timeout=0):
      payload = ""
      try:
         while length > 0:
            payload += chr(int(self._read_queue.get()))
            length -= 1
      finally:
         return payload


class WifiLightPlayer(LightPlayer):
   pass
   #specify correct transport, expose proper options in __init__
   #def __init__(self, ip, port):
   #   pass

   #SEE!  now you can do all the TCP partial read/ partial write handling in the transport specific read/write methods which are, as of yet, not implemented
