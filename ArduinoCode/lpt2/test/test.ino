#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

byte incomingByte = 0;  // for incoming serial data
byte command =0; //3 MSBs of incomingByte used to store high level command
byte xbits = 0; //two middle bits used for which light, etc,
byte ybits = 0; //3 LSBs used for which color or subommand

// arduino pins
int motora = 4;
int motorb = 7;
int motore = 3;
int leddriver_enable = 2;


int motorspeed = 100;
boolean motor_is_on = false; // 
boolean thisway = true;  // direction state

// color code: 0 = red, 1 = orange, 2 = yellow, 3 = green, 4 = blue, 5 = violet, 6 = white, 7 = surprise

int RGBWtable[28] = {4095,0,0,0,2500,1500,0,0,1500,2500,0,0,0,4095,0,0,0,0,4095,0,2000,0,3000,0,0,0,0,4095};
// this table contains the RGBW values for colors 0-6, in order.

int light1color = 6; // set current color of light 1 to white
int light2color = 6; // set current color of light 2 to white
int light3color = 6; // set current color of light 3 to white

 // for storing the light_is_on? states  of the three lights
boolean light1_is_on = false; //
boolean light2_is_on = false; //
boolean light3_is_on = false; //
boolean light1_is_fading_in = false; //
boolean light2_is_fading_in = false; //
boolean light3_is_fading_in = false; //
boolean light1_is_fading_out = false; //
boolean light2_is_fading_out = false; //
boolean light3_is_fading_out = false; //
boolean light1_is_fading_to = false; //
boolean light2_is_fading_to = false; //
boolean light3_is_fading_to = false; //



// for edge detection
boolean oldsensor = false;
boolean newsensor = false;
int threshold = 512;

const int tablesize = 200;
int fadetable[tablesize];

unsigned long t1start = 0;
unsigned long t1 = 0;
unsigned long t2start = 0;
unsigned long t2 = 0;
unsigned long t3start = 0;
unsigned long t3 = 0;
unsigned long tfade = 5000;
int ptr = 0;


// setup led_drivercalled this way, it uses the default address 0x40
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x70);
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);


void setup()
  {
    // initially set L293 inputs LOW, but with chip enabled
    pinMode(13, OUTPUT);
    pinMode(motora, OUTPUT);
    pinMode(motorb, OUTPUT);
    pinMode(motore, OUTPUT);

    pinMode(A0, INPUT_PULLUP); // sensor pin 
    pinMode(A1, INPUT_PULLUP); // for dynamic threshold
    pinMode(A2, INPUT); // pin A2 is unconnected, so random # generator should differ each time you run sketch
    randomSeed(analogRead(A2));
    
    // turn off motor initially
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
    // but 200 kHz is fast enough. With 1 k pullups on a breadboard, the clock signal looks ok on scope, no need to go faster though
    bootflash(); // flash all the lights
  }

void loop()
  { 
    pwm.setPWM(0, 0, 2500);
    delay(100); // set timing of single event loop; delay also eliminates effects of sensor bounce
  }

void bootflash()
  {
    for (int i = 0; i <= 11; i++)
      {
        pwm.setPWM(i, 0, 4095);
        delay(100); // bootflash
        pwm.setPWM(i, 0, 0);
      }
  }





