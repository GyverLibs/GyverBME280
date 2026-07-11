This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverBME280.svg?color=brightgreen)](https://github.com/GyverLibs/GyverBME280/releases/latest/download/GyverBME280.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverBME280.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverBME280)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverBME280?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverBME280
Lightweight library to work with BME280 over I2C for Arduino

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)

## Contents
- [Installation](#install)
- [Initialization](#init)
- [Use of use](#usage)
- [Example](#example)
- [Versions](#versions)
- [Bugs and feedback](#feedback)

<a id="install"></a>
## Installation
- The library can be found under the name **GyverBME280** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/GyverBME280/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="init"></a>
## Initialization
```cpp
GyverBME280 bme;
```

<a id="usage"></a>
## Use of use
```cpp
bool begin();                               // Start with a standard address (0x76)
bool begin(uint8_t address);                // launch
bool isMeasuring();                         // It returns true while the measurement is in progress.
float readPressure();                       // pressurize
float readHumidity();                       // Read humidity in %
void oneMeasurement();                      // Take one dimension and go to sleep
float readTemperature();                    // Read the temperature in degrees C

void setMode(uint8_t mode);                 // set up
// regimes
NORMAL_MODE
FORCED_MODE

void setFilter(uint8_t mode);               // Change the filtration factor. Call before start
// factors:
FILTER_DISABLE
FILTER_COEF_2
FILTER_COEF_4
FILTER_COEF_8
FILTER_COEF_16

void setStandbyTime(uint8_t mode);          // Change the time between measurements. Call before start
// regimes
STANDBY_500US
STANDBY_10MS
STANDBY_20MS
STANDBY_6250US
STANDBY_125MS
STANDBY_250MS
STANDBY_500MS
STANDBY_1000MS

void setHumOversampling(uint8_t mode);      // Set up oversampling or turn off humidity. Call before start
void setTempOversampling(uint8_t mode);     // Set up oversampling or turn off the temperature. Call before start
void setPressOversampling(uint8_t mode);    // Set up oversampling or turn off pressure. Call before start
// regimes
MODULE_DISABLE
OVERSAMPLING_1
OVERSAMPLING_2
OVERSAMPLING_4
OVERSAMPLING_8
OVERSAMPLING_16
```

<a id="example"></a>
## Example
For more examples see **examples**!
```cpp
/*
   Простой пример, демонстрирующий основные функции измерения температуры, давления и влажности
*/

#include <GyverBME280.h>                      // Connecting the library
GyverBME280 bme;                              // Creation of the bme object

void setup() {
  Serial.begin(9600);                         // Starting a serial port
  bme.begin();                                // If additional settings are not needed - initialize the sensor
}

void loop() {
  Serial.print("Temperature: ");
  Serial.print(bme.readTemperature());        // Reduce the temperature to [*C]
  Serial.println(" *C");

  Serial.print("Humidity: ");
  Serial.print(bme.readHumidity());           // Reduce humidity in [%]
  Serial.println(" %");

  float pressure = bme.readPressure();        // Read the pressure in [Pa]
  Serial.print("Pressure: ");
  Serial.print(pressure / 100.0F);            // Put pressure in [gPa]
  Serial.print(" hPa , ");
  Serial.print(pressureToMmHg(pressure));     // Put pressure in [mmHg column]
  Serial.println(" mm Hg");
  Serial.print("Altitide: ");
  Serial.print(pressureToAltitude(pressure)); // We will bring the height to the sea.
  Serial.println(" m");
  Serial.println("");
  delay(1000);
}
```

<a id="versions"></a>
## Versions
- v1.3 - corrected error in denials. temperature
- v1.4 - split into h and cpp
- v1.5 - BMP280 support added

<a id="feedback"></a>
## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.
