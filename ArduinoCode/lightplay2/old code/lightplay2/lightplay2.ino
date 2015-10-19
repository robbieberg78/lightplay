/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  PWM test - this will drive 16 PWMs in a 'wave'
n
 ****************************************************/

 

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
byte incomingByte = 0;  // for incoming serial data
int motora = 4;
int motorb = 7;
int motore = 3;
int leddriver_enable = 2;

// for edge detection
int oldsensor0 = 0;
int newsensor0 = 0;
int oldsensor1 = 0;
int newsensor1 = 0;

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

void setup() {
  byte incomingByte = 0;  // for incoming serial data
  pinMode(motora, OUTPUT);
  pinMode(motorb, OUTPUT);
  pinMode(motore, OUTPUT);
    // initially set L293 inputs LOW, but with chip enabled
  digitalWrite(motora, LOW);
  digitalWrite(motorb, LOW);
  digitalWrite(motore, HIGH);
  pinMode(leddriver_enable, OUTPUT);
  digitalWrite(leddriver_enable, LOW); // enable LED driver

  oldsensor0 = digitalRead(A0);
  oldsensor1 = digitalRead(A1);

  
  Serial.begin(9600);

  pwm.begin();
  pwm.setPWMFreq(1600);  // This is the maximum PWM frequency
    
  // save I2C bitrate
  uint8_t twbrbackup = TWBR;
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; 
  // TWBR = 12  causes the 8 MHz Mini Pro i2 to have a ~ 200 KHz clock speed
  // that's probably fast enough. The Atmega128 formular for clock speed is
  // 8 Mhz / (16 + 2*(TWBR)*(prescale value)
  // so setting TWBR to 2 would raise clock speed to max allowable of 400 kHz
  // but 200 kHz is fast enough. ith 1 k pullups on breadboard clock signal looks ok on scope
    
}

void loop() {
  // 
//  for (uint16_t i=0; i<4096; i += 1) {
//    
//      pwm.setPWM(0, 0, i );
//      // syntax is: (lednum, ton, toff)
//      // where lednum is 0-15, and ton and toff are 0-4095
//      // standard use is to set ton to zero, so that output goes HIGH at start of 12 bit count
//      // duty cycle is then = toff/4095 * 100%
// 
//  }
 if (Serial.available() > 0) {
    // if there is an incoming serial byte from Python use switch command to dispatch it

    incomingByte = Serial.read();

    // extract low nibble, which cotains the command
    byte command = incomingByte & 0x0f; // bitwise and selects lower 4 bits
    // command = 0 => turn LED off
    // command = 1 => turn LED on
    // comand = 2 => toggle state of LED

    // extract channel number from high nibble of byte
    byte channel = (incomingByte & 0xf0) >> 4; //  bitwise and selects upper 4 bits, then  shift right by 4 bits


    //  use switch case to select which command to execute

    switch (command)  {
      case 0: // turn all leds off
      for (uint8_t i=0; i<=15; i += 1) {
          pwm.setPWM(i, 0, 0 );
      }
          break;
      case 1: // turn all leds on max brightness
      for (uint8_t i=0; i<=15; i += 1) {
          pwm.setPWM(i, 0, 4095 );
      }
          break;
      case 2: // turn off led channel 
          pwm.setPWM(channel, 0, 0 );
          break;
      case 3: // turn on led channel 
          pwm.setPWM(channel, 0, 4095 );
          break;
   }  // end of switch

}  // end of serial command processing
delay(25); // set timing of single event loop; also eliminates effects of switch bounce
} // end of main loop
