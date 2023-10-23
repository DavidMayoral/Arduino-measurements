#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <Wire.h>  // Wire library - used for I2C communication

// PINS
const int force1dout  = 2; //mcu > HX711 no 1 dout pin
const int force1sck   = 4; //mcu > HX711 no 1 sck pin
const int force2dout  = 3; //mcu > HX711 no 2 dout pin
const int force2sck   = 5; //mcu > HX711 no 2 sck pin

//HX711 constructortau
HX711_ADC LoadCell_1(force1dout, force1sck); //HX711 1
HX711_ADC LoadCell_2(force2dout, force2sck); //HX711 2

// EEPROM adress for calibration value (4 bytes)
const int calVal_eepromAdress_1 = 0;
const int calVal_eepromAdress_2 = 4;

unsigned long t = 0;
float startTimer = 0;
float lastPrint = 0;
volatile boolean newDataReady;

// ACCELEROMETER ADXL345
int ADXL345 = 0x53; // The ADXL345 sensor I2C address

float accCalibration = 1;
const int sampleSize = 10;  // size of the measurement sample

// SERIAL MONITOR
int counter = 0;
const int n = 100;       // number of measurements until values are printed
int timesTimed = 0;
int timesChecked = 0;
String data[n];
float timer[n];         // time different processes to check delays
unsigned long timer2 = 0.;


// *****************************************
//                    SETUP
// *****************************************

void setup() {
  Serial.begin(57600); delay(10);
  //Serial.println();
  //Serial.println("Starting...");

  // LOAD CELLS
  //////////////////////////////////////////////////
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
  /////////////////////////////////////////////////////
  float X_raw, Y_raw, Z_raw;  
  long Raw1[3];                  // Acceleration outputs
  float X_acc, Y_acc, Z_acc;  // Acceleration scaled in G

  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();
  delay(10);
 
  // Attaching interrupt in the DOUT pins in order to achieve maximum effective sample rate
  attachInterrupt(digitalPinToInterrupt(force1dout), dataReadyISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(force2dout), dataReadyISR, FALLING);
}



//interrupt routine:
void dataReadyISR() {
  if (LoadCell_1.update()) newDataReady = true;
  LoadCell_2.update(); 
}

// *****************************************
//                    LOOP
// *****************************************

void loop() {
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
  //if (LoadCell_1.update()) newDataReady = true;
  //timesChecked++;
  /*
  if (digitalRead(force1dout) == 0 | digitalRead(force2dout) == 0){
    timesTimed++;
    timer2 += (millis() - startTimer);
  }
  */
  
  if (newDataReady) {
    timer[counter] = millis() - startTimer;
    if (millis() > t + serialPrintInterval) {
      
      // UPDATE LOAD CELLS
      float force1 = LoadCell_1.getData();
      float force2 = LoadCell_2.getData();
      //Serial.print("[T]getData duration: ");
      //Serial.println(millis() - startGetData);
      
      // UPDATE ACCELEROMETERS
      /*
      // read raw values - ReadAccel() takes 10 values and returns the average
      float Raw1[3];
      Raw1 = ReadAccel(sampleSize);
      ReadAccel(Raw1, sampleSize);

      
    // scaling the raw acceleration values
      float X_acc = Raw1[0]/256 * accCalibration; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
      float Y_acc = Raw1[1]/256 * accCalibration;
      float Z_acc = Raw1[2]/256 * accCalibration;
      */
      
      Wire.beginTransmission(ADXL345);
      Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(ADXL345, 6, true);

      float X_acc = ( Wire.read()| Wire.read() << 8); // X-axis value
      float Y_acc = ( Wire.read()| Wire.read() << 8); // Y-axis value
      float Z_acc = ( Wire.read()| Wire.read() << 8); // Z-axis value
      X_acc = X_acc/256 * accCalibration; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
      Y_acc = Y_acc/256 * accCalibration;
      Z_acc = Z_acc/256 * accCalibration;
      float acc = sqrt(pow(X_acc,2) + pow(Y_acc,2) + pow(Z_acc,2));

      //Serial.print("[T]Acc measurement duration: ");
      //Serial.println(millis() - startAccMeas);

      newDataReady = 0;
      
      /*
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
      */
      
      //////////////////////////
      // SERIAL PRINT
      
      data[counter] = String(force1) + "," + String(force2) + "," + String(X_acc) + "," + String(Y_acc) + "," + String(Z_acc) + "," + String(acc);
      /*
      data[counter] = force1;
      data[counter].append(",");
      data[counter].append(force2);
      data[counter].append(",");
      data[counter].append(X_acc);
      data[counter].append(",");
      data[counter].append(Y_acc);
      data[counter].append(",");
      data[counter].append(Z_acc);
      data[counter].append(",");
      data[counter].append(acc);
      */
      
      Serial.print(force1);
      Serial.print(",");
      Serial.print(force2);
      Serial.print(",");
      
      Serial.print(X_acc);
      Serial.print(",");
      Serial.print(Y_acc);
      Serial.print(",");
      Serial.println(Z_acc);
      //Serial.print(",");
      //Serial.println(acc);
      
      t = millis();
      startTimer = millis();    
    }
    /*
    counter += 1;
    if (counter == n) {
      counter = 0;
      float sum = 0;
      for (int i=0; i<n; i++) sum += timer[i];
      double freq = 1000.*n / (millis()-lastPrint);
      Serial.print("[TIMER] ");
      Serial.println(sum/n);
      Serial.print("[TIMER2] ");
      Serial.println(timer2/timesTimed);
      Serial.print("[TIMES TIMED] ");
      Serial.println(timesTimed);
      //Serial.print("[TIMES CHECKED] ");
      //Serial.println(timesChecked);
      Serial.print("[t=");
      Serial.print(millis());
      Serial.print("] ");
      Serial.println(1000./freq);
      Serial.print("[FREQ] ");
      Serial.println(freq);
      Serial.print("[DATA] ");
      Serial.println(data[n-1]);
      lastPrint = millis();
      for (int i=0; i<n; i++) {
        //Serial.println(data[i]);
        data[i] = "";
      timer2 = 0;
      timesTimed = 0;
      timesChecked = 0;
      }
    }
    */
    startTimer = millis();
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
    //Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus() == true) {
    //Serial.println("Tare load cell 2 complete");
  }
  
}


// takes samples and returns the average
void ReadAccel(float* output, int sampleSize){
  float X_out = 0;
  float Y_out = 0;
  float Z_out = 0;

  for (int i = 0; i < sampleSize; i++){
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    delay(1);
    
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    X_out += ( Wire.read()| Wire.read() << 8); // X-axis value
    Y_out += ( Wire.read()| Wire.read() << 8); // Y-axis value
    Z_out += ( Wire.read()| Wire.read() << 8); // Z-axis value
  }
  //static long output[3];
  output[0] = X_out / sampleSize;
  output[1] = Y_out / sampleSize;
  output[2] = Z_out / sampleSize;  
  //return output;
}
