#include <Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#define VERSION 0x22    // version 2.2
//(version number is X.Y format with X given by high nibble and Y given by low nibble)

byte incomingByte = 0;  // for incoming serial data.
byte command =0; //3 MSBs of incomingByte used to store high level command
byte xbits = 0; //two middle bits used for which light
byte ybits = 0; //3 LSBs used for which subommand

// arduino pins
int motora = 4;
int motorb = 7;
int motore = 12; //enable line on the L293D motor driver is run by LED12 line on PCA9685 pwm driver
int leddriver_enable = 2;

int motorspeed = 10;
boolean motora_alt = false; // remember if digital state has been temporarily altered for speed control
boolean motorb_alt = false; // remember if digital state has been temporarily altered for speed control

int pwmchan[] = {0,7,3,11}; // this maps which cluster of LED drover outputs go to which light  
byte packet[8]; // this is where the incoming RGBW bytes are buffered
int RGBWvals[4];
int newRGBWvals[4];
 
 typedef struct {
  int redval = 0;
  int greenval = 0;
  int blueval = 0;  
  int whiteval = 0;
  int newredval = 0;
  int newgreenval = 0;
  int newblueval = 0;  
  int newwhiteval = 0;
  int intredval = 0;
  int intgreenval = 0;
  int intblueval = 0;  
  int intwhiteval = 0;
  int power = 1; 
  boolean is_fading = false;
  boolean expfade = false; // true if you want to use an exponential fade table on a light (use when that light is fading out or in)
  unsigned long tstart = 0;
  unsigned long t;
} Light;

Light lights[4];

const int tablesize = 200;
int fadetable[tablesize];
int fadetotable[tablesize];


unsigned long tfade = 5000;
unsigned long tloop = 0; // to keep track of time since last sendsensor
int ptr = 0;

