#!/usr/bin/env python
import select
import argparse
import threading
import sensor
import glob
import time
import traceback

from arduino import SerialLightPlayer, SerialBtLight, MidiDevice
from SocketServer import ThreadingMixIn
from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
from urlparse import urlparse, parse_qs


def run_test_arduino(args):
   arduino = SerialLightPlayer(args.serial_port, args.baudrate)
   devices = [arduino]
   arduino.addSensor(sensor.ArduinoEdgeTriggeredSensor, 0)
   arduino.addSensor(sensor.ArduinoEdgeTriggeredSensor, 1)
   actions = {
       "On": arduino.on,
       "Off": arduino.off,
       "Toggle": arduino.toggle,
       "Rev": arduino.reverse,
       "Register": arduino.register_and_wait_for_state,
       # add colors
       "FadeIn": arduino.fade_in,
       "FadeOut": arduino.fade_out,
       #       "Poll": arduino.poll,
       "Low": arduino.low,
       "Med": arduino.med,
       "High": arduino.high,
       "FadeSlow": arduino.fade_slow,
       "FadeFaster": arduino.fade_faster,
       "FadeFastest": arduino.fade_fastest,


       # need
       "Set": arduino.set,
       "FadeTo": arduino.fade_to
   }
   if args.bt_devices:
      bt_port = args.bt_devices[0]
      bt = SerialBtLight(bt_port, args.baudrate)
      bt_actions = {
          "Set": bt.set,
          "BtOff": bt.off,
          "FadeTo": bt.fade_to
      }
      devices.append(bt)
      actions.update(bt_actions)
   else:
      print "Warning:  No bluetooth devices were detected"
   if args.midi:
      # IAC Driver IAC Bus 1 is a good choice
      midi = MidiDevice(args.midi)
      midi.addSensor(sensor.MidiEventSensor, 66)

      midi_actions = {
          "Register": midi.register_and_wait
      }
      devices.append(midi)
      actions.update(midi_actions)

   if not devices:
      raise RuntimeError(
          "Plugin was unable to detect any devices, please check wired connections and the state of bluetooth devices")
   return actions, devices


class ArduinoHTTPRequestHandler(BaseHTTPRequestHandler):

   def __init__(self, *args, **kwargs):
      BaseHTTPRequestHandler.__init__(self, *args, **kwargs)

   def do_GET(self):
      self.send_response(200)
      self.send_header('Access-Control-Allow-Origin', '*')
      self.send_header('Content-type', 'text/html')
      self.end_headers()
      query = parse_qs(urlparse(self.path)[4])
      for field, value in query.items():
         if len(value) == 1:
            query[field] = value[0]
      print query
      action_name = query.pop("action")
      action = self.server.actions[action_name]
      try:
         result = action(**query)
         print "RESULT: ", result
         self.wfile.write(result)
      except Exception as e:
         traceback.print_exc()
         self.wfile.write("Error: {0}".format(e))


class ArduinoHTTPServer(ThreadingMixIn, HTTPServer):

   def __init__(self, actions, *args, **kwargs):
      self.actions = actions
      HTTPServer.__init__(self, *args, **kwargs)


if __name__ == "__main__":

   while True:
      parser = argparse.ArgumentParser()
      parser.add_argument('--port', '-p', type=int, default=8000)
      parser.add_argument('--ip', '-i', default='localhost')
      parser.add_argument('--serial-port', '-s', dest="serial_port")
      parser.add_argument(
          '--bt-device', nargs='*', default=[], dest='bt_devices')
      parser.add_argument('--baudrate', '-b', type=int, default=9600)
      parser.add_argument('--midi', '-m')
      args = parser.parse_args()
      if not args.serial_port:
         # look for valid serial devices, if more than one is available, choose
         # one but notify the user
         serial_list = glob.glob("/dev/tty.usbserial*")
         if serial_list:
            args.serial_port = serial_list[0]
            if len(serial_list) > 1:
               print "from available devices {0}, selected {1}.  Run with --serial-port to specify another device.".format(serial_list, args.serial_port)
      if not args.bt_devices:
         # look for valid serial devices, if more than one is available, choose
         # one but notify the user
         bt_list = glob.glob("/dev/*FireFly*")
         if bt_list:
            args.bt_devices = bt_list
      actions, devices = run_test_arduino(args)
      server = ArduinoHTTPServer(
          actions, (args.ip, args.port), ArduinoHTTPRequestHandler)
      t = threading.Thread(target=server.serve_forever)
      t.daemon = True
      t.start()
      while True:
         read_fds, write_fds, x_fds = select.select(devices, [], [], .1)
         if read_fds:
            for device in read_fds:
               device.update()

