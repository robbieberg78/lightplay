/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  PWM test - this will drive 16 PWMs in a 'wave'
n
 ****************************************************/

 

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
byte incomingByte = 0;  // for incoming serial data

// pins
int motora = 4;
int motorb = 7;
int motore = 3;
int leddriver_enable = 2;


int motorspeed = 100;
boolean motoron = false; // 
boolean thisway = true;  // 

int color1 = 6; // set current color of light 1 to white
int color2 = 6; // set current color of light 2 to white
int color3 = 6; // set current color of light 3 to white
boolean lighton1 = false; //
boolean lighton2 = false; //
boolean lighton3 = false; // 

int i; // variable used as counter

int RGBWtable[28] = {4095,0,0,0,2500,1500,0,0,1500,2500,0,0,0,4095,0,0,0,0,4095,0,2000,0,3000,0,0,0,0,4095};

// for edge detection
boolean oldsensor = false;
boolean newsensor = false;
int threshold = 512;

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

void setup() {
  byte incomingByte = 0;  // for incoming serial data
  pinMode(13, OUTPUT);
  
  pinMode(motora, OUTPUT);
  pinMode(motorb, OUTPUT);
  pinMode(motore, OUTPUT);
    // initially set L293 inputs LOW, but with chip enabled
  digitalWrite(motora, LOW);
  digitalWrite(motorb, LOW);
  analogWrite(motore, motorspeed);
  pinMode(leddriver_enable, OUTPUT);
  digitalWrite(leddriver_enable, LOW); // enable LED driver

  oldsensor = analogRead(0);
 

  
  Serial.begin(9600);

  pwm.begin();
  pwm.setPWMFreq(1600);  // This is the maximum PWM frequency
    
  // save I2C bitrate
  uint8_t twbrbackup = TWBR;
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; 
  // TWBR = 12  causes the 8 MHz Mini Pro i2 to have a ~ 200 KHz clock speed
  // that's probably fast enough. The Atmega128 formula for clock speed is
  // clockfreq = 8 Mhz / (16 + 2*(TWBR)*(prescale value)
  // so setting TWBR to 2 would raise clock speed to max allowable of 400 kHz
  // but 200 kHz is fast enough. With 1 k pullups on a breadboard, the clock signal looks ok on scope, no need t go faster though
    
}