// setup led_drivercalled this way, it uses the default address 0x40:
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
// on v.0.2a board all user address lines are tied LOW, so correct address is 0x40
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
void setup() {
   // initially set L293 inputs LOW, but with chip enabled
    pinMode(13, OUTPUT);
    pinMode(motora, OUTPUT);
    pinMode(motorb, OUTPUT);
    pinMode(A0, INPUT_PULLUP); // sensor pin 
    pinMode(A1, INPUT_PULLUP); // for dynamic threshold
    pinMode(A2, INPUT); // pin A2 is unconnected, so random # generator should differ each time you run sketch
    randomSeed(analogRead(A2));
    pinMode(leddriver_enable, OUTPUT);
    digitalWrite(leddriver_enable, LOW); // enable LED driver
    tloop = millis();
    
    Serial.begin(9600);
    pwm.begin();
    pwm.setPWMFreq(1600);  // This is the maximum PWM frequency
     // turn off motor initially
    digitalWrite(motora, LOW);
    digitalWrite(motorb, LOW);
    pwm.setPWM(motore, 4096, 0); // this is the correct way to initialize motor duty cycle to 100%
  
    fadetable_init();
      
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

void loop() {
    if (Serial.available() > 0)  // when a serial command comes in from Scratch, dispatch it
      {
        incomingByte = Serial.read();
        dispatch(incomingByte);
        delay(5); // set timing of single event loop; delay also eliminates effects of sensor bounce
      }
    update_fades();
    if (motorspeed < 10)
      {
        if (digitalRead(motora))
          {
            motora_alt = true;
            digitalWrite(motora,LOW);
          }
        if (digitalRead(motorb))
          {
            motorb_alt = true;
            digitalWrite(motorb,LOW);
          }
        delay(10 - motorspeed);
        if (motora_alt)
          {
            motora_alt = false;
            digitalWrite(motora,HIGH);
          }
        if (motorb_alt)
          {
            motorb_alt = false;
            digitalWrite(motorb,HIGH);
          }
        delay(motorspeed);
       
      }

    else
      {
        delay(10);
      }
    if((millis() - tloop) > 50)  //stream sensors at 20 Hz
      { tloop = millis();
         sendsensor();
      }
}

void dispatch(byte incomingByte)
  {
    command = (incomingByte & 0xe0) >> 5; // bitwise + shift  selects bits 5-7, which are used to store high level command
    xbits = (incomingByte & 0x18) >> 3; // bits 3 and 4 used to select which light
    ybits = incomingByte & 0x07; // bits 0-2 select which subcommand or which color 
    if (command == 1) // motor commands
      {switch(ybits)
        {case 0:
          onthisway();
          break;
         case 1:
           onthatway();
           break;
         case 2:
           motoroff();
           break;
         case 3:
           setmotorspeed();
           break;       
        }
        
      }
      
    if (command == 2) // light commands
      {switch(ybits)
        {case 0:
          setlightcolor();
          break;
         case 1:
           lightoff();
           break;
         case 2:
           fadeto();
           break;
         case 3:
           fadeout();
           break;
         case 4:
           setbrightness();
           break;
         case 5:
           setfadespeed();
           break;
        }
      }

    if (command == 3) // other commands
      {switch(ybits)
        {case 0:
           resetstate();
           break;
         case 1:
           stopfades();
           break;
         case 2: // send firmware version number
           if (xbits == 0)
             {Serial.write(VERSION+170);} 
             //The response has to be in the 202-247 range because by the time the ipad sends the command the data is flowing from the sensors.       
           break;
         
          
        }

      }
  }

void getPacket()
  {
    while (Serial.available() < 8)
      {
      digitalWrite(13, LOW);   // nop
      }
    
    Serial.readBytes(packet,8);
  }

void onthisway()
  {
    digitalWrite(motora, HIGH);
    digitalWrite(motorb, LOW);
  }

void onthatway()
  {
    digitalWrite(motora, LOW);
    digitalWrite(motorb, HIGH);    
  }

void motoroff()
  {
    digitalWrite(motora, LOW);
    digitalWrite(motorb, LOW);    
  }

void setmotorspeed()
  {
   while (Serial.available() < 1)
    {
    digitalWrite(13, LOW);   // nop
    }
     
  motorspeed = Serial.read();    
  }

void setlightcolor(){
  getPacket();
  for(int l=1;l<4;l++){
    if ((xbits==l)||(xbits==0)) {
      if (lights[l].is_fading)
        {lights[l].is_fading = false;
        byte fadedone = 248 + l;
        Serial.write(fadedone);}
      lights[l].redval = 256 * packet[0] + packet[1];
      lights[l].greenval = 256 * packet[2] + packet[3];
      lights[l].blueval = 256 * packet[4] + packet[5];
      lights[l].whiteval = 256 * packet[6] + packet[7];
      lights[l].intredval = lights[l].redval;
      lights[l].intgreenval = lights[l].greenval;
      lights[l].intblueval = lights[l].blueval;
      lights[l].intwhiteval = lights[l].whiteval;            
      RGBWvals[0] = lights[l].redval;
      RGBWvals[1] = lights[l].greenval;
      RGBWvals[2] = lights[l].blueval;
      RGBWvals[3] = lights[l].whiteval;
       for (int i=0;i<=3;i++)pwm.setPWM(pwmchan[l]-i, 0, RGBWvals[i]/lights[l].power);
    }
  }
}

void lightoff(){
  for(int l=1;l<4;l++){
    if (lights[l].is_fading)
      {lights[l].is_fading = false;
      byte fadedone = 248 + l;
      Serial.write(fadedone);}
    if ((xbits == l) || (xbits == 0))
    {
      for (int i = 0; i <= 3; i++){pwm.setPWM(pwmchan[l]-i, 0, 0);}
      lights[l].redval = 0;
      lights[l].greenval = 0;
      lights[l].blueval = 0;
      lights[l].whiteval = 0;
      lights[l].intredval = 0;
      lights[l].intgreenval = 0;
      lights[l].intblueval = 0;
      lights[l].intwhiteval = 0;      
    }
  }
}

void fadeto(){
  getPacket();
  for(int l=1;l<4;l++){
    if ((xbits == l) || (xbits == 0)){
      if (lights[l].is_fading)
        {
         lights[l].redval = lights[l].intredval;
         lights[l].greenval = lights[l].intgreenval;
         lights[l].blueval = lights[l].intblueval;
         lights[l].whiteval = lights[l].intwhiteval;
         byte fadedone = 248 + l;
         Serial.write(fadedone);
         }
      lights[l].newredval = 256 * packet[0] + packet[1];
      lights[l].newgreenval = 256 * packet[2] + packet[3];
      lights[l].newblueval = 256 * packet[4] + packet[5];
      lights[l].newwhiteval = 256 * packet[6] + packet[7];  
      lights[l].tstart = millis();
      lights[l].is_fading = true;
      if ((lights[l].redval < 2000) && (lights[l].greenval < 2000) && (lights[l].blueval < 2000) && (lights[l].whiteval < 2000))
        {lights[l].expfade = true;}
      else
        {lights[l].expfade = false;}
    }
  }   
}

void fadeout(){
  for(int l=1;l<4;l++){
    if ((xbits == l) || (xbits == 0)){
      if (lights[l].is_fading)
        {
         lights[l].redval = lights[l].intredval;
         lights[l].greenval = lights[l].intgreenval;
         lights[l].blueval = lights[l].intblueval;
         lights[l].whiteval = lights[l].intwhiteval;
         byte fadedone = 248 + l;
         Serial.write(fadedone);
         }      
      lights[l].newredval = 0;
      lights[l].newgreenval = 0;
      lights[l].newblueval = 0;
      lights[l].newwhiteval = 0;  
      lights[l].tstart = millis();
      lights[l].is_fading = true;
      lights[l].expfade = true;
    }
  }  
}

void setbrightness(){
    int x;
   while (Serial.available() < 1)
    {
    digitalWrite(13, LOW);   // nop
    }
  x = Serial.read();   // capture the power level
  for(int l=1;l<4;l++){
    if ((xbits == l) || (xbits == 0)) {
      lights[l].power = x;
      RGBWvals[0] = lights[l].redval;
      RGBWvals[1] = lights[l].greenval;
      RGBWvals[2] = lights[l].blueval;
      RGBWvals[3] = lights[l].whiteval;
       for (int i=0;i<=3;i++)pwm.setPWM(pwmchan[l]-i, 0, RGBWvals[i]/lights[l].power);
    }
  }
}

void setfadespeed(){
   while (Serial.available() < 1)
    {
    digitalWrite(13, LOW);   // nop
    }
 
  int x = Serial.read(); // x stores fade speed in seconds
  tfade = x * 1000; 
}

void resetstate(){
  for(int l=1;l<4;l++){lights[l].power = 1;}
  motorspeed = 10;
  tfade = 1000;  
}

void stopfades(){
  for(int l=1;l<4;l++){
     if ((xbits == l) || (xbits == 0)) {
        lights[l].is_fading = false;
     }
   } 
}

void update_fades()
  { int x=1;
    int ystart;
    int ystop;
    int z;
    int temp;
    for(int l=1;l<4;l++){
            if (lights[l].is_fading)
          {
             RGBWvals[0] = lights[l].redval;
             RGBWvals[1] = lights[l].greenval;
             RGBWvals[2] = lights[l].blueval;
             RGBWvals[3] = lights[l].whiteval;
             newRGBWvals[0] = lights[l].newredval;
             newRGBWvals[1] = lights[l].newgreenval;
             newRGBWvals[2] = lights[l].newblueval;
             newRGBWvals[3] = lights[l].newwhiteval;
            lights[l].t = millis() - lights[l].tstart;
            ptr = int(tablesize * lights[l].t / tfade);
            if (ptr <= tablesize - 1)
              {
                for (int i = 0; i <= 3; i++)
                  {
                    ystart = RGBWvals[i];
                    ystop = newRGBWvals[i];
                    if (abs(ystop - ystart) > 10 )
                      {if (lights[l].expfade)
                          {if (ystart < ystop)
                             {x = fadetable[ptr];}
                           else
                             {
                              x = fadetable[tablesize - 1- ptr];
                              temp = ystart;
                              ystart = ystop;
                              ystop = temp;
                             }         
                          }
                        else
                          {x = fadetotable[ptr];}
                        z = (map(x, 0, 4095, ystart, ystop))/lights[l].power;
                        switch(i)
                          {
                            case 0:
                              lights[l].intredval = z;
                              break;
                            case 1:
                              lights[l].intgreenval = z;
                              break;
                            case 2:
                              lights[l].intblueval = z;
                              break;
                            case 3:
                              lights[l].intwhiteval = z;
                              break;
                          }
                        pwm.setPWM(pwmchan[l]-i, 0, z);
                      }
                  }         
              }
            else
              {
                lights[l].is_fading = false;
                for (int i = 0; i <= 3; i++)
                  {pwm.setPWM(pwmchan[l]-i, 0, newRGBWvals[i]/lights[l].power);}  
              lights[l].redval = lights[l].newredval;
              lights[l].greenval = lights[l].newgreenval;
              lights[l].blueval = lights[l].newblueval;
              lights[l].whiteval = lights[l].newwhiteval;
              // bytes 253,254,255 indicate fade done on channels 1,2,3
              byte fadedone = 252 + l;
              Serial.write(fadedone);           
              }
          }
    }          
  }

void sendsensor() // map sensor values to a range of 100 
  {
    int x=analogRead(0);
    x = map(x,0,1023,0,100);  // when iPad receives a value from 0 to 100, that's sensor0 value 
    Serial.write(x);
    x=analogRead(1);
    x=map(x,0,1023,101,201); // when iPad receives a value from 101 to 201, subract 101 to get sensor1 value 
    Serial.write(x);      
  }

void bootflash()
  {
    for (int i = 0; i <= 11; i++)
      {
        pwm.setPWM(i, 4096, 0); // 100% duty cycle 
        delay(50); // bootflash
        pwm.setPWM(i, 0, 0); //0% duty cycle 
      }
  }

void fadetable_init() // create a look-up table that grows exponentially from 1 to 4095
  {
    for (int i = 0; i < tablesize; i++)
      { float x = float (i) * 12 / tablesize;
        x = pow(2,x) + 2;
        fadetable[i] = int(x);
        if (fadetable[i] > 4095)
            {fadetable[i] = 4095;}
        fadetotable[i] = map(i,0,tablesize - 1, 0, 4095);
      }
    fadetable[0] = 0;
    fadetable[tablesize - 1] = 4095;
    fadetotable[tablesize - 1] = 4095;
  }
