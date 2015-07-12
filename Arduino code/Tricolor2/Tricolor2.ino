int white = 11;
int blue = 10;
int green = 9;
int red = 6;
byte incomingByte = 0;  // for incoming serial data

void setup() {
  // put your setup code here, to run once:
  pinMode(white, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  analogWrite(white, 0);
  analogWrite(blue, 0);
  analogWrite(green, 0);
  analogWrite(red, 0);
  Serial.begin(115200); // opens serial port, sets data rate to 9600 bps, same as Python
  // initialize the digital pins as outputs.
}

void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    byte command = incomingByte & 0x0f; // bitwise and selects lower 4 bits
    // command = 0 => turn lamp off
    // command = 1 => turn lamp on
    // comand = 2 => slowly change color of lamp
    byte color = (incomingByte & 0xf0) >> 4; //  bitwise and selects upper 4 bits, then  shift right by 4 bits
    switch (command)  {
      case 0:
        analogWrite(white, 0);
        analogWrite(blue, 0);
        analogWrite(green, 0);
        analogWrite(red, 0);
        break;

      case 1:

        if (color == 10) {
          color = random(1,10);
        }
        
        if (color == 1) {  // white
          analogWrite(white, 255);
          analogWrite(blue, 0);
          analogWrite(green, 0);
          analogWrite(red, 0);
        }
        if (color == 2) {  // blue
          analogWrite(white, 0);
          analogWrite(blue, 255);
          analogWrite(green, 0);
          analogWrite(red, 0);
        }
        if (color == 3) {  // teal
          analogWrite(white, 0);
          analogWrite(blue, 180);
          analogWrite(green, 100);
          analogWrite(red, 0);
        }
        if (color == 4) { // green
          analogWrite(white, 0);
          analogWrite(blue, 0);
          analogWrite(green, 255);
          analogWrite(red, 0);
        }
        if (color == 5) {  // yellow
          analogWrite(white, 0);
          analogWrite(blue, 0);
          analogWrite(green, 130);
          analogWrite(red, 130);
        }
        if (color == 6) {  // orange
          analogWrite(white, 0);
          analogWrite(blue, 0);
          analogWrite(green, 100);
          analogWrite(red, 200);
        }
        if (color == 7) {  // red
          analogWrite(white, 0);
          analogWrite(blue, 0);
          analogWrite(green, 0);
          analogWrite(red, 255);
        }
        if (color == 8) { // pink
          analogWrite(white, 50);
          analogWrite(blue, 0);
          analogWrite(green, 0);
          analogWrite(red, 255);
        }
        if (color == 9) { // purple
          analogWrite(white, 0);
          analogWrite(blue, 170);
          analogWrite(green, 0);
          analogWrite(red, 130);
        }
        break;



    }

  }
  //  analogWrite(white,255);
  //  delay(2000);
  //  analogWrite(white,0);
  //
  //  analogWrite(blue,255);
  //  delay(2000);
  //  analogWrite(blue,0);
  //
  //  analogWrite(green,255);
  //  delay(2000);
  //  analogWrite(green,0);
  //
  //  analogWrite(red,255);
  //  delay(2000);
  //  analogWrite(red,0);
  //
  //  analogWrite(red,255);
  //  analogWrite(green,75);
  //  delay(2000);
  //  analogWrite(red,0);
  //  analogWrite(green,0);
  //
  //
  //
  //  analogWrite(red,200);
  //  analogWrite(blue,250);
  //  delay(2000);
  //  analogWrite(red,0);
  //  analogWrite(blue,0);
  //
  //  delay(2000);

}
