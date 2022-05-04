# GyverBME280
Lightweight BME280 I2C library for Arduino

### Compatibility
Compatible with all Arduino platforms (using Arduino functions)

## Content
- [Install](#install)
- [Initialization](#init)
- [Usage](#usage)
- [Example](#example)
- [Versions](#versions)
- [Bugs and feedback](#feedback)

<a id="install"></a>
## Installation
- The library can be found by the name **GyverBME280** and installed via the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download library](https://github.com/GyverLibs/GyverBME280/archive/refs/heads/main.zip) .zip archive for manual installation:
    - Unzip and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unzip and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/*
    - (Arduino IDE) automatic installation from .zip: *Sketch/Include library/Add .ZIP library…* and specify the downloaded archive
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE% D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

<a id="init"></a>
## Initialization
```cpp
GyverBME280 bme;
```

<a id="usage"></a>
## Usage
```cpp
boolbegin(void); // run with standard addresscatfish (0x76)
bool begin(uint8_t address); // run with address
bool isMeasuring(void); // returns true while the measurement is in progress
float readPressure(void); // read pressure in Pa
float readHumidity(void); // Read humidity in %
void oneMeasurement(void); // Take one measurement and go to sleep
float readTemperature(void); // Read temperature in degrees C

void setMode(uint8_tmode); // set the mode of operation
// modes:
NORMAL_MODE
FORCED_MODE

void setFilter(uint8_tmode); // change the filter factor. Call before begin
// coefficients:
FILTER_DISABLE
FILTER_COEF_2
FILTER_COEF_4
FILTER_COEF_8
FILTER_COEF_16

void setStandbyTime(uint8_tmode); // Change time between measurements. Call before begin
// modes:
STANDBY_500US
STANDBY_10MS
STANDBY_20MS
STANDBY_6250US
STANDBY_125MS
STANDBY_250MS
STANDBY_500MS
STANDBY_1000MS

void setHumOversampling(uint8_t mode); // Set up oversampling or disable humidity. Call before begin
void setTempOversampling(uint8_t mode); // Set up oversampling or disable temperature. Call before begin
void setPressOversampling(uint8_t mode); // Set up oversampling or disable pressure. Call before begin
// modes:
MODULE_DISABLE
OVERSAMPLING_1
OVERSAMPLING_2
OVERSAMPLING_4
OVERSAMPLING_8
OVERSAMPLING_16
```

<a id="example"></a>
## Example
See **examples** for other examples!
```cpp
/*
   A simple example showing the basic functions of measuring temperature, pressure and humidity
*/

#include <GyverBME280.h> // Including the library
GyverBME280 bme; // Create bme object

void setup() {
  Serial.begin(9600); // Start serial port
  bme.begin(); // If add. settings are not needed - initialize the sensor
}

void loop() {Serial.print("Temperature: ");
  Serial.print(bme.readTemperature()); // Print temperature in [*C]
  Serial.println("*C");

  Serial.print("Humidity: ");
  Serial.print(bme.readHumidity()); // Output humidity in [%]
  Serial.println("%");

  float pressure = bme.readPressure(); // Read pressure in [Pa]
  Serial.print("Pressure: ");
  Serial.print(pressure / 100.0F); // Display pressure in [hPa]
  Serial.print(" hPa , ");
  Serial.print(pressureToMmHg(pressure)); // Output pressure in [mmHg. pillar]
  Serial.println("mm hg");
  Serial.print("Altide: ");
  Serial.print(pressureToAltitude(pressure)); // Print height in [m a.s.l. seas]
  Serial.println("m");
  Serial.println("");
  delay(1000);
}
```

<a id="versions"></a>
## Versions
- v1.3 - fixed bug when negative. temperature
- v1.4 - split into h and cpp
- v1.5 - added support for BMP280

<a id="feedback"></a>
## Bugs and feedback
When you find bugs, create an **Issue**, or better, immediately write to the mail [alex@alexgyver.ru](mailto:alex@alexgyver.ru)
The library is open for revision and your **Pull Request**'s!