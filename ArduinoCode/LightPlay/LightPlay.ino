byte incomingByte = 0;  // for incoming serial data
byte val = 0; // for analog sensor value

// for edge detection
int oldsensor1 = 0;
int newsensor1 = 0;
int oldsensor2 = 0;
int newsensor2 = 0;

// for fades
float imin = 2; // initial value of pwm duty cycle for fade in
  // these variables track current value of duty cycle
float iin1 = imin;
float iin2 = imin;
float iin3 = imin;
float iin4 = imin;
float iout1 = 255;
float iout2 = 255;
float iout3 = 255;
float iout4 = 255;
  // these flags are set when fade is in progress
int fadein1 = 0;
int fadein2 = 0;
int fadein3 = 0;
int fadein4 = 0;
int fadeout1 = 0;
int fadeout2 = 0;
int fadeout3 = 0;
int fadeout4 = 0;


int ch1a = 10;
int ch1b = 11;
int ch1e = 6;

int ch2a = 12;
int ch2b = 13;
int ch2e = 8; // v. 1.0 of layout has pins 8 and 9 swapped,  pin 9 has pwm capabilities but pin 8 does not! so channel 2 always runs at full power

int ch3a = 2;
int ch3b = 4;
int ch3e = 3;

int ch4a = 7;
int ch4b = 9;
int ch4e = 5;

int dirstate = 255;  // bitwise direction state
int onstate = 0; // bitwise onstate state; intially off; can easily add thisway/thatway command

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps, same as Python
  // initialize the digital pins as outputs.
  pinMode(ch1a, OUTPUT);
  pinMode(ch1b, OUTPUT);
  pinMode(ch1e, OUTPUT);
  pinMode(ch2a, OUTPUT);
  pinMode(ch2b, OUTPUT);
  pinMode(ch2e, OUTPUT);
  pinMode(ch3a, OUTPUT);
  pinMode(ch3b, OUTPUT);
  pinMode(ch3e, OUTPUT);
  pinMode(ch4a, OUTPUT);
  pinMode(ch4b, OUTPUT);
  pinMode(ch4e, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);

  // set digital sensor pin to have internal 20 k pullup resistor to HIGH
  // so that connecting the sensor leads will drive these pins LOW without drawing much current


  // initially set L293 inputs LOW, but with chip enabled
  digitalWrite(ch1a, LOW);
  digitalWrite(ch1b, LOW);
  digitalWrite(ch1e, HIGH);
  digitalWrite(ch2a, LOW);
  digitalWrite(ch2b, LOW);
  digitalWrite(ch2e, HIGH);
  digitalWrite(ch3a, LOW);
  digitalWrite(ch3b, LOW);
  digitalWrite(ch3e, HIGH);
  digitalWrite(ch4a, LOW);
  digitalWrite(ch4b, LOW);
  digitalWrite(ch4e, HIGH);

  oldsensor1 = digitalRead(A1);
  oldsensor2 = digitalRead(A0);

}

