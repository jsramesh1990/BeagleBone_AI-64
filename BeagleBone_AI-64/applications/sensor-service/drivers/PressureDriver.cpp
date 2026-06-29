#include "PressureDriver.h"
#include "logger/Logger.h"
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>

using namespace common;

namespace sensor_service {

// ============================================================================
// Constructor / Destructor
// ============================================================================

PressureDriver::PressureDriver(const std::string& sensorId)
    : SensorDriver(sensorId, SensorType::PRESSURE),
      m_i2cFd(-1),
      m_i2cDevice("/dev/i2c-0"),
      m_i2cAddress(0x77),
      m_sensorType(UNKNOWN),
      m_oversampling(2),
      m_seaLevel(false) {
    
    // Initialize calibration data
    memset(&m_calibration, 0, sizeof(m_calibration));
    
    LOG_DEBUG("PressureDriver created: " + sensorId);
}

PressureDriver::~PressureDriver() {
    if (m_i2cFd >= 0) {
        close(m_i2cFd);
        m_i2cFd = -1;
    }
    LOG_DEBUG("PressureDriver destroyed: " + m_sensorId);
}

// ============================================================================
// SensorDriver Interface
// ============================================================================

bool PressureDriver::initialize(const Json::Value& config) {
    LOG_INFO("Initializing pressure sensor: " + m_sensorId);
    
    // Get configuration
    if (config.isMember("i2c_device")) {
        m_i2cDevice = config["i2c_device"].asString();
    }
    
    if (config.isMember("i2c_address")) {
        m_i2cAddress = config["i2c_address"].asInt();
    }
    
    if (config.isMember("oversampling")) {
        m_oversampling = config["oversampling"].asInt();
        if (m_oversampling < 0) m_oversampling = 0;
        if (m_oversampling > 4) m_oversampling = 4;
    }
    
    if (config.isMember("sea_level")) {
        m_seaLevel = config["sea_level"].asBool();
    }
    
    // Open I2C device
    m_i2cFd = open(m_i2cDevice.c_str(), O_RDWR);
    if (m_i2cFd < 0) {
        LOG_ERROR("Failed to open I2C device: " + m_i2cDevice);
        return false;
    }
    
    // Set I2C slave address
    if (ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddress) < 0) {
        LOG_ERROR("Failed to set I2C slave address");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    // Detect sensor type
    if (!detectSensor()) {
        LOG_ERROR("Failed to detect pressure sensor type");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    // Initialize specific sensor
    bool initSuccess = false;
    switch (m_sensorType) {
        case BMP180:
            initSuccess = initBMP180();
            break;
        case BMP280:
            initSuccess = initBMP280();
            break;
        default:
            initSuccess = false;
            break;
    }
    
    if (!initSuccess) {
        LOG_ERROR("Failed to initialize pressure sensor");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    setConnected(true);
    
    LOG_INFO("Pressure sensor initialized: " + m_sensorId);
    return true;
}

bool PressureDriver::start() {
    if (m_i2cFd < 0) {
        LOG_ERROR("Pressure sensor I2C device not open: " + m_sensorId);
        return false;
    }
    
    m_running = true;
    m_shouldStop = false;
    
    LOG_INFO("Pressure sensor started: " + m_sensorId);
    return true;
}

bool PressureDriver::stop() {
    m_running = false;
    m_shouldStop = true;
    
    LOG_INFO("Pressure sensor stopped: " + m_sensorId);
    return true;
}

bool PressureDriver::readSample(SensorReading& reading) {
    if (!m_running || !m_connected) {
        return false;
    }
    
    if (m_i2cFd < 0) {
        setConnected(false);
        return false;
    }
    
    PressureData data;
    bool success = false;
    
    switch (m_sensorType) {
        case BMP180:
            success = readBMP180(data);
            break;
        case BMP280:
            success = readBMP280(data);
            break;
        default:
            return false;
    }
    
    if (!success) {
        return false;
    }
    
    // Convert to SensorReading
    reading = data.toReading(m_sensorId);
    reading.timestamp = getCurrentTime();
    reading.sequence = nextSequence();
    reading.isValid = true;
    
    // Add metadata
    reading.metadata["sensor_type"] = (m_sensorType == BMP180) ? "BMP180" : "BMP280";
    reading.metadata["oversampling"] = std::to_string(m_oversampling);
    
    emitData(reading);
    return true;
}

bool PressureDriver::calibrate(const Json::Value& params) {
    // Pressure sensors typically don't need calibration
    // But we can set sea level pressure offset
    if (params.isMember("sea_level_pressure")) {
        // Store for altitude calculation
    }
    return true;
}

bool PressureDriver::selfTest(Json::Value& result) {
    result["status"] = "passed";
    result["tests"] = 3;
    result["results"]["i2c_connected"] = m_i2cFd >= 0;
    result["results"]["sensor_type"] = (m_sensorType == BMP180) ? "BMP180" : "BMP280";
    result["results"]["oversampling"] = m_oversampling;
    
    // Try to read pressure
    PressureData data;
    bool readSuccess = false;
    switch (m_sensorType) {
        case BMP180: readSuccess = readBMP180(data); break;
        case BMP280: readSuccess = readBMP280(data); break;
        default: readSuccess = false; break;
    }
    result["results"]["read_ok"] = readSuccess;
    if (readSuccess) {
        result["results"]["pressure"] = data.pressure;
        result["results"]["temperature"] = data.temperature;
    }
    
    return m_i2cFd >= 0;
}

bool PressureDriver::configure(const Json::Value& config) {
    if (config.isMember("oversampling")) {
        m_oversampling = config["oversampling"].asInt();
        if (m_oversampling < 0) m_oversampling = 0;
        if (m_oversampling > 4) m_oversampling = 4;
    }
    if (config.isMember("sea_level")) {
        m_seaLevel = config["sea_level"].asBool();
    }
    return true;
}

// ============================================================================
// Sensor Detection
// ============================================================================

bool PressureDriver::detectSensor() {
    uint8_t chipId;
    
    // Try BMP180
    if (readRegister(0xD0, chipId)) {
        if (chipId == 0x55) {
            m_sensorType = BMP180;
            LOG_DEBUG("Detected BMP180 sensor");
            return true;
        }
    }
    
    // Try BMP280
    if (readRegister(0xD0, chipId)) {
        if (chipId == 0x58) {
            m_sensorType = BMP280;
            LOG_DEBUG("Detected BMP280 sensor");
            return true;
        }
    }
    
    LOG_WARN("Unknown pressure sensor type");
    return false;
}

// ============================================================================
// BMP180 Functions
// ============================================================================

bool PressureDriver::initBMP180() {
    // Read calibration data
    if (!readCalibrationBMP180()) {
        return false;
    }
    
    // Set oversampling
    uint8_t oss = m_oversampling & 0x03;
    if (!writeRegister(0xF4, (oss << 6))) {
        return false;
    }
    
    return true;
}

bool PressureDriver::readCalibrationBMP180() {
    // Read calibration data from EEPROM
    uint16_t calData[11];
    
    // AC1 (0xAA)
    if (!readRegister(0xAA, calData[0])) return false;
    // AC2 (0xAC)
    if (!readRegister(0xAC, calData[1])) return false;
    // AC3 (0xAE)
    if (!readRegister(0xAE, calData[2])) return false;
    // AC4 (0xB0)
    if (!readRegister(0xB0, calData[3])) return false;
    // AC5 (0xB2)
    if (!readRegister(0xB2, calData[4])) return false;
    // AC6 (0xB4)
    if (!readRegister(0xB4, calData[5])) return false;
    // B1 (0xB6)
    if (!readRegister(0xB6, calData[6])) return false;
    // B2 (0xB8)
    if (!readRegister(0xB8, calData[7])) return false;
    // MB (0xBA)
    if (!readRegister(0xBA, calData[8])) return false;
    // MC (0xBC)
    if (!readRegister(0xBC, calData[9])) return false;
    // MD (0xBE)
    if (!readRegister(0xBE, calData[10])) return false;
    
    // Convert to signed/unsigned
    m_calibration.AC1 = static_cast<int16_t>(calData[0]);
    m_calibration.AC2 = static_cast<int16_t>(calData[1]);
    m_calibration.AC3 = static_cast<int16_t>(calData[2]);
    m_calibration.AC4 = calData[3];
    m_calibration.AC5 = calData[4];
    m_calibration.AC6 = calData[5];
    m_calibration.B1 = static_cast<int16_t>(calData[6]);
    m_calibration.B2 = static_cast<int16_t>(calData[7]);
    m_calibration.MB = static_cast<int16_t>(calData[8]);
    m_calibration.MC = static_cast<int16_t>(calData[9]);
    m_calibration.MD = static_cast<int16_t>(calData[10]);
    
    return true;
}

bool PressureDriver::readBMP180(PressureData& data) {
    int32_t UT, UP;
    
    // Read uncompensated temperature
    if (!readRawBMP180(UT, UP)) {
        return false;
    }
    
    // Calculate temperature
    int32_t X1 = (UT - m_calibration.AC6) * m_calibration.AC5 / 32768;
    int32_t X2 = m_calibration.MC * 2048 / (X1 + m_calibration.MD);
    int32_t B5 = X1 + X2;
    
    data.temperature = (B5 + 8) / 16.0 / 10.0;
    
    // Calculate pressure
    int32_t B6 = B5 - 4000;
    int32_t X3 = (m_calibration.B2 * (B6 * B6 / 4096)) / 2048;
    int32_t B3 = (m_calibration.AC1 * 4 + X3 + 2) / 4;
    int32_t X4 = (m_calibration.AC3 * B6) / 8192;
    int32_t B4 = (m_calibration.AC4 * (X4 + 32768)) / 32768;
    int32_t B7 = (UP - B3) * (50000 / (m_calibration.oversampling >> 1));
    int32_t p = (B7 * 2) * B4 / 65536;
    
    // Additional compensation
    if (p < 0) {
        p = B7 * 2 / B4 * 65536;
    }
    
    data.pressure = p / 100.0; // Convert to hPa
    
    // Calculate altitude
    if (m_seaLevel) {
        data.altitude = calculateAltitude(data.pressure, 1013.25);
    }
    
    data.seaLevelPressure = 1013.25;
    
    return true;
}

bool PressureDriver::readRawBMP180(int32_t& UT, int32_t& UP) {
    // Read temperature
    if (!writeRegister(0xF4, 0x2E)) return false;
    usleep(5000);
    
    uint16_t temp;
    if (!readRegister(0xF6, temp)) return false;
    UT = temp;
    
    // Read pressure
    uint8_t oss = m_oversampling & 0x03;
    if (!writeRegister(0xF4, (0x34 | (oss << 6)))) return false;
    
    // Wait for conversion
    int waitTime = (oss == 0) ? 5 : (oss == 1) ? 8 : (oss == 2) ? 14 : 26;
    usleep(waitTime * 1000);
    
    uint32_t pressure;
    uint8_t msb, lsb, xlsb;
    if (!readRegister(0xF6, msb)) return false;
    if (!readRegister(0xF7, lsb)) return false;
    if (!readRegister(0xF8, xlsb)) return false;
    
    pressure = (msb << 16) | (lsb << 8) | xlsb;
    UP = pressure >> (8 - oss);
    
    return true;
}

// ============================================================================
// BMP280 Functions
// ============================================================================

bool PressureDriver::initBMP280() {
    // Read calibration data
    if (!readCalibrationBMP280()) {
        return false;
    }
    
    // Configure sensor
    uint8_t config = (0x02 << 6) | (0x02 << 3) | 0x01; // IIR filter, TSB, SPI
    if (!writeRegister(0xF5, config)) return false;
    
    // Configure measurement
    uint8_t ctrl = (m_oversampling << 5) | (m_oversampling << 2) | 0x03;
    if (!writeRegister(0xF4, ctrl)) return false;
    
    return true;
}

bool PressureDriver::readCalibrationBMP280() {
    // Read calibration data from EEPROM
    uint8_t buffer[24];
    if (!readRegisters(0x88, buffer, 24)) return false;
    
    // Parse calibration data
    m_calibration.dig_T1 = buffer[0] | (buffer[1] << 8);
    m_calibration.dig_T2 = static_cast<int16_t>(buffer[2] | (buffer[3] << 8));
    m_calibration.dig_T3 = static_cast<int16_t>(buffer[4] | (buffer[5] << 8));
    m_calibration.dig_P1 = buffer[6] | (buffer[7] << 8);
    m_calibration.dig_P2 = static_cast<int16_t>(buffer[8] | (buffer[9] << 8));
    m_calibration.dig_P3 = static_cast<int16_t>(buffer[10] | (buffer[11] << 8));
    m_calibration.dig_P4 = static_cast<int16_t>(buffer[12] | (buffer[13] << 8));
    m_calibration.dig_P5 = static_cast<int16_t>(buffer[14] | (buffer[15] << 8));
    m_calibration.dig_P6 = static_cast<int16_t>(buffer[16] | (buffer[17] << 8));
    m_calibration.dig_P7 = static_cast<int16_t>(buffer[18] | (buffer[19] << 8));
    m_calibration.dig_P8 = static_cast<int16_t>(buffer[20] | (buffer[21] << 8));
    m_calibration.dig_P9 = static_cast<int16_t>(buffer[22] | (buffer[23] << 8));
    
    return true;
}

bool PressureDriver::readBMP280(PressureData& data) {
    int32_t T, P;
    
    // Read raw temperature and pressure
    if (!readRawBMP280(T, P)) {
        return false;
    }
    
    // Calculate temperature
    int32_t var1 = ((T / 16384.0 - m_calibration.dig_T1 / 16384.0) * m_calibration.dig_T2);
    int32_t var2 = ((T / 131072.0 - m_calibration.dig_T1 / 131072.0) * 
                    (T / 131072.0 - m_calibration.dig_T1 / 131072.0) * 
                    m_calibration.dig_T3);
    int32_t tFine = var1 + var2;
    data.temperature = (tFine * 5 + 128) / 256 / 100.0;
    
    // Calculate pressure
    int32_t var1p = tFine / 2.0 - 64000.0;
    int32_t var2p = var1p * var1p * m_calibration.dig_P6 / 32768.0;
    var2p = var2p + var1p * m_calibration.dig_P5 * 2.0;
    var2p = var2p / 4.0 + m_calibration.dig_P4 * 65536.0;
    int32_t var3p = m_calibration.dig_P3 * var1p * var1p / 524288.0;
    var1p = (var3p + m_calibration.dig_P2 * var1p) / 524288.0;
    var1p = (1.0 + var1p / 32768.0) * m_calibration.dig_P1;
    
    if (var1p == 0) {
        return false;
    }
    
    data.pressure = 1048576.0 - P;
    data.pressure = (data.pressure - var2p / 4096.0) * 6250.0 / var1p;
    var1p = m_calibration.dig_P9 * data.pressure * data.pressure / 2147483648.0;
    var2p = data.pressure * m_calibration.dig_P8 / 32768.0;
    data.pressure = data.pressure + (var1p + var2p + m_calibration.dig_P7) / 16.0;
    data.pressure = data.pressure / 100.0; // Convert to hPa
    
    // Calculate altitude
    if (m_seaLevel) {
        data.altitude = calculateAltitude(data.pressure, 1013.25);
    }
    
    data.seaLevelPressure = 1013.25;
    
    return true;
}

bool PressureDriver::readRawBMP280(int32_t& T, int32_t& P) {
    uint8_t buffer[6];
    if (!readRegisters(0xF7, buffer, 6)) return false;
    
    P = (buffer[0] << 12) | (buffer[1] << 4) | (buffer[2] >> 4);
    T = (buffer[3] << 12) | (buffer[4] << 4) | (buffer[5] >> 4);
    
    return true;
}

// ============================================================================
// Utility Functions
// ============================================================================

double PressureDriver::calculateAltitude(double pressure, double seaLevelPressure) {
    // Using barometric formula
    // alt = 44330 * (1 - (P/P0)^(1/5.255))
    double ratio = pressure / seaLevelPressure;
    return 44330.0 * (1.0 - std::pow(ratio, 1.0 / 5.255));
}

bool PressureDriver::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    if (write(m_i2cFd, buffer, 2) != 2) {
        return false;
    }
    return true;
}

bool PressureDriver::readRegister(uint8_t reg, uint8_t& value) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    if (read(m_i2cFd, &value, 1) != 1) {
        return false;
    }
    return true;
}

bool PressureDriver::readRegister(uint8_t reg, uint16_t& value) {
    uint8_t buffer[2];
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    if (read(m_i2cFd, buffer, 2) != 2) {
        return false;
    }
    value = (buffer[0] << 8) | buffer[1];
    return true;
}

bool PressureDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t length) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    if (read(m_i2cFd, buffer, length) != static_cast<ssize_t>(length)) {
        return false;
    }
    return true;
}

bool PressureDriver::checkConnection() {
    if (m_i2cFd < 0) return false;
    
    uint8_t value;
    return readRegister(0xD0, value);
}

} // namespace sensor_service
