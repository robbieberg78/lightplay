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
  analogWrite(white,0);
  analogWrite(blue,0);
  analogWrite(green,0);
  analogWrite(red,0);
  Serial.begin(115200); // opens serial port, sets data rate to 9600 bps, same as Python
  // initialize the digital pins as outputs.
}

void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    switch(incomingByte)  {
      case 0:
        analogWrite(white,0);
        analogWrite(blue,0);
        analogWrite(green,0);
        analogWrite(red,0);
      break;

      case 1:
        analogWrite(white,255);
        break;
        
      case 2:
        analogWrite(blue,255);
        break;

      case 3:
        analogWrite(green,255);
        break;

      case 4:
        analogWrite(red,255);
        break;

      case 5:
        analogWrite(red,255);
        analogWrite(green,75);
        break;

      case 6:
        analogWrite(red,200);
        analogWrite(blue,250);
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
