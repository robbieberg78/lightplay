#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

void setlightcolor(bool transform=true);

byte incomingByte = 0;  // for incoming serial data
byte command =0; //3 MSBs of incomingByte used to store high level command
byte xbits = 0; //two middle bits used for which light, etc,
byte ybits = 0; //3 LSBs used for which color or subommand
byte rgb[3];


// arduino pins
int motora = 4;
int motorb = 7;
int motore = 12; //enable line on the L293D motor driver is run by LED12 line on PCA9685 pwm driver
int leddriver_enable = 2;


int motorspeed = 10;
boolean motor_is_on = false; // 
boolean thisway = true;  // direction state
boolean motora_alt = false; // remember if digital state has been temporarily altered for speed control
boolean motorb_alt = false; // remember if digital state has been temporarily altered for speed control

// color code: 0 = red, 1 = orange, 2 = yellow, 3 = green, 4 = blue, 5 = violet, 6 = white, 7 = surprise

int RGBWtable[28] = {4095,0,0,0,2800,1200,0,0,2100,1900,0,0,0,4095,0,0,0,0,4095,0,2000,0,3000,0,0,0,0,4095};
// this table contains the RGBW values for colors 0-6, in order.

int light1color = 6; // set current color of light 1 to white
int light2color = 6; // set current color of light 2 to white
int light3color = 6; // set current color of light 3 to white

int lastsurprise = 0;


int light1newcolor = 0; // target color for light 1 fade to
int light2newcolor = 0; // target color for light 2 fade to
int light3newcolor = 0; // target color for light 3 fade to

int light1power = 1; // power divisor for light 1
int light2power = 1; // power divisor for light 2
int light3power = 1; // power divisor for light 3

// for storing the light_is_on? states  of the three lights
boolean light1_is_on = false; //
boolean light2_is_on = false; //
boolean light3_is_on = false; //

// for storing the fade states  of the three lights
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
int fadetotable[tablesize];

unsigned long t1start = 0;
unsigned long t1 = 0;
unsigned long t2start = 0;
unsigned long t2 = 0;
unsigned long t3start = 0;
unsigned long t3 = 0;
unsigned long tfade = 1000;
int ptr = 0;


// setup led_drivercalled this way, it uses the default address 0x40:
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
// on v.0.2a board all user address lines are tied LOW, so correct address is 0x40
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);


void setup()
  {
    memset(rgb, 0, sizeof(rgb));
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
  
    oldsensor = analogRead(0);
    
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

void loop()
  { 
    if (Serial.available() > 0)  // when a serial command comes in from Scratch, dispatch it
      {
        incomingByte = Serial.read();
        dispatch(incomingByte);
        // digitalWrite(13, HIGH);
        delay(5); // set timing of single event loop; delay also eliminates effects of sensor bounce
        // digitalWrite(13, LOW);
      }
      
     // threshold = analogRead(1); // dynamically set threshold
    check_for_sensor_edge(); //check for sensor edge
    update_fades();
   
    //delay(10); // set timing of single event loop; delay also eliminates effects of sensor bounce
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
                if ((xbits == 1) || (xbits == 0))
                  {
                    light1power = 4;
                  }
                if ((xbits == 2) || (xbits == 0))
                  {
                    light2power = 4;
                  }
                if ((xbits == 3) || (xbits == 0))
                  {
                    light3power = 4;
                  }
                lighton();                
                break;

              case 6:
                // set brightness medium
                if ((xbits == 1) || (xbits == 0))
                  {
                    light1power = 2;
                  }
                if ((xbits == 2) || (xbits == 0))
                  {
                    light2power = 2;
                  }
                if ((xbits == 3) || (xbits == 0))
                  {
                    light3power = 2;
                  }
                lighton();                               
                break;

              case 7:
                // set brightness high
                if ((xbits == 1) || (xbits == 0))
                  {
                    light1power = 1;
                  }
                if ((xbits == 2) || (xbits == 0))
                  {
                    light2power = 1;
                  }
                if ((xbits == 3) || (xbits == 0))
                  {
                    light3power = 1;
                  }
                lighton();                  
                break;
                
            }
          break;
        
        case 1:
          // Serial.println("command 1 - motor commands");
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
           
                // pwm.setPWM(motore, 0, 200); // this is the correct way to initialize motor duty cycle to 100%
                motorspeed = 3;
                break;

              case 5: //set motorspeed faster
               // pwm.setPWM(motore, 0, 1200); // this is the correct way to initialize motor duty cycle to 100%
                motorspeed = 7;
                break;

              case 6: //set motorspeed fastest
               // pwm.setPWM(motore, 4096, 0); // this is the correct way to initialize motor duty cycle to 100%
                motorspeed = 10;
                break;
            } 
          break;
        
        case 2:
          // Serial.println("command 2 - fade color to");
          fadeto();
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
                      tfade = 1000;
                      break;
                  }
                break;
                
              case 1:
                Serial.readBytes(rgb, 3);
                setlightcolor(false);
                break;                 
            }
          break;

        case 4:
          // Serial.println("command 4 - set color to");
          setlightcolor();
          break;
       
      }
  }

