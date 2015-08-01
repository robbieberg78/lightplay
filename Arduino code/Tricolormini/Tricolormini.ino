
// output pin numbers:
int white = 9; // this in the pinnumber for the white LED
int blue = 6; // this in the pinnumber for the blue LED
int green = 5; // this in the pinnumber for the green LED
int red = 3; // this in the pinnumber for the red LED

// these are next four variables contain the  current "WRGB" values that are used to pwm the outputs
int whiteval = 0; 
int blueval = 0;
int greenval = 0;
int redval = 0;

// these are next four variables contain the  destination "WRGB" values reached in the change color command
int newwhiteval = 0;
int newblueval = 0;
int newgreenval = 0;
int newredval = 0;

// these are next four variables contain the net change in  "WRGB" values in the change color command
int dw = 0;
int db = 0;
int dg = 0;
int dr = 0;

int counter = 0; // loop counter used in change color
int val = 0; // temporary variable used in change color loop
int dt = 25; // time delay used in change color loop , I believe loop period is actually twice this number due to 16 vs. 8 MHz confusion

byte incomingByte = 0;  // for incoming serial data

void setup() {
  pinMode(13, OUTPUT);
  pinMode(white, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  analogWrite(white, 0);
  analogWrite(blue, 0);
  analogWrite(green, 0);
  analogWrite(red, 0);
  Serial.begin(19200); // Arduino IDE apparently thinks the clock speed is 16 MHz, nit 8, so timing is off by a factor of 2!!
}// setting baud rate to 19200 is ncessary in order to get 9600...!!

void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    
    byte command = incomingByte & 0x0f; // bitwise and selects lower 4 bits
    // command = 0 => turn lamp off
    // command = 1 => turn lamp on
    // comand = 2 => slowly change color of lamp
    byte color = (incomingByte & 0xf0) >> 4; //  bitwise and selects upper 4 bits, then  shift right by 4 bits
    switch (command)  {
      case 0: // turn lamp off
        analogWrite(white, 0);
        analogWrite(blue, 0);
        analogWrite(green, 0);
        analogWrite(red, 0);
        break;

      case 1: // set lamp color

        if (color == 10) { // choose a random color
          color = random(1, 10);
        }

        if (color == 1) {  // white
          whiteval = 255;
          blueval = 0;
          greenval = 0;
          redval = 0;
        }
        if (color == 2) {  // blue
          whiteval = 0;
          blueval = 255;
          greenval = 0;
          redval = 0;
        }
        if (color == 3) {  // teal
          whiteval = 0;
          blueval = 255;
          greenval = 50;
          redval = 0;
        }
        if (color == 4) { // green
          whiteval = 0;
          blueval = 0;
          greenval = 255;
          redval = 0;
        }
        if (color == 5) {  // yellow
          whiteval = 0;
          blueval = 130;
          greenval = 130;
          redval = 0;
        }
        if (color == 6) {  // orange
          whiteval = 0;
          blueval = 0;
          greenval = 75;
          redval = 255;
        }
        if (color == 7) {  // red
          whiteval = 0;
          blueval = 0;
          greenval = 0;
          redval = 255;
        }
        if (color == 8) { // pink
          whiteval = 50;
          blueval = 0;
          greenval = 0;
          redval = 255;
        }
        if (color == 9) { // purple
          whiteval = 0;
          blueval = 255;
          greenval = 0;
          redval = 100;
        }
        analogWrite(white, whiteval);
        analogWrite(blue, blueval);
        analogWrite(green, greenval);
        analogWrite(red, redval);
         
        break;

      case 2:
        if (color == 10) { // choose a random color
          color = random(1, 10);
        }

        if (color == 1) {  // slowly turn white
          newwhiteval = 255;
          newblueval = 0;
          newgreenval = 0;
          newredval = 0;
        }
      
        if (color == 2) {  // slowly turn blue
          newwhiteval = 0;
          newblueval = 255;
          newgreenval = 0;
          newredval = 0;
        }

        if (color == 2) {  // slowly turn teal
          newwhiteval = 0;
          newblueval = 255;
          newgreenval = 50;
          newredval = 0;
        }

         if (color == 4) {  // slowly turn green
          newwhiteval = 0;
          newblueval = 0;
          newgreenval = 255;
          newredval = 0;
        }

        if (color == 5) {  // slowly turn yellow
          newwhiteval = 0;
          newblueval = 0;
          newgreenval = 130;
          newredval = 130;
        }

        if (color == 6) {  // slowly turn orange
          newwhiteval = 0;
          newblueval = 0;
          newgreenval = 75;
          newredval = 255;
        }
        
        if (color == 7) {  // slowly turn red
          newwhiteval = 0;
          newblueval = 0;
          newgreenval = 0;
          newredval = 255;
        }

        if (color == 8) {  // slowly turn pink
          newwhiteval = 50;
          newblueval = 0;
          newgreenval = 0;
          newredval = 255;
        }

        if (color == 9) {  // slowly turn purple
          newwhiteval = 0;
          newblueval = 245;
          newgreenval = 0;
          newredval = 100;
        }

        
        dw = newwhiteval - whiteval;
        db = newblueval - blueval;
        dg = newgreenval - greenval;
        dr = newredval - redval;
        counter = 1;
        while (counter <= 100) {
          val = whiteval + (counter * dw)  / 100;
          analogWrite(white, val);
          val = blueval + (counter * db)  / 100;
          analogWrite(blue, val);
          val = greenval + (counter * dg)  / 100;
          analogWrite(green, val);
          val = redval + (counter * dr)  / 100;
          analogWrite(red, val);
          counter = counter + 1;
          delay(dt);
        }

        whiteval = newwhiteval;
        blueval = newblueval;
        greenval = newgreenval;
        redval = newredval;

        break;



    }

  }

}
