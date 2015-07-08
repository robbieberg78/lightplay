
import serial
import time
# to pair Spark Bluesmirf: from Mac Bluetooth menu select open bluetooth prefs
# remove FireFly-2713
# wait
# when it appears, attempt to pair, it will initially fail
# then selct option, using code 1234 repair, it should suceed
# s.write(chr(1)) will transmit a binary 1 at 115200 baud, etc
s = serial.Serial(port='/dev/cu.FireFly-2713-SPP', baudrate=115200, timeout = 0.01)

##while True:
##        s.write(chr(0x18))
##        time.sleep(1.0)
