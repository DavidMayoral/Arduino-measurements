/*
    Arduino and ADXL345 Accelerometer Tutorial
     by Dejan, https://howtomechatronics.com
*/

#include <Wire.h>  // Wire library - used for I2C communication

int ADXL345 = 0x53; // The ADXL345 sensor I2C address

float X_out, Y_out, Z_out;  // Outputs
float calibrationVal = 1;

// SERIAL MONITOR
int counter = 0;
const int n = 500;       // number of measurements until values are printed
int timesTimed = 0;
float lastPrint = 0;
float frequency;


void setup() {
  Serial.begin(57600); // Initiate serial communication 
  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();
  delay(10);
}

void loop() {  
  // === Read acceleromter data === //
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
  X_out = X_out/256 * calibrationVal; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
  Y_out = Y_out/256 * calibrationVal;
  Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
  Z_out = Z_out/256 * calibrationVal;
/*
  Serial.print("Xa= ");
  Serial.print(X_out);
  Serial.print("   Ya= ");
  Serial.print(Y_out);
  Serial.print("   Za= ");
  Serial.print(Z_out);

  frequency = 1000. / (millis()-lastPrint);
  lastPrint = millis();
  
  Serial.print("   Freq= ");
  Serial.println(frequency);
  */
  //delay();
  
  counter ++;
  if (counter == n) {
    counter = 0;
    double freq = 1000.*n / (millis()-lastPrint);

    Serial.print("[t=");
    Serial.print(millis());
    Serial.print("] ");
    Serial.println(1000./freq);
    Serial.print("Xa= ");
    Serial.print(X_out);
    Serial.print("   Ya= ");
    Serial.print(Y_out);
    Serial.print("   Za= ");
    Serial.println(Z_out);
    Serial.print("[FREQ] ");
    Serial.println(freq);
    
    lastPrint = millis();
    }
  
}
