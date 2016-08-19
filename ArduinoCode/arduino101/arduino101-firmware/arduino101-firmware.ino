#include <CurieBLE.h>
#include <CurieIMU.h>

#define CALIBRATE_OFFSETS

// frequency to read sensor values in mu
const unsigned long SENSOR_UPDATE_FREQ = 50000;
const unsigned long IMU_READ_FREQ = 50000;

const byte CMD_DIGITAL_WRITE = 0x73;
const byte CMD_DIGITAL_READ = 0x74;
const byte CMD_ANALOG_WRITE = 0x75;
const byte CMD_ANALOG_READ = 0x76;
const byte CMD_PIN_MODE = 0x77;

BLEPeripheral blePeripheral;
BLEService bleService("a56ada00-ed09-11e5-9c97-0002a5d5c51b");

BLECharacteristic analogWriteChar("a56ada01-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLEWrite, 2);
BLECharacteristic digitalWriteChar("a56ada02-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLEWrite, 2);
BLECharacteristic analogReadChar("a56ada03-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLENotify, 6);
BLECharacteristic digitalReadChar("a56ada04-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLENotify, 12);
BLECharacteristic digitalModeChar("a56ada05-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLEWrite | BLENotify, 12);
BLECharacteristic imuChar("a56ada06-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLENotify, 4);
BLECharCharacteristic imuShockChar("a56ada07-ed09-11e5-9c97-0002a5d5c51b", BLERead | BLEWrite | BLENotify);

unsigned long nextSensorUpdate;
unsigned long nextIMURead;

int axRaw, ayRaw, azRaw;
int gxRaw, gyRaw, gzRaw;
double ax, ay, az;
double gx, gy;
double compAngleX, compAngleY;
double roll, pitch;
double dt;

byte analogReadVals[6];
byte digitalReadVals[12];
byte digitalPinModes[12];
unsigned char imuData[4];

boolean newDigitalVal = false;
boolean imuShockDetected = false;

void setup() {
  //digitalWrite(A0, HIGH);  // set pullup on analog pin 0 
  Serial.begin(9600);
  Serial.println("Start");

  CurieIMU.begin();
  CurieIMU.attachInterrupt(curieInterrupt);
  CurieIMU.setDetectionThreshold(CURIE_IMU_SHOCK, 2500); //2.5g
  CurieIMU.setDetectionDuration(CURIE_IMU_SHOCK, 1000); // 1000ms
  CurieIMU.interrupts(CURIE_IMU_SHOCK);
  CurieIMU.setAccelerometerRange(2);
  CurieIMU.setGyroRange(250);

  #ifdef CALIBRATE_OFFSETS
    CurieIMU.autoCalibrateGyroOffset();
    CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
  #endif

  CurieIMU.readAccelerometer(axRaw, ayRaw, azRaw);
  ax = convertRawAcceleration(axRaw);
  ay = convertRawAcceleration(ayRaw);
  az = convertRawAcceleration(azRaw);

  compAngleX = atan2(ay, az) * RAD_TO_DEG;
  compAngleY = atan2(-ax, az) * RAD_TO_DEG;

  nextIMURead = micros() + IMU_READ_FREQ;
  
  blePeripheral.setLocalName("Arduino101");
  blePeripheral.setAdvertisedServiceUuid(bleService.uuid());

  blePeripheral.addAttribute(bleService);
  blePeripheral.addAttribute(analogWriteChar);
  blePeripheral.addAttribute(digitalWriteChar);
  blePeripheral.addAttribute(analogReadChar);
  blePeripheral.addAttribute(digitalReadChar);
  blePeripheral.addAttribute(digitalModeChar);
  blePeripheral.addAttribute(imuChar);
  blePeripheral.addAttribute(imuShockChar);

  analogWriteChar.setEventHandler(BLEWritten, analogWriteWritten);
  digitalWriteChar.setEventHandler(BLEWritten, digitalWriteWritten);
  digitalModeChar.setEventHandler(BLEWritten, digitalModeWritten);
  
  memset(analogReadVals, 0, sizeof(analogReadVals));
  memset(digitalReadVals, 0, sizeof(digitalReadVals));
  memset(digitalPinModes, 0, sizeof(digitalPinModes));
  memset(imuData, 0, sizeof(imuData));

  analogReadChar.setValue(analogReadVals, 6);
  digitalReadChar.setValue(digitalReadVals, 12);
  digitalModeChar.setValue(digitalPinModes, 12);
  imuChar.setValue(imuData, 4);
  imuShockChar.setValue(0);
  
  blePeripheral.begin();
}

void loop() {
  BLECentral bleCentral = blePeripheral.central();

  if (bleCentral) {
    while (bleCentral.connected()) {
      blePeripheral.poll();
  
      if ((long)(micros() - nextSensorUpdate) >= 0) {
        updateSensorValues();
        nextSensorUpdate += SENSOR_UPDATE_FREQ;
      }

      if ((long)(micros() - nextIMURead) >= 0) {
        updateIMUValues();
      }
    }
  }
}

void analogWriteWritten(BLECentral& central, BLECharacteristic& characteristic) {
  byte pin = characteristic.value()[0];
  byte val = characteristic.value()[1];
  
  if (digitalPinModes[pin-2] != OUTPUT) {
    pinMode(pin, OUTPUT);
    digitalPinModes[pin-2] = OUTPUT;
    //digitalModeChar.setValue(digitalPinModes, 12);
  }
  analogWrite(pin, val);
}

void digitalWriteWritten(BLECentral& central, BLECharacteristic& characteristic) {
  byte pin = characteristic.value()[0];
  byte val = characteristic.value()[1];
  
  if (digitalPinModes[pin-2] != OUTPUT) {
    pinMode(pin, OUTPUT);
    digitalPinModes[pin-2] = OUTPUT;
    //digitalModeChar.setValue(digitalPinModes, 12);
  }
  digitalWrite(pin, val);
}

void digitalModeWritten(BLECentral& central, BLECharacteristic& characteristic) {
  byte i, mode;
  for (i=0; i<12; i++) {
    mode = characteristic.value()[i];
    if (digitalPinModes[i] != mode) {
      pinMode(i+2, mode);
      digitalPinModes[i] = mode;
    }
  }
}

float convertRawAcceleration(int aRaw) {
  // since we are using 2G range
  // -2g maps to a raw value of -32768
  // +2g maps to a raw value of 32767
  float a = (aRaw * 2.0) / 32768.0;
  return a;
}

float convertRawGyro(int gRaw) {
  // since we are using 250 degrees/seconds range
  // -250 maps to a raw value of -32768
  // +250 maps to a raw value of 32767
  float g = (gRaw * 250.0) / 32768.0;
  return g;
}

void curieInterrupt(void) {
  if (CurieIMU.getInterruptStatus(CURIE_IMU_SHOCK)) {
    imuShockDetected = true;
  }
}

void updateIMUValues() {
  CurieIMU.readMotionSensor(axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw);
  ax = convertRawAcceleration(axRaw);
  ay = convertRawAcceleration(ayRaw);
  az = convertRawAcceleration(azRaw);
  gx = convertRawGyro(gxRaw);
  gy = convertRawGyro(gyRaw);

  dt = (double)(micros() - (nextIMURead - IMU_READ_FREQ)) / 1000000; // Calculate delta time
  nextIMURead = micros() + IMU_READ_FREQ;
  
  roll  = atan2(ay, az) * RAD_TO_DEG;
  pitch  = atan2(-ax, az) * RAD_TO_DEG;

  compAngleX = 0.93 * (compAngleX + gx * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
  compAngleY = 0.93 * (compAngleY + gy * dt) + 0.07 * pitch;
}

void updateSensorValues() {
  byte i, val;
  
  // Update IMU values
  imuData[0] = (compAngleX < 0);
  imuData[1] = abs(compAngleX);
  imuData[2] = (compAngleY < 0);
  imuData[3] = abs(compAngleY);
  imuChar.setValue(imuData, 4);

  if (imuShockDetected) {
    imuShockChar.setValue(1);
    imuShockDetected = false;
  }
  
  // Update Analog values
  for (i=0; i<=5; i++) {
    analogReadVals[i] = map(analogRead(i), 0, 1023, 0, 255);
  }
  analogReadChar.setValue(analogReadVals, 6);

  // Update Digital values
  for (i=0; i<=11; i++) {
    if (digitalPinModes[i]) {
      val = digitalRead(i+2);
      if (digitalReadVals[i] != val) {
        digitalReadVals[i] = val;
        newDigitalVal = true;
      }
    }
  }
  if (newDigitalVal) {
    digitalReadChar.setValue(digitalReadVals, 12);
    newDigitalVal = false;
  }
}
