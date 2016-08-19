int led1 = 3;
 int led2 = 5;
 int led3 = 6;
 int led4 = 9;

int red = led1;
int green = led2;
int blue = led3;
int white = led4;

 
 int analogPin = 0;
 int octant = 0;
 int val = 0;
 int rem = 0;
 int firstpin = 0;
 int secondpin = 0;
 
void setup() {
//Serial.begin(9600);
pinMode(13, OUTPUT);
pinMode(led1, OUTPUT);
pinMode(led2, OUTPUT);
pinMode(led3, OUTPUT);
pinMode(led4, OUTPUT);

digitalWrite(red, HIGH);
digitalWrite(green, LOW);
digitalWrite(blue, LOW);
digitalWrite(white, LOW);
delay(20);

digitalWrite(red, LOW);
digitalWrite(green, HIGH);
digitalWrite(blue, LOW);
digitalWrite(white, LOW);
delay(20);

digitalWrite(red, LOW);
digitalWrite(green, LOW);
digitalWrite(blue, HIGH);
digitalWrite(white, LOW);
delay(20);

digitalWrite(red, LOW);
digitalWrite(green, LOW);
digitalWrite(blue, LOW);
digitalWrite(white, HIGH);
delay(20);

digitalWrite(white, LOW);


}

void loop() {
 


  val = 1023 - analogRead(analogPin);
  octant = (val /128) + 1;
  //rem = val % 256;
 // Serial.println(rem);

  
  switch (octant) {
    case 1:
      digitalWrite(red, HIGH);
      digitalWrite(green, LOW);
      digitalWrite(blue, LOW);
      digitalWrite(white, LOW);
    break;

    case 2:
    digitalWrite(blue, LOW);
    digitalWrite(white, LOW);
    for (int i = 0; i <= 10; i++){
      digitalWrite(red, LOW);
      digitalWrite(green, HIGH);
      delay(2);
      digitalWrite(green, LOW);
      digitalWrite(red, HIGH);
      delay(2);
    }
    

    break;
    
    case 3:
      digitalWrite(red, LOW);
      digitalWrite(green, HIGH);
      digitalWrite(blue, LOW);
      digitalWrite(white, LOW);
      break;

    case 4:
    digitalWrite(red, LOW);
    digitalWrite(white, LOW);
    for (int i = 0; i <= 10; i++){
      digitalWrite(green, LOW);
      digitalWrite(blue, HIGH);
      delay(2);
      digitalWrite(blue, LOW);
      digitalWrite(green, HIGH);
      delay(2);
    }
          
    
    case 5:
      digitalWrite(red, LOW);
      digitalWrite(green, LOW);
      digitalWrite(blue, HIGH);
      digitalWrite(white, LOW);
    break;

    case 6:
    digitalWrite(green, LOW);
    digitalWrite(white, LOW);
    for (int i = 0; i <= 10; i++){
      digitalWrite(red, LOW);
      digitalWrite(blue, HIGH);
      delay(2);
      digitalWrite(blue, LOW);
      digitalWrite(red, HIGH);
      delay(2);
    }
    break;
 
    case 7:
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    for (int i = 0; i <= 10; i++){
      digitalWrite(red, LOW);
      digitalWrite(white, HIGH);
      delay(2);
      digitalWrite(white, LOW);
      digitalWrite(red, HIGH);
      delay(2);
    }
                
    case 8:
      digitalWrite(red, LOW);
      digitalWrite(green, LOW);
      digitalWrite(blue, LOW);
      digitalWrite(white, HIGH);
    break;
  }

//  digitalWrite(firstpin, HIGH);
//  digitalWrite(secondpin, LOW);
//  for (int i = 0; i <= rem; i++){
//    delayMicroseconds(100);
//  }
//
//   digitalWrite(firstpin, LOW);
//   digitalWrite(secondpin, HIGH);  
//  for (int i = rem; i <= 255; i++){
//    delayMicroseconds(100);
//    }

  }

