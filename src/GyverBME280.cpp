#include "GyverBME280.h"

/* ============ Utilities ============ */

float pressureToAltitude(float pressure) {
    if (!pressure) return 0;                                   // If the pressure module has been disabled return '0'
    pressure /= 100.0F;                                        // Convert [Pa] to [hPa]
    return 44330.0 * (1.0 - pow(pressure / 1013.25, 0.1903));  // Сalculate altitude
}

float pressureToMmHg(float pressure) {
    return (float)(pressure * 0.00750061683f);  // Convert [Pa] to [mm Hg]
}

/* ============ Setup & begin ============ */

bool GyverBME280::begin() {
    return begin(0x76);
}

bool GyverBME280::begin(uint8_t address) {
    _i2c_address = address;
    /* === Start I2C bus & check BME280 === */
    Wire.begin();                // Start I2C bus
    if (!reset()) return false;  // BME280 software reset & ack check
    uint8_t ID = readRegister(0xD0);
    if (ID != 0x60 && ID != 0x58) return false;  // Check chip ID (bme/bmp280)
    _hasHumidity = (ID == 0x60);
    if (!readCalibrationData()) return false;    // Read all calibration values

    /* === Load settings to BME280 === */
    if (_hasHumidity) writeRegister(0xF2, _hum_oversampl);                                      // write hum oversampling value
    writeRegister(0xF4, ((_temp_oversampl << 5) | (_press_oversampl << 2) | _operating_mode));  // write temp & press oversampling value , normal mode
    writeRegister(0xF5, ((_standby_time << 5) | (_filter_coef << 2)));                          // write standby time & filter coef
    return true;
}

void GyverBME280::setMode(uint8_t mode) {
    _operating_mode = mode;
}

void GyverBME280::setFilter(uint8_t mode) {
    _filter_coef = mode;
}

void GyverBME280::setStandbyTime(uint8_t mode) {
    _standby_time = mode;
}

void GyverBME280::setHumOversampling(uint8_t mode) {
    _hum_oversampl = mode;
}

void GyverBME280::setTempOversampling(uint8_t mode) {
    _temp_oversampl = mode;
}

void GyverBME280::setPressOversampling(uint8_t mode) {
    _press_oversampl = mode;
}

/* ============ Reading ============ */

int32_t GyverBME280::readTempInt() {
    int32_t temp_raw = readRegister24(0xFA);  // Read 24-bit value
    if (temp_raw == 0x800000) return 0;       // If the temperature module has been disabled return '0'

    temp_raw >>= 4;  // Start temperature reading in integers
    int32_t value_1 = ((((temp_raw >> 3) - ((int32_t)CalibrationData._T1 << 1))) * ((int32_t)CalibrationData._T2)) >> 11;
    int32_t value_2 = (((((temp_raw >> 4) - ((int32_t)CalibrationData._T1)) * ((temp_raw >> 4) - ((int32_t)CalibrationData._T1))) >> 12) * ((int32_t)CalibrationData._T3)) >> 14;
    return ((int32_t)value_1 + value_2);  // Return temperature in integers
}

float GyverBME280::readTemperature() {
    int32_t temp_raw = readTempInt();
    float T = (temp_raw * 5 + 128) >> 8;
    return T / 100.0;  // Return temperature in float
}

float GyverBME280::readPressure() {
    uint32_t press_raw = readRegister24(0xF7);  // Read 24-bit value
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

float GyverBME280::readHumidity() {
    if (!_hasHumidity) return 0;

    Wire.beginTransmission(_i2c_address);  // Start I2C transmission
    Wire.write(0xFD);                      // Request humidity data register
    if (Wire.endTransmission() != 0) return 0;
    if (Wire.requestFrom((uint8_t)_i2c_address, (uint8_t)2) != 2) return 0;  // Request humidity data
    int32_t hum_raw = ((uint16_t)readByte() << 8) | readByte();              // Read humidity data
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

bool GyverBME280::isMeasuring() {
    return (bool)((readRegister(0xF3) & 0x08) >> 3);  // Read status register & mask bit "measuring"
}

void GyverBME280::oneMeasurement() {
    writeRegister(0xF4, ((readRegister(0xF4) & 0xFC) | 0x02));  // Set the operating mode to FORCED_MODE
}

GyverBME280::GyverBME280() {}

/* ============ Private ============ */

/* = BME280 software reset = */
bool GyverBME280::reset() {
    if (!writeRegister(0xE0, 0xB6)) return false;
    delay(10);
    return true;
}

/* = Read and combine three BME280 registers = */
uint32_t GyverBME280::readRegister24(uint8_t address) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(address);
    if (Wire.endTransmission() != 0) return 0x800000;
    if (Wire.requestFrom((uint8_t)_i2c_address, (uint8_t)3) != 3) return 0x800000;
    return (((uint32_t)readByte() << 16) | ((uint32_t)readByte() << 8) | readByte());
}

/* = Write one 8-bit BME280 register = */
bool GyverBME280::writeRegister(uint8_t address, uint8_t data) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(address);
    Wire.write(data);
    if (Wire.endTransmission() != 0) return false;
    return true;
}

/* = Read one 8-bit BME280 register = */
uint8_t GyverBME280::readRegister(uint8_t address) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(address);
    if (Wire.endTransmission() != 0) return 0;
    if (Wire.requestFrom((uint8_t)_i2c_address, (uint8_t)1) != 1) return 0;
    return readByte();
}

/* = Structure to store all calibration values = */
bool GyverBME280::readCalibrationData() {
    /* first part request*/
    if (!requestBytes(0x88, 26)) return false;
    /* reading */
    CalibrationData._T1 = readU16LE();
    CalibrationData._T2 = readS16LE();
    CalibrationData._T3 = readS16LE();
    CalibrationData._P1 = readU16LE();
    CalibrationData._P2 = readS16LE();
    CalibrationData._P3 = readS16LE();
    CalibrationData._P4 = readS16LE();
    CalibrationData._P5 = readS16LE();
    CalibrationData._P6 = readS16LE();
    CalibrationData._P7 = readS16LE();
    CalibrationData._P8 = readS16LE();
    CalibrationData._P9 = readS16LE();
    readByte();  // skip 25
    CalibrationData._H1 = readByte();

    if (!_hasHumidity) return true;

    /* second part request*/
    if (!requestBytes(0xE1, 7)) return false;
    /* reading */
    CalibrationData._H2 = readS16LE();
    CalibrationData._H3 = readByte();
    uint8_t e4 = readByte();
    uint8_t e5 = readByte();
    uint8_t e6 = readByte();
    CalibrationData._H4 = signExtend12(((uint16_t)e4 << 4) | (e5 & 0x0F));
    CalibrationData._H5 = signExtend12(((uint16_t)e6 << 4) | (e5 >> 4));
    CalibrationData._H6 = (int8_t)readByte();
    return true;
}

uint8_t GyverBME280::readByte() {
    int value = Wire.read();
    return (value < 0) ? 0 : (uint8_t)value;
}

uint16_t GyverBME280::readU16LE() {
    uint16_t low = readByte();
    return low | ((uint16_t)readByte() << 8);
}

int16_t GyverBME280::readS16LE() {
    return (int16_t)readU16LE();
}

int16_t GyverBME280::signExtend12(uint16_t value) {
    value &= 0x0FFF;
    if (value & 0x0800) value |= 0xF000;
    return (int16_t)value;
}

bool GyverBME280::requestBytes(uint8_t address, uint8_t len) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(address);
    if (Wire.endTransmission() != 0) return false;
    return Wire.requestFrom((uint8_t)_i2c_address, len) == len;
}
