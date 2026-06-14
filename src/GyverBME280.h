/*
    Лёгкая библиотека для работы с BME280 по I2C для Arduino
    Документация:
    GitHub: https://github.com/GyverLibs/GyverBME280

    Egor 'Nich1con' Zaharov for AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.3 - исправлена ошибка при отриц. температуре
    v1.4 - разбил на h и cpp
    v1.5 - добавлена поддержка BMP280
*/

#ifndef GyverBME280_h
#define GyverBME280_h

#include <Arduino.h>
#include <Wire.h>
#include <cmath>

#define NORMAL_MODE 0x03
#define FORCED_MODE 0x02

#define STANDBY_500US 0x00
#define STANDBY_10MS 0x06
#define STANDBY_20MS 0x07
#define STANDBY_6250US 0x01
#define STANDBY_125MS 0x02
#define STANDBY_250MS 0x03
#define STANDBY_500MS 0x04
#define STANDBY_1000MS 0x05

#define MODULE_DISABLE 0x00
#define OVERSAMPLING_1 0x01
#define OVERSAMPLING_2 0x02
#define OVERSAMPLING_4 0x03
#define OVERSAMPLING_8 0x04
#define OVERSAMPLING_16 0x05

#define FILTER_DISABLE 0x00
#define FILTER_COEF_2 0x01
#define FILTER_COEF_4 0x02
#define FILTER_COEF_8 0x03
#define FILTER_COEF_16 0x04

#define MODE_DATA_DEFAULT  0x00
#define MODE_DATA_LAST_VALID  0x01
#define MODE_DATA_INTERPOLATED_LINE  0x02
#define MODE_DATA_INTERPOLATED_GAUS  0x03

// ================================= CLASS ===================================

class GyverBME280 {
   public:
    GyverBME280();                // Create an object of class BME280
    bool begin(void);             // Initialize sensor with standart 0x76 address
    bool begin(uint8_t address);  // Initialize sensor with not standart 0x76 address
    bool isMeasuring(void);       // Returns 'true' while the measurement is in progress
    float readPressure(void);     // Read and calculate atmospheric pressure [float , Pa]
    float readHumidity(void);     // Read and calculate air humidity [float , %]
    void oneMeasurement(void);    // Make one measurement and go back to sleep [FORCED_MODE only]
    void setMode(uint8_t mode);
    float readTemperature(void);              // Read and calculate air temperature [float , *C]
    void setFilter(uint8_t mode);             // Adjust the filter ratio other than the standard one [before begin()]
    void setStandbyTime(uint8_t mode);        // Adjust the sleep time between measurements [NORMAL_MODE only][before begin()]
    void setHumOversampling(uint8_t mode);    // Set oversampling or disable humidity module [before begin()]
    void setTempOversampling(uint8_t mode);   // Set oversampling or disable temperature module [before begin()]
    void setPressOversampling(uint8_t mode);  // Set oversampling or disable pressure module [before begin()]
    void setDataMode(uint8_t mode);           // Set the data mode for the humidity, temperature and pressure
    void setAlpha(float alpha);               // Set the alpha value for the humidity, temperature and pressure
    void initInterpolateData();               // Initialize the interpolate data

   private:
    //============================== DEFAULT SETTINGS ========================================|
    int _i2c_address = 0x76;                    // BME280 address on I2C bus                  |
    uint8_t _operating_mode = NORMAL_MODE;      // Sensor operation mode                      |
    uint8_t _standby_time = STANDBY_250MS;      // Time between measurements in NORMAL_MODE   |
    uint8_t _filter_coef = FILTER_COEF_16;      // Filter ratio IIR                           |
    uint8_t _temp_oversampl = OVERSAMPLING_4;   // Temperature module oversampling parameter  |
    uint8_t _hum_oversampl = OVERSAMPLING_1;    // Humidity module oversampling parameter     |
    uint8_t _press_oversampl = OVERSAMPLING_2;  // Pressure module oversampling parameter     |
    uint8_t _data_mode = MODE_DATA_DEFAULT;     // Data output mode: default (0x00)           |
    float _alpha = 0.5f;                        // Alpha for initialization                   |
    //========================================================================================|

    bool reset(void);                                   // BME280 software reset
    int32_t readTempInt();                              // Temperature reading in integers for the function of reading
    void readCalibrationData(void);                     // Read all cells containing calibration values
    uint8_t readRegister(uint8_t address);              // Read one 8-bit BME280 register
    uint32_t readRegister24(uint8_t address);           // Read and combine three BME280 registers
    bool writeRegister(uint8_t address, uint8_t data);  // Write one 8-bit BME280 register
    void updateInterpolateData(float newValue, float* dataArray) {};     // Update interpolation parameters float
    void updateInterpolateData(int32_t newValue, int32_t* dataArray) {}; // Update interpolation parameters int32_t
    float linear_interpolate(float* dataArray) {};
    int32_t linear_interpolate(int32_t* dataArray) {};
    float gaussian_interpolate(float* dataArray) {};
    int32_t gaussian_interpolate(int32_t* dataArray) {};

    struct {  // Structure to store all calibration values
        uint16_t _T1;
        int16_t _T2;
        int16_t _T3;
        uint16_t _P1;
        int16_t _P2;
        int16_t _P3;
        int16_t _P4;
        int16_t _P5;
        int16_t _P6;
        int16_t _P7;
        int16_t _P8;
        int16_t _P9;
        uint8_t _H1;
        int16_t _H2;
        uint8_t _H3;
        int16_t _H4;
        int16_t _H5;
        int8_t _H6;
    } CalibrationData;
    
    struct {  // A structure for storing the last two correct values used for interpolation
        int32_t temperature[2];     // The last two correct temperature values
        float pressure[2];    // The last two correct pressure values
        float humidity[2];    // The last two correct humidity values
    } InterpolateData;
    
};

float pressureToMmHg(float pressure);      // Convert [Pa] to [mm Hg]
float pressureToAltitude(float pressure);  // Convert pressure to altitude
#endif