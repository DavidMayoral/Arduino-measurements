void setup() {
  const int acc1X = A0;
  const int acc1Y = A1;
  const int acc1Z = A2;

  // minimum and maximum ranges for each axis
  const int RawMin = 0;
  const int RawMax = 1023;

  const int SampleSize = 10;

  analogReference(EXTERNAL);
  Serial.begin(57600); delay(10);

}

void loop() {
  // read raw values - ReadAxis() takes 10 values and returns the average
  int xRaw1 = ReadAxis(acc1X);
  int yRaw1 = ReadAxis(acc1Y);
  int zRaw1 = ReadAxis(acc1Z);

  // converting values to acceleration
  long xScaled1 = map(xRaw1, RawMin, RawMax, -3000, 3000)
  long yScaled1 = map(yRaw1, RawMin, RawMax, -3000, 3000)
  long zScaled1 = map(zRaw1, RawMin, RawMax, -3000, 3000)

  // converting accelerations to fractional Gs
  float xAccel1 = xScaled1 / 1000.0;
  float yAccel1 = yScaled1 / 1000.0;
  float zAccel1 = zScaled1 / 1000.0;

  Serial.print(xAccel1);
  Serial.print(',');
  Serial.print(yAccel1);
  Serial.print((',');
  Serial.println(zAccel1);
  

}
