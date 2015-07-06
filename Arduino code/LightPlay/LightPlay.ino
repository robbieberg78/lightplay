byte incomingByte = 0;  // for incoming serial data
byte pwmchannel = 1;
byte val = 0; // for analog sensor value
int oldsensor1 = 0;
int newsensor1 = 0;
int oldsensor2 = 0;
int newsensor2 = 0;

float i = 1;
float imin = 5;

int ch1a = 10;
int ch1b = 11;
int ch1e = 6;

int ch2a = 12;
int ch2b = 13;
int ch2e = 8; // v. 1.0 of layout has pins 8 and 9 swapped, need to rewire to this config, pin 9 has pwm capabilities but pin 8 does not!

int ch3a = 2;
int ch3b = 4;
int ch3e = 3;

int ch4a = 7;
int ch4b = 9; // v. 1.0 of layout has pins 8 and 9 swapped, need to rewire to this config
int ch4e = 5;

int dirstate = 255;  // bitwise direction state

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps, same as Python
  // initialize the digital pins as outputs.
  pinMode(ch1a, OUTPUT);
  pinMode(ch1b, OUTPUT);
  pinMode(ch1e, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);

  // set digital sensor pin to have internal 20 k pullup resistor to HIGH
  // so that connecting the sensor leads will drive these pins LOW without drawing much current


  // turn ports off initially, but with L293's enabled
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
  if (Serial.available() == 0) {
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
        if (channel == 0) { // channel 0 talks to all four outputs
          digitalWrite(ch1a, LOW);
          digitalWrite(ch1b, LOW);
          digitalWrite(ch2a, LOW);
          digitalWrite(ch2b, LOW);
          digitalWrite(ch3a, LOW);
          digitalWrite(ch3b, LOW);
          digitalWrite(ch4a, LOW);
          digitalWrite(ch4b, LOW);
        }
        if (channel == 1) {
          digitalWrite(ch1a, LOW);
          digitalWrite(ch1b, LOW);
        }

        if (channel == 2) {
          digitalWrite(ch2a, LOW);
          digitalWrite(ch2b, LOW);
        }

        if (channel == 3) {
          digitalWrite(ch3a, LOW);
          digitalWrite(ch3b, LOW);
        }

        if (channel == 4) {
          digitalWrite(ch4a, LOW);
          digitalWrite(ch4b, LOW);
        }

        break;
      case 1: // turn on port
        if (channel == 0) { // channel 0 talks to all four outputs
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          digitalWrite(ch1e, HIGH);
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          digitalWrite(ch2e, HIGH);
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          digitalWrite(ch3e, HIGH);
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          digitalWrite(ch4e, HIGH);

        }
        if (channel == 1) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          digitalWrite(ch1e, HIGH);
        }

        if (channel == 2) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          digitalWrite(ch2e, HIGH);
        }

        if (channel == 3) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          digitalWrite(ch3e, HIGH);
        }

        if (channel == 4) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          digitalWrite(ch4e, HIGH);
        }
        break;

      case 2: // reverse
        if (channel == 0) { // channel 0 talks to all four outputs
          digitalWrite(ch1a, !digitalRead(ch1a));
          digitalWrite(ch1b, !digitalRead(ch1b));
          digitalWrite(ch2a, !digitalRead(ch2a));
          digitalWrite(ch2b, !digitalRead(ch1b));
          digitalWrite(ch3a, !digitalRead(ch3a));
          digitalWrite(ch3b, !digitalRead(ch3b));
          digitalWrite(ch3a, !digitalRead(ch3a));
          digitalWrite(ch4b, !digitalRead(ch4b));
          dirstate = dirstate ^ 255; // xor to toggle all the bits in dirstate
        }

        if (channel == 1) {
          digitalWrite(ch1a, !digitalRead(ch1a));
          digitalWrite(ch1b, !digitalRead(ch1b));
          dirstate = dirstate ^ 2; // xor to toggle bit 1

        }

        if (channel == 2) {
          digitalWrite(ch2a, !digitalRead(ch2a));
          digitalWrite(ch2b, !digitalRead(ch2b));
          dirstate = dirstate ^ 4; // xor to toggle bit 2
        }

        if (channel == 3) {
          digitalWrite(ch3a, !digitalRead(ch3a));
          digitalWrite(ch3b, !digitalRead(ch3b));
          dirstate = dirstate ^ 8; // xor to toggle bit 3
        }

        if (channel == 4) {
          digitalWrite(ch4a, !digitalRead(ch4a));
          digitalWrite(ch4b, !digitalRead(ch4b));
          dirstate = dirstate ^ 16; // xor to toggle bit 4
        }
        break;

      case 3: // fade in
        // still need to implement for channel zero case...rewrite as part of single event loop

        i = imin;
        if (channel == 1) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          pwmchannel = ch1e;
        }
        if (channel == 2) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          pwmchannel = ch2e;
        }

        if (channel == 3) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          pwmchannel = ch3e;
        }
        if (channel == 4) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          pwmchannel = ch4e;
        }
        while (i <= 254) {
          analogWrite(pwmchannel, i);
          i = 30 * i / 29;
          delay(20);
        }
        break;

      case 4: // fade out
        // still need to implement for channel zero case...rewrite as part of single event loop

        i = 255;
        if (channel == 1) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          pwmchannel = ch1e;
        }
        if (channel == 2) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          pwmchannel = ch2e;
        }

        if (channel == 3) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          pwmchannel = ch3e;
        }
        if (channel == 4) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          pwmchannel = ch4e;
        }
        while (i >= imin) {
          analogWrite(pwmchannel, i);
          i = 29 * i / 30;
          delay(20);
        }
        analogWrite(pwmchannel, 0);
        break;

      case 5: // setpowerlow
        // still need to implement for channel zero case...
        if (channel == 1) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          pwmchannel = ch1e;
        }
        if (channel == 2) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          pwmchannel = ch2e;
        }

        if (channel == 3) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          pwmchannel = ch3e;
        }
        if (channel == 4) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          pwmchannel = ch4e;
        }
        analogWrite(pwmchannel, 10);
        break;

      case 6: // setpowermedium
        // still need to implement for channel zero case...
        if (channel == 1) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          pwmchannel = ch1e;
        }
        if (channel == 2) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          pwmchannel = ch2e;
        }

        if (channel == 3) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          pwmchannel = ch3e;
        }
        if (channel == 4) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          pwmchannel = ch4e;
        }
        analogWrite(pwmchannel, 50);
        break;

      case 7: // setpowerhigh
        // still need to implement for channel zero case...
        if (channel == 1) {
          digitalWrite(ch1a, bitRead(dirstate, 1));
          digitalWrite(ch1b, !bitRead(dirstate, 1));
          pwmchannel = ch1e;
        }
        if (channel == 2) {
          digitalWrite(ch2a, bitRead(dirstate, 2));
          digitalWrite(ch2b, !bitRead(dirstate, 2));
          pwmchannel = ch2e;
        }

        if (channel == 3) {
          digitalWrite(ch3a, bitRead(dirstate, 3));
          digitalWrite(ch3b, !bitRead(dirstate, 3));
          pwmchannel = ch3e;
        }
        if (channel == 4) {
          digitalWrite(ch4a, bitRead(dirstate, 4));
          digitalWrite(ch4b, !bitRead(dirstate, 4));
          pwmchannel = ch4e;
        }
        analogWrite(pwmchannel, 255);
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
    } // end of switch
  } // end of  if incoming byte then dispatch
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
  delay(50); // to eliminate effects of switch bounce
} // end of global loop
q
