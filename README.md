# Arduino Measurements
This repository aims to set up an Arduino environment to carry out several simultaneous force and acceleration measurements.
The data will then be postprocessed with Python and stored locally.

## 1. Hardware setup
### 1.1 Force sensors
Initially, two load modules of 10kg and 20kg each have been used for testing and configuration purposes. Each set comes with the load cell itself, and also an HX-711 amplifier module, which provides the measured values in a readable scale for Arduino.

![LoadCell & HX-711](Sketches/LoadCell.jpg)

The connections between the different components have been carried out as follows:

![LoadCell connections](Sketches/LoadCell-diagram.webp)

*Source: [circuitjournal.com](https://circuitjournal.com/four-wire-load-cell-with-HX711#arduino-code)*

A+ and A- wires can be interchanged, since this would only affect the sign of the output value. DT and SCK pins have to be connected to digital pins on the Arduino board.

The pins have been soldered to the HX-711 amplifier module to make easier but solid connections with the wires.

### 1.2 Accelerometers

Apart from a 5V power supply, each accelerometer requires connection with 3 analogue pins. No amplifier module is needed in this case.

![Connection Accelerometer to Arduino](Sketches/Accelerometer-to-Arduino.png)

*Source: lastminuteengineers.com*

As well as with the HX-711 amplifier modules, the pins have been soldered to the accelerometers.

## 2. Arduino software setup
The code that was loaded on the Arduino board (`Read_load_accelerometer.ino`) has been constructed based on two independent pieces of code:
- The base code for the load cells (`Read_1x_load_cell.ino`) was obtained from [Olav Kallhovd (GitHub)](https://github.com/olkal/HX711_ADC). Apart from reading, processing and filtering the measurements, the package also features an option to tare the measuring equipment. Also, running the `Calibrate.ino` file generates a calibration value which can then be used in the main code.
- The base code for the accelerometers (`Read_accelerometer.ino`) was obtained from [LastMinuteEngineers](https://lastminuteengineers.com/adxl335-accelerometer-arduino-tutorial/). The main feature is the use of a function (`int ReadAxis()`) that performs 10 independent measurements and averages them before providing a value.

#### Measurements
The board has been programmed according to the following sequence:

- First, it waits until new measurement data from the load cells are available.
- Then, if enough time has passed since the last serial print, the measurement process of the accelerometers is triggered.
- Once the acceleration data are available, they are postprocessed and translated into acceleration in G's.

Every set of measurements will be composed of one value per load cell and three values for every accelerometer.

#### Transmission over Serial port

Every set of simultaneous measurements are transmitted in a single line, separated by commas (','). 

## 3. Python postprocessing
The operations detailed below have been implemented in the following file: `Postprocessing` / `serial_connection.py`

The Python 'serial' module enables to read the incoming data over the Serial port. 

Each new set of measurements must first be decoded, separated into individual values and turned into 'float' variables. Then, they are stored as a list and they can be used to perform the desired operations.

Also, a variable of interest would be the frequency at which new data are obtained.