void lighton()
// need to check for special cases of all on and all off
  {
    if ((xbits == 1) || (xbits == 0)) 
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(7-i, 0, (RGBWtable[4 * light1color + i])/light1power);}
        light1_is_on = true;
      }
       
    if ((xbits == 2) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(3 - i, 0, (RGBWtable[4 * light2color + i])/light2power);}
        light2_is_on = true;
      }
            
    if ((xbits == 3) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(11 - i, 0, (RGBWtable[4 * light3color + i])/light3power);}
        light3_is_on = true;
      }      
  }

void lightoff()
  {
    // Serial.println("turn off light");
    if ((xbits == 1) || (xbits == 0)) 
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(7-i, 0, 4096);}
        light1_is_on = false;
      }
       
    if ((xbits == 2) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(3 - i, 0, 4096);}
        light2_is_on = false;
      }
            
    if ((xbits == 3) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {pwm.setPWM(11 - i, 0, 4096);}
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
            {pwm.setPWM(7-i, 0, (RGBWtable[4 * light1color + i])/light1power);}
          else
            {pwm.setPWM(7-i, 0, 4096);}
        }
      light1_is_on = !light1_is_on;
      }
    
    if ((xbits == 2) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {
            if (!light2_is_on)
              {pwm.setPWM(3 - i, 0, (RGBWtable[4 * light2color + i])/light2power);}
            else
              {pwm.setPWM(3 - i, 0, 4096);}
          }
        light2_is_on = !light2_is_on;
      }
  
    if ((xbits == 3) || (xbits == 0))
      {
        for (int i = 0; i <= 3; i++)
          {
            if (!light3_is_on)
              {pwm.setPWM(11 - i, 0, (RGBWtable[4 * light3color + i])/light2power);}
            else
              {pwm.setPWM(11 - i, 0, 4096);}
          }
        light3_is_on = !light3_is_on;
      }
    
  }

  void setlightcolor(bool transform) // set color to, xbits contains which light, ybits contains which color
    {
      if (ybits == 7) 
        {ybits = random(7);
        while (ybits == lastsurprise)
          {
            ybits = random(7);
          }
          lastsurprise = ybits;
        }
      if(!transform){
        getPWMColor();  
      }
      if ((xbits == 1) || (xbits == 0))
        {
          if(transform){
            light1color = ybits;
          }
          for (int i = 0; i <= 3; i++)
            {pwm.setPWM(7-i, 0, transform ? (RGBWtable[4 * light1color + i])/light1power : rgb[i]);}
          light1_is_on = true;
        }

     if ((xbits == 2) || (xbits == 0))
       {
         if(transform){
            light2color = ybits;
         }
         for (int i = 0; i <= 3; i++)
           {pwm.setPWM(3 - i, 0, transform ? (RGBWtable[4 * light2color + i])/light2power : rgb[i]);}
         light2_is_on = true;
       }
      
     if ((xbits == 3) || (xbits == 0))
       {
         if(transform){
            light3color = ybits;
         }
         for (int i = 0; i <= 3; i++)
           {pwm.setPWM(11 - i, 0, transform ? (RGBWtable[4 * light3color + i])/light3power : rgb[i]);}
         light3_is_on = true;
       } 
      
  }

void getPWMColor(){
    float adjR = (rgb[0]/255.0) * 4095;
    float adjG = (rgb[1]/255.0) * 4095;
    float adjB = (rgb[2]/255.0) * 4095;
    float scale = (adjR + adjG + adjB) / 4095.0;
    rgb[0] = (int)(adjR/scale);
    rgb[1] = (int)(adjG/scale);
    rgb[2] = (int)(adjB/scale);
}

