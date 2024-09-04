#include "GyverBME280.h"

/* ============ Utilities ============ */

float pressureToAltitude(float pressure) {
    if (!pressure) return 0;                                   // If the pressure module has been disabled return '0'
    // "sea-level" pressure in Pa (within 10% of 101325)
    // https://en.wikipedia.org/wiki/Atmospheric_pressure#Mean_sea-level_pressure
    return (float)44330.77 * ((float)1.0 - pow((float)pressure / (float)REF_PRESSURE, 0.190263));  // Сalculate altitude
}

float pressureToMmHg(float pressure) {
    return (float)(pressure * 0.00750061683f);  // Convert [Pa] to [mm Hg]
}

/* ============ Setup & begin ============ */

bool GyverBME280::begin(TwoWire &wirePort) {
    return begin(_i2c_address,wirePort);
}

bool GyverBME280::begin(uint8_t address, TwoWire &wirePort) {
    _i2c_address = address;
    // We use an I2C Wire bus already created.
    _wire = &wirePort;
    if (!reset()) return false;  // BME280 software reset & ack check
    uint8_t ID = readRegister(BME280_CHIPID);
    if (ID != 0x60 && ID != 0x58) return false;  // Check chip ID (bme/bmp280)
    readCalibrationData();                       // Read all calibration values

    /* === Load settings to BME280 === */
    writeRegister(BME280_CONTROLHUMID, _hum_oversampl);                                                        // write hum oversampling value
    writeRegister(BME280_CONTROLHUMID, readRegister(BME280_CONTROLHUMID));                                                    // rewrite hum oversampling register
    writeRegister(BME280_CONTROL, ((_temp_oversampl << 5) | (_press_oversampl << 2) | _operating_mode));  // write temp & press oversampling value , normal mode
    writeRegister(BME280_CONFIG, ((_standby_time << 5) | (_filter_coef << 2)));                          // write standby time & filter coef
    return true;
}

void GyverBME280::setMode(uint8_t value) {
    _operating_mode = value;
}

void GyverBME280::setFilter(uint8_t value) {
    _filter_coef = value;
}

void GyverBME280::setStandbyTime(uint8_t value) {
    _standby_time = value;
}

void GyverBME280::setHumOversampling(uint8_t value) {
    _hum_oversampl = value;
}

void GyverBME280::setTempOversampling(uint8_t value) {
    _temp_oversampl = value;
}

void GyverBME280::setPressOversampling(uint8_t value) {
    _press_oversampl = value;
}

/* ============ Reading ============ */

int32_t GyverBME280::readTempInt(void) {
    int32_t temp_raw = readRegister24(BME280_TEMPDATA);  // Read 24-bit value
    if (temp_raw == 0x800000) return 0;       // If the temperature module has been disabled return '0'

    temp_raw >>= 4;  // Start temperature reading in integers
    int32_t value_1 = ((((temp_raw >> 3) - ((int32_t)CalibrationData._T1 << 1))) * ((int32_t)CalibrationData._T2)) >> 11;
    int32_t value_2 = (((((temp_raw >> 4) - ((int32_t)CalibrationData._T1)) * ((temp_raw >> 4) - ((int32_t)CalibrationData._T1))) >> 12) * ((int32_t)CalibrationData._T3)) >> 14;
    return ((int32_t)value_1 + value_2);  // Return temperature in integers
}

float GyverBME280::readTemperature(void) {
    int32_t temp_raw = readTempInt();
    float T = (temp_raw * 5 + 128) >> 8;
    return T / 100.0;  // Return temperature in float
}

float GyverBME280::readPressure(void) {
    uint32_t press_raw = readRegister24(BME280_PRESSUREDATA);  // Read 24-bit value
    if (press_raw == 0x800000) return 0;        // If the pressure module has been disabled return '0'

    press_raw >>= 4;  // Start pressure converting
    int64_t value_1 = ((int64_t)readTempInt()) - 128000;
    int64_t value_2 = value_1 * value_1 * (int64_t)CalibrationData._P6;
    value_2 = value_2 + ((value_1 * (int64_t)CalibrationData._P5) << 17);
    value_2 = value_2 + (((int64_t)CalibrationData._P4) << 35);
    value_1 = ((value_1 * value_1 * (int64_t)CalibrationData._P3) >> 8) + ((value_1 * (int64_t)CalibrationData._P2) << 12);
    value_1 = (((((int64_t)1) << 47) + value_1)) * ((int64_t)CalibrationData._P1) >> 33;

    if (!value_1) return 0;  // Avoid division by zero

    int64_t p = 1048576 - press_raw;
    p = (((p << 31) - value_2) * 3125) / value_1;
    value_1 = (((int64_t)CalibrationData._P9) * (p >> 13) * (p >> 13)) >> 25;
    value_2 = (((int64_t)CalibrationData._P8) * p) >> 19;
    p = ((p + value_1 + value_2) >> 8) + (((int64_t)CalibrationData._P7) << 4);

    return (float)p / 256;  // Return pressure in float
}

