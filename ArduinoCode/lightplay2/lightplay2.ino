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


int motorspeed = 255;
boolean motor_is_on = false; // 
boolean thisway = true;  // direction state

// color code: 0 = red, 1 = orange, 2 = yellow, 3 = green, 4 = blue, 5 = violet, 6 = white, 7 = surprise

int RGBWtable[28] = {4095,0,0,0,2500,1500,0,0,1500,2500,0,0,0,4095,0,0,0,0,4095,0,2000,0,3000,0,0,0,0,4095};
// this table contains the RGBW values for colors 0-6, in order.

int light1color = 0; // set current color of light 1 to white
int light2color = 0; // set current color of light 2 to white
int light3color = 0; // set current color of light 3 to white

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


// setup led_drivercalled this way, it uses the default address 0x40:
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

void loop()
  { 
    if (Serial.available() > 0)  // when a serial command comes in from Scratch, dispatch it
      {
        incomingByte = Serial.read();
        dispatch(incomingByte);
        digitalWrite(13, HIGH);
        delay(20); // set timing of single event loop; delay also eliminates effects of sensor bounce
        digitalWrite(13, LOW);
      }
      
     // threshold = analogRead(1); // dynamically set threshold
    check_for_sensor_edge(); //check for sensor edge
    update_fades();
   
    delay(20); // set timing of single event loop; delay also eliminates effects of sensor bounce
  }

void dispatch(byte incomingByte)
  {
    command = (incomingByte & 0xe0) >> 5; // bitwise + shift  selects bits 5-7, which are used to store high level command
    xbits = (incomingByte & 0x18) >> 3; // bits 3 and 4 used to select which light
    ybits = incomingByte & 0x07; // bits 0-2 select which subcommand or which color 
    // Serial.println(command);
    // Serial.println(xbits);
    // Serial.println(ybits);
    switch(command)
      {
        // Serial.println(command);
        case 0:
          // Serial.println("command 0 one argument light commands");
          // one argument light commands
          switch(ybits)
            {
              case 0:
                // Serial.println("ybits 0 - turn on light(s)");
                // turn on light(s)
                lighton();
                break;
      
              case 1:
                // Serial.println("ybits 1 - turn off light(s)");
                // turn off light(s)
                lightoff();
                break;

              case 2:
                // Serial.println("ybits 2 - fade in light(s)");
                // fade in light(s)
                fadein();
                break;

              case 3:
                // Serial.println("ybits 3 - fade out light(s)");
                // fade in light(s)
                fadeout();
                break;
      
              case 4:
                // Serial.println("ybits 4 - toggle light(s)");
                // toggle light(s)
                lighttoggle();
                break;

              case 5:
                // set brightness low
                // not yet implemented
                break;

              case 6:
                // set brightness medium
                // not yet implemented                
                break;

              case 7:
                // set brightness high
                // not yet implemented
                break;
                
            }
          break;
        case 1:
          // Serial.println("command 1 - set color to");
          setlightcolor();
          break;
        case 2:
          // Serial.println("command 2 - fade color to");
          break;
        case 3:
          // Serial.println("command 3 - other/extension light commands");
          switch(ybits)
            {
              case 0:
                switch(xbits)
                  {
                    case 0:
                      tfade = 10000;
                      break;

                    case 1:
                      tfade = 5000;
                      break;

                    case 2:
                      tfade = 2500;
                      break;
                  }
                break;
            }
            
        case 4:
          // Serial.println("command 4 - motor commands");
          switch(ybits)
            {
              case 0:
                // Serial.println("ybits 0 - turn motor on");
                // turn on motor
                motoron();
                break;

              case 1:
                // Serial.println("ybits 1 - turn motor off");
                // turn motor off
                motoroff();
                break;

              case 2:
                // Serial.println("ybits 2 - reverse motor");
                // reverse motor
                reversemotor();
                break;

              case 3:
                // Serial.println("ybits 3 - toggle motor");
                // toggle motor
                togglemotor();
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
            } 
          break;
      }
  }

void lighton()
  {
    // Serial.println("turn on light");
    if ((xbits == 1) || (xbits == 0)) 
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(i, 0, RGBWtable[4 * light1color + i]);}
        light1_is_on = true;
      }
       
    if ((xbits == 2) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(4 + i, 0, RGBWtable[4 * light2color + i]);}
        light2_is_on = true;
      }
            
    if ((xbits == 3) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(8 + i, 0, RGBWtable[4 * light3color + i]);}
        light3_is_on = true;
      }      
  }

void lightoff()
  {
    // Serial.println("turn off light");
    if ((xbits == 1) || (xbits == 0)) 
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(i, 0, 0);}
        light1_is_on = false;
      }
       
    if ((xbits == 2) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(4 + i, 0, 0);}
        light2_is_on = false;
      }
            
    if ((xbits == 3) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(8 + i, 0, 0);}
        light3_is_on = false;
      }      
  }