void fadein()
  {
     if ((xbits == 1) || (xbits == 0))
       {
         t1start = millis();
         light1_is_fading_in = true;
         light1_is_fading_out = false;
         light1_is_fading_to = false;
       }
      if ((xbits == 2) || (xbits == 0))
       {
         t2start = millis();
         light2_is_fading_in = true;
         light2_is_fading_out = false;
         light2_is_fading_to = false;
       }
     if ((xbits == 3) || (xbits == 0))
       {
         t3start = millis();
         light3_is_fading_in = true;
         light3_is_fading_out = false;
         light3_is_fading_to = false;
       }
  }


void fadeout()
  {
     if ((xbits == 1) || (xbits == 0))
       {
         t1start = millis();
         light1_is_fading_in = false;
         light1_is_fading_out = true;
         light1_is_fading_to = false;
       }
      if ((xbits == 2) || (xbits == 0))
       {
         t2start = millis();
         light2_is_fading_in = false;
         light2_is_fading_out = true;
         light2_is_fading_to = false;
       }
     if ((xbits == 3) || (xbits == 0))
       {
         t3start = millis();
         light3_is_fading_in = false;
         light3_is_fading_out = true;
         light3_is_fading_to = false;
       }
  }

void fadeto()
  {
      if (ybits == 7) 
        {ybits = random(7);
        while (ybits == lastsurprise)
          {
            ybits = random(7);
          }
          lastsurprise = ybits;
        }
     if (((xbits == 1) || (xbits == 0)) && light1_is_fading_to == false)
       { 
         t1start = millis();
         light1_is_fading_in = false;
         light1_is_fading_out = false;
         light1_is_fading_to = true;
         light1newcolor = ybits;
       }
      if ((xbits == 2) || (xbits == 0))
       {
         t2start = millis();
         light2_is_fading_in = false;
         light2_is_fading_out = false;
         light2_is_fading_to = true;
         light2newcolor = ybits;
       }
     if ((xbits == 3) || (xbits == 0))
       {
         t3start = millis();
         light3_is_fading_in = false;
         light3_is_fading_out = false;
         light3_is_fading_to = true;
         light3newcolor = ybits;
       }
  }