void loop() {
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
      case 0: // turn actuator off     
        if (channel == 1  || channel == 0) {
          digitalWrite(ch1a, LOW);
          digitalWrite(ch1b, LOW);
        }

        if (channel == 2  || channel == 0) {
          digitalWrite(ch2a, LOW);
          digitalWrite(ch2b, LOW);
        }

        if (channel == 3  || channel == 0) {
          digitalWrite(ch3a, LOW);
          digitalWrite(ch3b, LOW);
        }

        if (channel == 4  || channel == 0) {
          digitalWrite(ch4a, LOW);
          digitalWrite(ch4b, LOW);
        }

        if (channel == 0) {
          onstate = 0; // all bits off
        }

        else {
          onstate = bitClear(onstate, channel);
        }
        break;
        
      case 1: // turn on port       
        if (channel == 1 || channel == 0) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          digitalWrite(ch1e, HIGH);
        }

        if (channel == 2 || channel == 0) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          digitalWrite(ch2e, HIGH);
        }

        if (channel == 3 || channel == 0) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          digitalWrite(ch3e, HIGH);
        }

        if (channel == 4 || channel == 0) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          digitalWrite(ch4e, HIGH);
        }

        if (channel == 0) {
          onstate = 255; // all bits on
        }

        else {
          onstate = bitSet(onstate, channel);
        }
        break;

      case 2: // reverse

        if (channel == 1 || channel == 0) {
          digitalWrite(ch1a, !digitalRead(ch1a));
          digitalWrite(ch1b, !digitalRead(ch1b));
          dirstate = dirstate ^ 2; // xor to toggle bit 1
        }

        if (channel == 2 || channel == 0) {
          digitalWrite(ch2a, !digitalRead(ch2a));
          digitalWrite(ch2b, !digitalRead(ch2b));
          dirstate = dirstate ^ 4; // xor to toggle bit 2
        }

        if (channel == 3 || channel == 0) {
          digitalWrite(ch3a, !digitalRead(ch3a));
          digitalWrite(ch3b, !digitalRead(ch3b));
          dirstate = dirstate ^ 8; // xor to toggle bit 3
        }

        if (channel == 4 || channel == 0) {
          digitalWrite(ch4a, !digitalRead(ch4a));
          digitalWrite(ch4b, !digitalRead(ch4b));
          dirstate = dirstate ^ 16; // xor to toggle bit 4
        }
        break;

      case 3: // fade in
        if (channel == 1 || channel == 0) {
          iin1 = imin;
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          analogWrite(ch1e, iin1);
          fadein1 = 1; // turn fadein flag on
        }
        if (channel == 2 || channel == 0) {
          iin2 = imin;
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          analogWrite(ch2e, iin2);
          fadein2 = 1;
        }
        if (channel == 3 || channel == 0) {
          iin3 = imin;
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          analogWrite(ch3e, iin3);
          fadein3 = 1;
        }
        if (channel == 4 || channel == 0) {
          iin4 = imin;
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          analogWrite(ch4e, iin4);
          fadein4 = 1;
        }
        
        break;

    case 4: // fade out
        if (channel == 1 || channel == 0) {
          iout1 = 255;
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          analogWrite(ch1e, iout1);
          fadeout1 = 1;
        }
        if (channel == 2 || channel == 0) {
          iout2 = 255;
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          analogWrite(ch2e, iout2);
          fadeout2 = 1;
        }
        if (channel == 3 || channel == 0) {
          iout3 = 255;
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          analogWrite(ch3e, iout3);
          fadeout3 = 1;
        }
        if (channel == 4 || channel == 0) {
          iout4 = 255;
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          analogWrite(ch4e, iout4);
          fadeout4 = 1;
        }
        break;

      case 5: // setpowerlow
        if (channel == 1  || channel == 0) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          analogWrite(ch1e, 10);
        }
        if (channel == 2  || channel == 0) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          analogWrite(ch2e, 10);
        }

        if (channel == 3  || channel == 0) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          analogWrite(ch3e, 10);
        }
        if (channel == 4  || channel == 0) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          analogWrite(ch4e, 10);
        }
        
        break;

      case 6: // setpowermedium
        if (channel == 1 || channel == 0) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          analogWrite(ch1e, 50);
        }
        if (channel == 2 || channel == 0) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          analogWrite(ch2e, 50);
        }

        if (channel == 3 || channel == 0) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          analogWrite(ch3e, 50);
        }
        if (channel == 4 || channel == 0) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          analogWrite(ch4e, 50);
        }
        break;

      case 7: // setpowerhigh
        if (channel == 1  || channel == 0) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          analogWrite(ch1e, 255);
        }
        if (channel == 2 || channel == 0) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          analogWrite(ch2e, 255);
        }

        if (channel == 3 || channel == 0) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
         analogWrite(ch3e, 255);
        }
        if (channel == 4 || channel == 0) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          analogWrite(ch4e, 255);
        }     
        break;

      case 8:
        // sensor1 is wired to Arduino analog pin A1

        if (channel == 1) {

          val = analogRead(A1) / 4;   // read the input pin
          val = map(val, 0, 255, 3, 103); // remap 10 bit range to 3 to 103, In Python subtract 3 to get sensorval
          Serial.write(val); // and send it to Python
        }
        // sensor2 is wired to Arduino analog pin A1
        if (channel == 2) {

          val = analogRead(A0) / 4;   // read the input pin
          val = map(val, 0, 255, 104, 204); // remap 10 bit range to 104 to 204, In Python subtract 104 to get sensorval
          Serial.write(val); // and send it to Python
        }
        break;

      // skip command  9; it is used for edge detection in Python
      
      case 10: // toggle on / off
        if (channel == 1 || channel == 0) {
          if (bitRead(onstate, 1)) {
            digitalWrite(ch1a, LOW);
            digitalWrite(ch1b, LOW);
          }
          else {
            digitalWrite(ch1a, bitRead(dirstate, 1));
            digitalWrite(ch1b, !bitRead(dirstate, 1));
            digitalWrite(ch1e, HIGH);
          }
          onstate = onstate ^ 2; // xor to toggle bit 1
        }
        if (channel == 2 || channel == 0) {
          if (bitRead(onstate, 2)) {
            digitalWrite(ch2a, LOW);
            digitalWrite(ch2b, LOW);
          }
          else {
            digitalWrite(ch2a, bitRead(dirstate, 2));
            digitalWrite(ch2b, !bitRead(dirstate, 2));
            digitalWrite(ch2e, HIGH);
          }
          onstate = onstate ^ 4; // xor to toggle bit 2
        }
        if (channel == 3 || channel == 0) {
          if (bitRead(onstate, 3)) {
            digitalWrite(ch3a, LOW);
            digitalWrite(ch3b, LOW);
          }
          else {
            digitalWrite(ch3a, bitRead(dirstate, 3));
            digitalWrite(ch3b, !bitRead(dirstate, 3));
            digitalWrite(ch3e, HIGH);
          }
          onstate = onstate ^ 8; // xor to toggle bit 3
        }
        if (channel == 4 || channel == 0) {
          if (bitRead(onstate, 4)) {
            digitalWrite(ch4a, LOW);
            digitalWrite(ch4b, LOW);
          }
          else {
            digitalWrite(ch4a, bitRead(dirstate, 4));
            digitalWrite(ch4b, !bitRead(dirstate, 4));
            digitalWrite(ch4e, HIGH);
          }
          onstate = onstate ^ 16; // xor to toggle bit 4
        }
        break;

    } // end of switch
  } // end of  if incoming byte then dispatch
  
  //edge detection
  newsensor1 = digitalRead(A1);
  newsensor2 = digitalRead(A0);
  if ((newsensor1 != oldsensor1) && (newsensor1 == 0))
  { Serial.write(1);
  }
  if ((newsensor2 != oldsensor2) && (newsensor2 == 0))
  { Serial.write(2);
  }
  oldsensor1 = newsensor1;
  oldsensor2 = newsensor2;

  // update fades
  if (fadein1 == 1  && iin1 <= 254) {
          analogWrite(ch1e, iin1);
          iin1 = 30 * iin1 / 29;
          if (iin1   >= 254) {
            fadein1 = 0;
            iin1 = 255;
          }
        }
 if (fadein2 == 1  && iin2 <= 254) {
          analogWrite(ch2e, iin2);
          iin2 = 30 * iin2 / 29;
          if (iin2   >= 254) {
            fadein2 = 0;
            iin2 = 255;
          }
        }
