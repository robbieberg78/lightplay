#! /usr/bin/env python
import argparse
import threading
import sensor
import time
import glob
from arduino import TestLightPlayer, SerialLightPlayer
from SocketServer import ThreadingMixIn
from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
from urlparse import parse_qs
import time
import traceback


def run_test_arduino(args):
   #   arduino = TestLightPlayer()
   arduino = SerialLightPlayer(args.serial_port, args.baudrate)
   arduino.addSensor(sensor.ArduinoEdgeTriggeredSensor, 1)
   arduino.addSensor(sensor.ArduinoAnalogSensor, 2)
   return arduino


def run_sensors():
   print "ctrl-c at any time to exit"
   print "background threads are not daemonized, and will finish execution before program exit"
   a = sensor.PromptingEdgeTriggeredSensor("sensor a")
   b = sensor.PromptingEdgeTriggeredSensor("sensor b")
   sensors = [a, b]
   # register behaviors here.
   sensors[0].register(test_function, "hello", msg2="world")
   sensors[1].register(test_function, "foo", "bar")
   # poll forever
   while True:
      for s in sensors:
         # holy polymorphism batman!  In this example, I happen to have 2 sensors of the same type, but since all sensors define register and poll,
         # The list of sensors could have sensors of any type and it would
         # still work!
         s.poll()


def test_function(msg1, msg2):
   print msg1
   # prove to me we're on a separate thread by not blocking the main thread while sleeping
   # arduino.reverse(9)
   time.sleep(2)
   print msg2


class ArduinoHTTPRequestHandler(BaseHTTPRequestHandler):

   def __init__(self, *args, **kwargs):
      self._event = threading.Event()
      BaseHTTPRequestHandler.__init__(self, *args, **kwargs)

   def do_GET(self):
      self.send_response(200)
      self.send_header('Access-Control-Allow-Origin', '*')
      self.send_header('Content-type', 'text/html')
      self.end_headers()
      query = parse_qs(self.path.strip("/?"))
      action = self.server.actions[query["action"][0]]
      channel = int(query["channel"][0])
      try:
         if query["action"][0] == "Register":
            action(channel, self._event.set)
            self._event.wait()
            self._event.clear()
            result = str(self.server.sensor_value(channel))
         else:
            result = action(channel)
         print "RESULT: ", result
         self.wfile.write(result)
      except Exception as e:
         traceback.print_exc()
         self.wfile.write("Error: {0}".format(e))


class ArduinoHTTPServer(ThreadingMixIn, HTTPServer):

   def __init__(self, arduino, *args, **kwargs):
      self._arduino = arduino
      self.actions = {
          "On": arduino.on,
          "Off": arduino.off,
          "Rev": arduino.reverse,
          "Register": arduino.register,
          "Poll": arduino.poll,
          "Low": arduino.low,
          "Med": arduino.med,
          "High": arduino.high
      }
      HTTPServer.__init__(self, *args, **kwargs)

   def sensor_value(self, channel):
      return self._arduino.sensor_value(channel)

if __name__ == "__main__":
   #   run_sensors()
   parser = argparse.ArgumentParser()
   parser.add_argument('--port', '-p', type=int, default=8000)
   parser.add_argument('--ip', '-i', default='localhost')
   parser.add_argument('--serial-port', '-s', dest="serial_port")
   parser.add_argument('--baudrate', '-b', type=int, default=9600)
   args = parser.parse_args()
   if not args.serial_port:
      # look for valid serial devices, if more than one is available, choose
      # one but notify the user
      serial_list = glob.glob("/dev/tty.usbmodem*")
      if serial_list:
         args.serial_port = serial_list[0]
         if len(serial_list) > 1:
            print "from available devices {0}, selected {1}.  Run with --serial-port to specify another device.".format(serial_list, args.serial_port)
      else:
         raise RuntimeError(
             "Failed to detect connected serial devices.  If one is available, specify with --serial-port")

   arduino = run_test_arduino(args)
   server = ArduinoHTTPServer(
       arduino, (args.ip, args.port), ArduinoHTTPRequestHandler)
   t = threading.Thread(target=server.serve_forever)
   t.daemon = True
   t.start()
   while True:
      arduino.poll()
      time.sleep(0.1)

