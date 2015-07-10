
import serial
import time

# this test program lays out the basic protocol for communicating with the the multicolored lamp 
# the multicolor lamp is a self contained unit that communicates with ScratchX via an onboard Bluetooth
# to serial converter wired to an Arduino UART.

# if there is difficulty connecting to the Bluetooth modem try the following:
# from Mac Bluetooth menu select open bluetooth prefs
# remove FireFly-2713
# wait
# when the Bluetooth modemr eappears on the list , attempt to pair, it will (probably) initially fail
# then select option, using code 1234 reattempt paiing r, it should suceed
# s.write(chr(1)) will transmit a binary 1 at 115200 baud, etc

##### Protocol for serial communication. As before we'll use a single byte to send discreet action commands.
# (There's no sensing, so communication is one -way.)
# The low nibble of the byte will contain the command and the high nibble will contain the color information
# for now there are three commands (and three associated blocks):

# command 0: turn lamp off  (no color info needed)
# command 1: set lamp color to: (colors: 1 = white, 2 = blue, 3 = teal, 4 = green, 5 = yellow, 6 = orange, 7 = red, 8 = pink, 9 = purple, 10 = surprise)
# command 2: slowly change color to (color codes are same as above), for now let's make the transition time 3 seconds, so add that 3 second wait in as in fade blcks

s = serial.Serial(port='/dev/cu.FireFly-2713-SPP', baudrate=115200, timeout = 0.01)



while True:

    s.write(chr(0x11)) # turn on white
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0x21)) # turn on blue
    time.sleep(2.0)
    
    s.write(chr(0x31)) # turn on teal
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0x41)) # turn on green
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0x51)) # turn on yellow
    time.sleep(2.0)
    
    s.write(chr(0x61)) # turn on orange
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0x71)) # turn on red
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0x81)) # turn on pink
    time.sleep(2.0)
    
    s.write(chr(0x91)) # turn on purple
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0xa1)) # turn on random color
    time.sleep(2.0)
    
    s.write(chr(0)) # turn off
    time.sleep(0.5)

    s.write(chr(0x11)) # turn on white
    time.sleep(2.0)

    s.write(chr(0x12)) # slowly change to blue
    time.sleep(2.0) # stay blue for a bit

    s.write(chr(0x72)) # slowly change to red
    time.sleep(2.0) # stay red for a bit

   # etc..

    

    


        