if (fadein3 == 1  && iin3 <= 254) {
          analogWrite(ch3e, iin3);
          iin3 = 30 * iin3 / 29;
          if (iin3   >= 254) {
            fadein3 = 0;
            iin3 = 255;
          }
        }
if (fadein4 == 1  && iin4 <= 254) {
          analogWrite(ch4e, iin4);
          iin4 = 30 * iin4 / 29;
          if (iin4   >= 254) {
            fadein4 = 0;
            iin4 = 255;
          }
        }
if (fadeout1 == 1  && iout1 >= imin) {
          analogWrite(ch1e, iout1);
          iout1 = 29 * iout1 / 30;
          if (iout1   <= imin) {
            fadeout1 = 0;
            analogWrite(ch1e, 0);
          }
        }
if (fadeout2 == 1  && iout2 >= imin) {
          analogWrite(ch2e, iout2);
          iout2 = 29 * iout2 / 30;
          if (iout2   <= imin) {
            fadeout2 = 0;
            analogWrite(ch2e, 0);
          }
        }
if (fadeout3 == 1  && iout3 >= imin) {
          analogWrite(ch3e, iout3);
          iout3 = 29 * iout3 / 30;
          if (iout3   <= imin) {
            fadeout3 = 0;
            analogWrite(ch3e, 0);
          }
        }
if (fadeout4 == 1  && iout4 >= imin) {
          analogWrite(ch4e, iout4);
          iout4 = 29 * iout4 / 30;
          if (iout4   <= imin) {
            fadeout4 = 0;
            analogWrite(ch4e, 0);
          }
        }
  delay(50); // set timing of single event loop; also eliminates effects of switch bounce
} // end of global loop