void loop() {

//      pwm.setPWM syntax is: 
//        pwm.setPWM(lednum, ton, toff)
//      where lednum is 0-15, and ton and toff are 0-4095
//      Our standard use is to set ton to zero, so that output goes HIGH at start of 12 bit count
//      and turns off at the count given by toff.
//      The duty cycle is then = toff/4095 * 100%
// 
//  }
 if (Serial.available() > 0) {
    // if there is an incoming serial byte from Python use switch command to dispatch it

    incomingByte = Serial.read();

    // extract three MSBs, which contains the high level command
    byte command = (incomingByte & 0xe0) >> 5; // bitwise and selects  3 MSBs and shift right 5 places
    byte xbits = (incomingByte & 0x18) >> 3; // bitwise and selects bits 3 and 4 shift right 3 places
    byte ybits = incomingByte & 0x07; // bitwise and selects 3 LSBs
    

    // extract channel number from high nibble of byte
    byte channel = (incomingByte & 0xf0) >> 4; //  bitwise and selects upper 4 bits, then  shift right by 4 bits


    //  use switch case to select which command to execute

    switch (command)  {
      case 0: // one argument light commands, xbits contains which light, ybits contains which command
        switch (ybits)  {
          case 0: // turn on lights
          
            if ((xbits == 1) || (xbits == 0)) {
            for (i = 0; i <= 3; i++)  {
               pwm.setPWM(i, 0, RGBWtable[4 * color1 + i]);
            }
            lighton1 = true;
          }
     
         if ((xbits == 2) || (xbits == 0)) {
            for (i = 0; i <= 3; i++)  {
               pwm.setPWM(4 + i, 0, RGBWtable[4 * color2 + i]);
            }
          lighton2 = true;
          }
          
         if ((xbits == 3) || (xbits == 0)) {
            for (i = 0; i <= 3; i++)  {
               pwm.setPWM(8 + i, 0, RGBWtable[4 * color3 + i]);
            }
          lighton3 = true;
          }      

            break; // end turn on lights
            
          case 1: // turn off lights
            if ((xbits == 1) || (xbits == 0)) {
                  for (i = 0; i <= 3; i++)  {
                     pwm.setPWM(i, 0, 0);
                  }
                lighton1 = false;
                }
                
                if ((xbits == 2) || (xbits == 0)) {
                  for (i = 4; i <= 7; i++)  {
                     pwm.setPWM(i, 0, 0);
                  }
                lighton2 = false;
                }
                
                if ((xbits == 3) || (xbits == 0)) {
                  for (i = 8; i <= 11; i++)  {
                     pwm.setPWM(i, 0, 0);
                  }
                lighton3 = false;
                }
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
            break;
            
          case 2:  // fade in lights

            break;

           case 3:  // fade out lights

            break;

           case 4: // toggle lights
              if ((xbits == 1) || (xbits == 0)) {
                for (i = 0; i <= 3; i++)  {
                    if (!lighton3) {
                      pwm.setPWM(i, 0, RGBWtable[4 * color1 + i]);
                      }
                    else  {
                       pwm.setPWM(i, 0, 0);
                      }
                   }
                  lighton1 = !lighton1;
                }
              

              if ((xbits == 2) || (xbits == 0)) {
                for (i = 0; i <= 3; i++)  {
                    if (!lighton2) {
                      pwm.setPWM(4 + i, 0, RGBWtable[4 * color2 + i]);
                      }
                    else  {
                       pwm.setPWM(4 + i, 0, 0);
                      }
                   }
                  lighton2 = !lighton2;
                }

              if ((xbits == 3) || (xbits == 0)) {
                for (i = 0; i <= 3; i++)  {
                    if (!lighton3) {
                      pwm.setPWM(8 + i, 0, RGBWtable[4 * color3 + i]);
                      }
                    else  {
                       pwm.setPWM(8 + i, 0, 0);
                      }
                   }
                  lighton3 = !lighton3;
                }
          
 

            break;
          

                
          case 5: // set brightness low

            break;
            
                       
          case 6: // set brightness medium

            break;
                         
          case 7: // set brightness high

            break;
          
            
        } // end of switch for one argument light commands
      
       
  case 1: // set color to, xbits contains which light, ybits contains which color
    if (ybits == 7) {ybits = random(7);}

     if ((xbits == 1) || (xbits == 0)) {
      color1 = ybits;
      for (i = 0; i <= 3; i++)  {
         pwm.setPWM(i, 0, RGBWtable[4 * color1 + i]);
      }
      lighton1 = true;
    }

   if ((xbits == 2) || (xbits == 0)) {
      color2 = ybits;
      for (i = 0; i <= 3; i++)  {
         pwm.setPWM(4 + i, 0, RGBWtable[4 * color2 + i]);
      }
    lighton2 = true;
    }
    
   if ((xbits == 3) || (xbits == 0)) {
      color3 = ybits;
      for (i = 0; i <= 3; i++)  {
         pwm.setPWM(8 + i, 0, RGBWtable[4 * color3 + i]);
      }
    lighton3 = true;
    } 

    break;
   
  case 2: // fade color to, xbits contains which light, ybits contains which color
    //if (ybits == 7) {ybits = random(7);}

    break;
   
  case 3: // other commands such as set fade speed 
  
     break;

                  
  case 4: // motor commands
    switch(ybits) {
      case 0: // turn motor on
        if (thisway)  {
          digitalWrite(motora, HIGH);
          digitalWrite(motorb, LOW);
        }
        else  {
          digitalWrite(motora, LOW);
          digitalWrite(motorb, HIGH);
        }
        motoron = true;
        break;

        case 1: // turn motor off
          digitalWrite(motora, LOW);
          digitalWrite(motorb, LOW);
          motoron = false;
        break;

        case 2: // reverse motor 
          digitalWrite(motora, !digitalRead(motora));
          digitalWrite(motorb, !digitalRead(motorb));
          thisway = !thisway;
        break;


       case 3: // toggle motor
          if (motoron)  {
            digitalWrite(motora, LOW);
            digitalWrite(motorb, LOW);
          }
          else {
            if (thisway)  {
              digitalWrite(motora, HIGH);
              digitalWrite(motorb, LOW);
            }
           else  {
            digitalWrite(motora, LOW);
            digitalWrite(motorb, HIGH);
            }
          }
          motoron = !motoron;
        break;

        case 4: //set motorspeed slow
          motorspeed = 75;
          analogWrite(motore, motorspeed);
        break;

        case 5: //set motorspeed faster
          motorspeed = 150;
          analogWrite(motore, motorspeed);
        break;

        case 6: //set motorspeed fastest
          motorspeed = 255;
          analogWrite(motore, motorspeed);
        break;

        
    } // end of motor commands switch
     
      
          break;
   }  // end of ubits command switch

}  // end of serial command processing
  //edge detection
  // threshold = analogRead(1); // dynamically set threshold
//  newsensor = (analogRead(0) < threshold);
//  // ifelse newsensor turn indicator light red/green
//  
//  if ((newsensor != oldsensor) && (newsensor == true))
//  { Serial.write(0);
//  }
//  if ((newsensor != oldsensor) && (newsensor == false))
//  { Serial.write(1);
//  }
//  oldsensor = newsensor;

delay(25); // set timing of single event loop; also eliminates effects of switch bounce
} // end of main loop