void update_fades()
  {
    if (light1_is_fading_in)
      {
        t1 = millis() - t1start;
        ptr = int(tablesize * t1 / tfade);
        if (ptr <= tablesize - 1)
          {
            int x = fadetable[ptr]; // comment out?
          
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetable[ptr];
                int y = RGBWtable[4 * light1color + i];
                int z = (map(x, 0, 4095, 0, y))/light1power;
                pwm.setPWM(7-i, 0, z);
              }
            
          }
        else
          {
            light1_is_fading_in = false;
            light1_is_on = true;
          }
   
      }

    if (light1_is_fading_out)
      {
        t1 = millis() - t1start;
        ptr = int(tablesize * t1 / tfade);
        ptr = tablesize - 1 - ptr;
        if (ptr >= 0)
          {
            int x = fadetable[ptr]; // comment out?
            
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetable[ptr];
                int y = RGBWtable[4 * light1color + i];
                int z = (map(x, 0, 4095, 0, y))/light1power;
                pwm.setPWM(7-i, 0, z);
              }
          }
        else
          {
            light1_is_fading_out = false;
            light1_is_on = false;
          }
   
      }
      
    if (light2_is_fading_in)
          {
            t2 = millis() - t2start;
            ptr = int(tablesize * t2 / tfade);
            if (ptr <= tablesize - 1)
              {
                int x = fadetable[ptr]; // comment out?
              
                for (int i = 0; i <= 3; i++)
                  {
                    int x = fadetable[ptr];
                    int y = RGBWtable[4 * light2color + i];
                    int z = (map(x, 0, 4095, 0, y))/light2power;
                    pwm.setPWM(3-i, 0, z);
                  }
                
              }
            else
              {
                light2_is_fading_in = false;
                light2_is_on = true;
              }
       
          }
    
    if (light2_is_fading_out)
      {
        t2 = millis() - t2start;
        ptr = int(tablesize * t2 / tfade);
        ptr = tablesize - 1 - ptr;
        if (ptr >= 0)
          {
            int x = fadetable[ptr];  // comment out?
            
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetable[ptr];
                int y = RGBWtable[4 * light2color + i];
                int z = (map(x, 0, 4095, 0, y))/light2power;
                pwm.setPWM(3-i, 0, z);
              }
          }
        else
          {
            light2_is_fading_out = false;
            light2_is_on = false;
          }
   
      }

    if (light3_is_fading_in)
          {
            t3 = millis() - t3start;
            ptr = int(tablesize * t3 / tfade);
            if (ptr <= tablesize - 1)
              {
                int x = fadetable[ptr];  // comment out?
              
                for (int i = 0; i <= 3; i++)
                  {
                    int x = fadetable[ptr];
                    int y = RGBWtable[4 * light3color + i];
                    int z = (map(x, 0, 4095, 0, y))/light3power;
                    pwm.setPWM(11-i, 0, z);
                  }
                
              }
            else
              {
                light3_is_fading_in = false;
                light3_is_on = true;
              }
       
          }
    
    if (light3_is_fading_out)
      {
        t3 = millis() - t3start;
        ptr = int(tablesize * t3 / tfade);
        ptr = tablesize - 1 - ptr;
        if (ptr >= 0)
          {
            int x = fadetable[ptr];  // comment out?
            
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetable[ptr];
                int y = RGBWtable[4 * light3color + i];
                int z = (map(x, 0, 4095, 0, y))/light3power;
                pwm.setPWM(11-i, 0, z);
              }
          }
        else
          {
            light3_is_fading_out = false;
            light3_is_on = false;
          }
   
      }

    if (light1_is_fading_to)
      {
        t1 = millis() - t1start;
        ptr = int(tablesize * t1 / tfade);
        if (ptr <= tablesize - 1)
          {
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetotable[ptr];
                int ystart = RGBWtable[4 * light1color + i];
                int ystop = RGBWtable[4 * light1newcolor + i];
                if (ystop != ystart)
                  {
                    int z = (map(x, 0, 4095, ystart, ystop))/light1power;
                    pwm.setPWM(7-i, 0, z);
                  }
              }         
          }
        else
          {
            light1_is_fading_to = false;
            light1color = light1newcolor;
            light1_is_on = true;
            for (int i = 0; i <= 3; i++)
              {pwm.setPWM(7-i, 0, (RGBWtable[4 * light1color + i])/light1power);}
            
          }
   
      }

    if (light2_is_fading_to)
      {
        t2 = millis() - t2start;
        ptr = int(tablesize * t2 / tfade);
        if (ptr <= tablesize - 1)
          {
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetotable[ptr];
                int ystart = RGBWtable[4 * light2color + i];
                int ystop = RGBWtable[4 * light2newcolor + i];
                if (ystop != ystart)
                  {
                    int z = (map(x, 0, 4095, ystart, ystop))/light2power;
                    pwm.setPWM(3-i, 0, z);
                  }
              }         
          }
        else
          {
            light2_is_fading_to = false;
            light2color = light2newcolor;
            light2_is_on = true;
            for (int i = 0; i <= 3; i++)
              {pwm.setPWM(3-i, 0, (RGBWtable[4 * light2color + i])/light2power);}
            
          }
   
      }      

    if (light3_is_fading_to)
      {
        t3 = millis() - t3start;
        ptr = int(tablesize * t3 / tfade);
        if (ptr <= tablesize - 1)
          {
            for (int i = 0; i <= 3; i++)
              {
                int x = fadetotable[ptr];
                int ystart = RGBWtable[4 * light3color + i];
                int ystop = RGBWtable[4 * light3newcolor + i];
                if (ystop != ystart)
                  {
                    int z = (map(x, 0, 4095, ystart, ystop))/light3power;
                    pwm.setPWM(11-i, 0, z);
                  }
              }         
          }
        else
          {
            light3_is_fading_to = false;
            light3color = light3newcolor;
            light3_is_on = true;
            for (int i = 0; i <= 3; i++)
              {pwm.setPWM(11-i, 0, (RGBWtable[4 * light3color + i])/light3power);}
            
          }
   
      }
      
  }


void check_for_sensor_edge() // check for sensor edge 
  {
    newsensor = (analogRead(0) < threshold);
    if ((newsensor != oldsensor) && (newsensor == true))
      {
        Serial.write(0);
        digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(10);              // wait for a second
        digitalWrite(13, LOW); 
      }
//    if ((newsensor != oldsensor) && (newsensor == false))
//      {
//        Serial.write(1);
//      }
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
        pwm.setPWM(i, 4096, 0); // 100% duty cycle 
        delay(100); // bootflash
        pwm.setPWM(i, 0, 4096); //0% duty cycle 
      }
  }

void fadetable_init() // create a look-up table that grows expoenentially from 1 to 4095
  {
    for (int i = 0; i < tablesize; i++)
      { float x = float (i) * 12 / tablesize;
        x = pow(2,x);
        fadetable[i] = int(x);
        fadetotable[i] = map(i,0,tablesize - 1, 0, 4095);
      }
    fadetable[tablesize - 1] = 4095;
    fadetotable[tablesize - 1] = 4095;
  }



