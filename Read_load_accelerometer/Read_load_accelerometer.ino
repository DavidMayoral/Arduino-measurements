#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// PINS
const int force1dout  = 4; //mcu > HX711 no 1 dout pin
const int force1sck   = 5; //mcu > HX711 no 1 sck pin
const int force2dout  = 6; //mcu > HX711 no 2 dout pin
const int force2sck   = 7; //mcu > HX711 no 2 sck pin
const int acc1X = A0;
const int acc1Y = A1;
const int acc1Z = A2;
const int acc2X = A3;
const int acc2Y = A4;
const int acc2Z = A5;

const int RawMin = 0;
const int RawMax = 1023;

const int sampleSize = 10;  // size of the measurement sample

//HX711 constructor
HX711_ADC LoadCell_1(force1dout, force1sck); //HX711 1
HX711_ADC LoadCell_2(force2dout, force2sck); //HX711 2

// EEPROM adress for calibration value (4 bytes)
const int calVal_eepromAdress_1 = 0;
const int calVal_eepromAdress_2 = 4;
unsigned long t = 0;

void setup() {
  Serial.begin(57600); delay(10);
  //Serial.println();
  //Serial.println("Starting...");

  // LOAD CELLS
  ///////////////////////////////////////////////////////////////
  float calibrationValue_1; // calibration value load cell 1
  float calibrationValue_2; // calibration value load cell 2

  calibrationValue_1 = 696.0; // uncomment this if you want to set this value in the sketch
  calibrationValue_2 = 733.0; // uncomment this if you want to set this value in the sketch
#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch the value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress_1, calibrationValue_1); // uncomment this if you want to fetch the value from eeprom
  //EEPROM.get(calVal_eepromAdress_2, calibrationValue_2); // uncomment this if you want to fetch the value from eeprom

  LoadCell_1.begin();
  LoadCell_2.begin();
  //LoadCell_1.setReverseOutput();
  //LoadCell_2.setReverseOutput();

  // TARE
  unsigned long stabilizingtime = 4000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { //run startup, stabilization and tare, both modules simultaniously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
  }
  LoadCell_1.setCalFactor(calibrationValue_1); // user set calibration value (float)
  LoadCell_2.setCalFactor(calibrationValue_2); // user set calibration value (float)
  //Serial.println("Startup is complete");


  // ACCELEROMETERS
  ///////////////////////////////////////////////////////////////

  // minimum and maximum ranges for each axis
  const int RawMin = 0;
  const int RawMax = 1023;

  const int SampleSize = 10;  // size of the measurement sample

  analogReference(EXTERNAL);
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell_1.update()) newDataReady = true;
  LoadCell_2.update();  

  
  //////////////////////////
  // SERIAL PRINT
  // get smoothed value from data set
  if ((newDataReady)) {
    if (millis() > t + serialPrintInterval) {
      // UPDATE LOAD CELLS
      float force1 = LoadCell_1.getData();
      float force2 = LoadCell_2.getData();

      // UPDATE ACCELEROMETERS
      // read raw values - ReadAxis() takes 10 values and returns the average
      int xRaw1 = ReadAxis(acc1X);
      int yRaw1 = ReadAxis(acc1Y);
      int zRaw1 = ReadAxis(acc1Z);
      int Raw1[3] = {ReadAxis(acc1X), ReadAxis(acc1Y), ReadAxis(acc1Z)};
    
      // converting values to acceleration
      long xScaled1 = map(xRaw1, RawMin, RawMax, -3000, 3000);
      long yScaled1 = map(yRaw1, RawMin, RawMax, -3000, 3000);
      long zScaled1 = map(zRaw1, RawMin, RawMax, -3000, 3000);
      long Scaled1[3];
        for(int i=0; i<=2; i++){
          Scaled1[i] = map(Raw1[i], RawMin, RawMax, -3000, 3000);
        }
    
      // converting accelerations to fractional Gs
      float xAccel1 = xScaled1 / 1000.0;
      float yAccel1 = yScaled1 / 1000.0;
      float zAccel1 = zScaled1 / 1000.0;
      float Accel1[3];
      for(int i=0; i<=2; i++){
        Accel1[i] = Scaled1[i] / 1000.0;
      }
      
      Serial.print(force1);
      Serial.print(",");
      Serial.println(force2);
      Serial.print(xAccel1);
      Serial.print(",");
      Serial.print(yAccel1);
      Serial.print(",");
      Serial.println(zAccel1);
      newDataReady = 0;
      t = millis();
    }
  }

  //////////////////////////
  // TARE OPERATION
  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
    }
  }
  //check if last tare operation is complete
  if (LoadCell_1.getTareStatus() == true) {
    Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare load cell 2 complete");
  }
}


// takes samples and return the average
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++){
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}