float GyverBME280::readHumidity(void) {
    _wire->beginTransmission(_i2c_address);  // Start I2C transmission
    _wire->write(BME280_HUMIDDATA);          // Request humidity data register
    if (_wire->endTransmission() != 0) return 0;
    _wire->requestFrom(_i2c_address, 2);                                       // Request humidity data
    int32_t hum_raw = ((uint16_t)_wire->read() << 8) | (uint16_t)_wire->read();  // Read humidity data
    if (hum_raw == 0x8000) return 0;                                         // If the humidity module has been disabled return '0'

    int32_t value = (readTempInt() - ((int32_t)76800));  // Start humidity converting
    value = (((((hum_raw << 14) - (((int32_t)CalibrationData._H4) << 20) - (((int32_t)CalibrationData._H5) * value)) + ((int32_t)16384)) >> 15) * (((((((value * ((int32_t)CalibrationData._H6)) >> 10) * (((value * ((int32_t)CalibrationData._H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)CalibrationData._H2) + 8192) >> 14));
    value = (value - (((((value >> 15) * (value >> 15)) >> 7) * ((int32_t)CalibrationData._H1)) >> 4));
    value = (value < 0) ? 0 : value;
    value = (value > 419430400) ? 419430400 : value;
    float h = (value >> 12);

    return h / 1024.0;  // Return humidity in float
}

/* ============ Misc ============ */

bool GyverBME280::isMeasuring(void) {
    return (bool)((readRegister(BME280_STATUS) & 0x08) >> 3);  // Read status register & mask bit "measuring"
}

void GyverBME280::oneMeasurement(void) {
    writeRegister(BME280_CONTROL, ((readRegister(BME280_CONTROL) & 0xFC) | FORCED_MODE));  // Set the operating mode to FORCED_MODE
}

void GyverBME280::triggerMeasurement(void) {
    oneMeasurement();
    // wait for it to complete
    while( isMeasuring() == false); // Wait for sensor to start measurment
    while( isMeasuring() == true); // Hang out while sensor completes the reading    
}

GyverBME280::GyverBME280() {}

/* ============ Private ============ */

/* = BME280 software reset = */
bool GyverBME280::reset(void) {
    if (!writeRegister(BME280_SOFTRESET, 0xB6)) return false;
    delay(10);
    return true;
}

/* = Read and combine three BME280 registers = */
uint32_t GyverBME280::readRegister24(uint8_t address) {
    _wire->beginTransmission(_i2c_address);
    _wire->write(address);
    if (_wire->endTransmission() != 0) return 0x800000;
    _wire->requestFrom(_i2c_address, 3);
    return (((uint32_t)_wire->read() << 16) | ((uint32_t)_wire->read() << 8) | (uint32_t)_wire->read());
}

/* = Write one 8-bit BME280 register = */
bool GyverBME280::writeRegister(uint8_t address, uint8_t data) {
    _wire->beginTransmission(_i2c_address);
    _wire->write(address);
    _wire->write(data);
    if (_wire->endTransmission() != 0) return false;
    return true;
}

/* = Read one 8-bit BME280 register = */
uint8_t GyverBME280::readRegister(uint8_t address) {
    _wire->beginTransmission(_i2c_address);
    _wire->write(address);
    if (_wire->endTransmission() != 0) return 0;
    _wire->requestFrom(_i2c_address, 1);
    return _wire->read();
}

/* = Structure to store all calibration values = */
void GyverBME280::readCalibrationData(void) {
    /* first part request*/
    _wire->beginTransmission(_i2c_address);
    _wire->write(BME280_CALIB_DATA1);
    if (_wire->endTransmission() != 0) return;
    _wire->requestFrom(_i2c_address, 25);
    /* reading */
    CalibrationData._T1 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._T2 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._T3 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P1 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P2 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P3 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P4 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P5 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P6 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P7 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P8 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._P9 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._H1 = _wire->read();

    /* second part request*/
    _wire->beginTransmission(_i2c_address);
    _wire->write(BME280_CALIB_DATA2);
    _wire->endTransmission();
    _wire->requestFrom(_i2c_address, 8);
    /* reading */
    CalibrationData._H2 = (_wire->read() | (_wire->read() << 8));
    CalibrationData._H3 = _wire->read();
    CalibrationData._H4 = (_wire->read() << 4);
    uint8_t interVal = _wire->read();
    CalibrationData._H4 |= (interVal & 0xF);
    CalibrationData._H5 = (((interVal & 0xF0) >> 4) | (_wire->read() << 4));
    CalibrationData._H6 = _wire->read();
}