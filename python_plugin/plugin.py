#! /usr/bin/env python
import threading
import sensor
import time
from arduino import TestLightPlayer
from SocketServer import ThreadingMixIn
from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
from urlparse import parse_qs

def run_test_arduino():
   arduino = TestLightPlayer()
   arduino.addSensor(sensor.ArduinoEdgeTriggeredSensor, 8)
   return arduino

def run_sensors():
   print "ctrl-c at any time to exit"
   print "background threads are not daemonized, and will finish execution before program exit"
   a = sensor.PromptingEdgeTriggeredSensor("sensor a")
   b = sensor.PromptingEdgeTriggeredSensor("sensor b")
   sensors = [a, b]
   #register behaviors here.
   sensors[0].register(test_function, "hello", msg2="world")
   sensors[1].register(test_function, "foo", "bar")
   #poll forever
   while True:
      for s in sensors:
         #holy polymorphism batman!  In this example, I happen to have 2 sensors of the same type, but since all sensors define register and poll,
         #The list of sensors could have sensors of any type and it would still work!
         s.poll()

def test_function(msg1, arduino,  msg2):
   print msg1
   #prove to me we're on a separate thread by not blocking the main thread while sleeping
   #arduino.reverse(9)
   time.sleep(2)
   arduino.reverse(9)
   print msg2

class ArduinoHTTPRequestHandler(BaseHTTPRequestHandler):

   def __init__(self, *args, **kwargs):
      self._event = threading.Event()
      BaseHTTPRequestHandler.__init__(self, *args, **kwargs)

   def do_GET(self):
      print "HELLO SERVER!"
      self.send_response(200)
      self.send_header('Access-Control-Allow-Origin', '*')
      self.send_header('Content-type', 'text/html')
      self.end_headers()
      query=parse_qs(self.path.strip("/?"))
      action = self.server.actions[query["action"][0]]
      channel = int(query["channel"][0])
      if query["action"][0] == "Register":
         action(channel, self._event.set)
         self._event.wait()
         self._event.clear()
      else:
         action(channel)


class ArduinoHTTPServer(HTTPServer, ThreadingMixIn):

   def __init__(self, arduino, *args, **kwargs):
      self._arduino = arduino
      self.actions = {"On": arduino.on, "Off": arduino.off, "Register": arduino.register}
      HTTPServer.__init__(self, *args, **kwargs)
 

if __name__ == "__main__":
#   run_sensors()
    arduino = run_test_arduino()
    server = ArduinoHTTPServer(arduino, ("localhost", 8000), ArduinoHTTPRequestHandler)
    t = threading.Thread(target=server.serve_forever)
    t.daemon = True
    t.start()
    arduino.poll()