void lighttoggle()
  {
    if ((xbits == 1) || (xbits == 0))
      {
      for (int i = 0; i <= 3; i++)
        {
          if (!light1_is_on)
            {pwm.setPWM(i, 0, RGBWtable[4 * light1color + i]);}
          else
            {pwm.setPWM(i, 0, 0);}
        }
      light1_is_on = !light1_is_on;
      }
    
    if ((xbits == 2) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {
            if (!light2_is_on)
              {pwm.setPWM(4 + i, 0, RGBWtable[4 * light1color + i]);}
            else
              {pwm.setPWM(4 + i, 0, 0);}
          }
        light2_is_on = !light2_is_on;
      }
  
    if ((xbits == 3) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {
            if (!light2_is_on)
              {pwm.setPWM(8 + i, 0, RGBWtable[4 * light1color + i]);}
            else
              {pwm.setPWM(8 + i, 0, 0);}
          }
        light3_is_on = !light3_is_on;
      }
    
  }

  void setlightcolor() // set color to, xbits contains which light, ybits contains which color
    {
      if (ybits == 7) {ybits = random(7);}

      if ((xbits == 1) || (xbits == 0))
        {
          light1color = ybits;
          for (int i = 0; i <= 3; i++)
            {pwm.setPWM(i, 0, RGBWtable[4 * light1color + i]);}
          light1_is_on = true;
        }

     if ((xbits == 2) || (xbits == 0))
       {
         light2color = ybits;
         for (int i = 0; i <= 3; i++)
           {pwm.setPWM(4 + i, 0, RGBWtable[4 * light2color + i]);}
         light2_is_on = true;
       }
      
     if ((xbits == 3) || (xbits == 0))
       {
         light3color = ybits;
         for (int i = 0; i <= 3; i++)
           {pwm.setPWM(8 + i, 0, RGBWtable[4 * light3color + i]);}
         light3_is_on = true;
       } 
      
  }

void fadein()
  {
    t1start = millis();
    light1_is_fading_in = true;
    light1_is_fading_out = false;
    light1_is_fading_to = false;
  }


void fadeout()
  {
    t1start = millis();
    light1_is_fading_out = true;
    light1_is_fading_in = false;
    light1_is_fading_to = false;
  }

void update_fades()
  {
    if (light1_is_fading_in)
      {
        t1 = millis() - t1start;
        ptr = int(tablesize * t1 / tfade);
        if (ptr <= tablesize - 1)
          {
            int x = fadetable[ptr];
          
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetable[ptr];
                int y = RGBWtable[4 * light1color + i];
                int z = map(x, 0, 4095, 0, y);
                pwm.setPWM(i, 0, z);
              }
            
          }
        else
          {
            light1_is_fading_in = false;
          }
   
      }

    if (light1_is_fading_out)
      {
        t1 = millis() - t1start;
        ptr = int(tablesize * t1 / tfade);
        ptr = tablesize - 1 - ptr;
        if (ptr >= 0)
          {
            int x = fadetable[ptr];
            
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetable[ptr];
                int y = RGBWtable[4 * light1color + i];
                int z = map(x, 0, 4095, 0, y);
                pwm.setPWM(i, 0, z);
              }
          }
        else
          {
            light1_is_fading_out = false;
          }
   
      }
  }


void check_for_sensor_edge() // check for sensor edge 
  {
    newsensor = (analogRead(0) < threshold);
    if ((newsensor != oldsensor) && (newsensor == true))
      {
        Serial.write(0);
        Serial.write(2);
        digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(10);              // wait for a second
        digitalWrite(13, LOW); 
      }
    if ((newsensor != oldsensor) && (newsensor == false))
      {
        Serial.write(1);
        Serial.write(3);
      }
    oldsensor = newsensor;
  }

void motoron()
  {
    if (thisway)
      {
        digitalWrite(motora, HIGH);
        digitalWrite(motorb, LOW);
      }
   else
     {
       digitalWrite(motora, LOW);
       digitalWrite(motorb, HIGH);
     }
   motor_is_on = true;  
  }

void motoroff()
  {
    digitalWrite(motora, LOW);
    digitalWrite(motorb, LOW);
    motor_is_on = false; 
  }

void reversemotor()
  {
    digitalWrite(motora, !digitalRead(motora));
    digitalWrite(motorb, !digitalRead(motorb));
    thisway = !thisway;
  }

void togglemotor()
  {
    if (motor_is_on)
      {
        digitalWrite(motora, LOW);
        digitalWrite(motorb, LOW);
      }
    else
      {
        if (thisway)
          {
            digitalWrite(motora, HIGH);
            digitalWrite(motorb, LOW);
          }
       else
         {
            digitalWrite(motora, LOW);
            digitalWrite(motorb, HIGH);
         }
       }
     motor_is_on = !motor_is_on;
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

void fadetable_init()
  {
    for (int i = 0; i < tablesize; i++)
      { float x = float (i) * 12 / tablesize;
        x = pow(2,x);
        fadetable[i] = int(x);
        // Serial.println(fadetable[i]);
      }
  }



