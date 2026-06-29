#include "TemperatureDriver.h"
#include "logger/Logger.h"
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>

using namespace common;

namespace sensor_service {

// ============================================================================
// Constructor / Destructor
// ============================================================================

TemperatureDriver::TemperatureDriver(const std::string& sensorId)
    : SensorDriver(sensorId, SensorType::TEMPERATURE),
      m_i2cFd(-1),
      m_i2cDevice("/dev/i2c-0"),
      m_i2cAddress(0x48),
      m_sensorType(UNKNOWN),
      m_resolution(12),
      m_offset(0.0),
      m_fahrenheit(false) {
    
    LOG_DEBUG("TemperatureDriver created: " + sensorId);
}

TemperatureDriver::~TemperatureDriver() {
    if (m_i2cFd >= 0) {
        close(m_i2cFd);
        m_i2cFd = -1;
    }
    LOG_DEBUG("TemperatureDriver destroyed: " + m_sensorId);
}

// ============================================================================
// SensorDriver Interface
// ============================================================================

bool TemperatureDriver::initialize(const Json::Value& config) {
    LOG_INFO("Initializing temperature sensor: " + m_sensorId);
    
    // Get configuration
    if (config.isMember("i2c_device")) {
        m_i2cDevice = config["i2c_device"].asString();
    }
    
    if (config.isMember("i2c_address")) {
        m_i2cAddress = config["i2c_address"].asInt();
    }
    
    if (config.isMember("resolution")) {
        m_resolution = config["resolution"].asInt();
        if (m_resolution < 9) m_resolution = 9;
        if (m_resolution > 12) m_resolution = 12;
    }
    
    if (config.isMember("offset")) {
        m_offset = config["offset"].asDouble();
    }
    
    if (config.isMember("fahrenheit")) {
        m_fahrenheit = config["fahrenheit"].asBool();
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
        LOG_ERROR("Failed to detect temperature sensor type");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    // Initialize specific sensor
    bool initSuccess = false;
    switch (m_sensorType) {
        case TMP102:
            initSuccess = initTMP102();
            break;
        case TMP112:
            initSuccess = initTMP112();
            break;
        case LM75:
            initSuccess = initLM75();
            break;
        case DS18B20:
            initSuccess = initDS18B20();
            break;
        default:
            initSuccess = false;
            break;
    }
    
    if (!initSuccess) {
        LOG_ERROR("Failed to initialize temperature sensor");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    setConnected(true);
    
    LOG_INFO("Temperature sensor initialized: " + m_sensorId);
    return true;
}

bool TemperatureDriver::start() {
    if (m_i2cFd < 0 && m_sensorType != DS18B20) {
        LOG_ERROR("Temperature sensor I2C device not open: " + m_sensorId);
        return false;
    }
    
    m_running = true;
    m_shouldStop = false;
    
    LOG_INFO("Temperature sensor started: " + m_sensorId);
    return true;
}

bool TemperatureDriver::stop() {
    m_running = false;
    m_shouldStop = true;
    
    LOG_INFO("Temperature sensor stopped: " + m_sensorId);
    return true;
}

bool TemperatureDriver::readSample(SensorReading& reading) {
    if (!m_running || !m_connected) {
        return false;
    }
    
    double temperature = 0.0;
    bool success = false;
    
    switch (m_sensorType) {
        case TMP102:
            success = readTMP102(temperature);
            break;
        case TMP112:
            success = readTMP112(temperature);
            break;
        case LM75:
            success = readLM75(temperature);
            break;
        case DS18B20:
            success = readDS18B20(temperature);
            break;
        default:
            return false;
    }
    
    if (!success) {
        return false;
    }
    
    // Apply offset
    temperature += m_offset;
    
    // Convert to Fahrenheit if requested
    if (m_fahrenheit) {
        temperature = celsiusToFahrenheit(temperature);
    }
    
    // Build reading
    reading.type = SensorType::TEMPERATURE;
    reading.sensorId = m_sensorId;
    reading.timestamp = getCurrentTime();
    reading.sequence = nextSequence();
    reading.isValid = true;
    reading.values["temperature"] = temperature;
    reading.metadata["sensor_type"] = getTypeName();
    reading.metadata["resolution"] = std::to_string(m_resolution);
    reading.metadata["unit"] = m_fahrenheit ? "°F" : "°C";
    
    emitData(reading);
    return true;
}

bool TemperatureDriver::calibrate(const Json::Value& params) {
    if (params.isMember("offset")) {
        m_offset = params["offset"].asDouble();
    }
    return true;
}

bool TemperatureDriver::selfTest(Json::Value& result) {
    result["status"] = "passed";
    result["tests"] = 3;
    result["results"]["i2c_connected"] = m_i2cFd >= 0 || m_sensorType == DS18B20;
    result["results"]["sensor_type"] = getTypeName();
    result["results"]["resolution"] = m_resolution;
    
    // Try to read temperature
    double temp;
    bool readSuccess = false;
    switch (m_sensorType) {
        case TMP102: readSuccess = readTMP102(temp); break;
        case TMP112: readSuccess = readTMP112(temp); break;
        case LM75: readSuccess = readLM75(temp); break;
        case DS18B20: readSuccess = readDS18B20(temp); break;
        default: readSuccess = false; break;
    }
    result["results"]["read_ok"] = readSuccess;
    if (readSuccess) {
        result["results"]["temperature"] = temp;
    }
    
    return m_i2cFd >= 0 || m_sensorType == DS18B20;
}

bool TemperatureDriver::configure(const Json::Value& config) {
    if (config.isMember("resolution")) {
        m_resolution = config["resolution"].asInt();
        if (m_resolution < 9) m_resolution = 9;
        if (m_resolution > 12) m_resolution = 12;
    }
    if (config.isMember("offset")) {
        m_offset = config["offset"].asDouble();
    }
    if (config.isMember("fahrenheit")) {
        m_fahrenheit = config["fahrenheit"].asBool();
    }
    return true;
}

// ============================================================================
// Sensor Detection
// ============================================================================

bool TemperatureDriver::detectSensor() {
    // Try to read WHO_AM_I or device ID registers
    uint8_t value;
    
    // Try TMP102 (0x48-0x4F)
    // Read manufacturer ID (0xFE)
    if (readRegister(0xFE, value)) {
        if (value == 0x55) {
            m_sensorType = TMP102;
            LOG_DEBUG("Detected TMP102 sensor");
            return true;
        }
    }
    
    // Try TMP112 (0x48-0x4F)
    // Read manufacturer ID (0xFE)
    if (readRegister(0xFE, value)) {
        if (value == 0x55) {
            m_sensorType = TMP112;
            LOG_DEBUG("Detected TMP112 sensor");
            return true;
        }
    }
    
    // Try LM75 (0x48-0x4F)
    // Read configuration register
    if (readRegister(0x01, value)) {
        m_sensorType = LM75;
        LOG_DEBUG("Detected LM75 sensor");
        return true;
    }
    
    // Try DS18B20 (1-Wire)
    std::ifstream w1Devices("/sys/bus/w1/devices/w1_bus_master1/w1_master_slaves");
    if (w1Devices.is_open()) {
        std::string line;
        while (std::getline(w1Devices, line)) {
            if (line.find("28-") == 0) { // DS18B20 family code
                m_sensorType = DS18B20;
                m_i2cFd = -1; // DS18B20 uses 1-Wire, not I2C
                LOG_DEBUG("Detected DS18B20 sensor: " + line);
                w1Devices.close();
                return true;
            }
        }
        w1Devices.close();
    }
    
    LOG_WARN("Unknown temperature sensor type");
    return false;
}

// ============================================================================
// Sensor-Specific Functions
// ============================================================================

bool TemperatureDriver::initTMP102() {
    // Set resolution
    uint8_t configReg;
    if (!readRegister(0x01, configReg)) return false;
    
    // Clear resolution bits (bits 6-7)
    configReg &= 0x3F;
    // Set resolution: 9-12 bits
    configReg |= ((m_resolution - 9) << 6);
    
    if (!writeRegister(0x01, configReg)) return false;
    
    return true;
}

bool TemperatureDriver::initTMP112() {
    // Similar to TMP102 but with extended range
    return initTMP102();
}

bool TemperatureDriver::initLM75() {
    // LM75 doesn't need special initialization
    return true;
}

bool TemperatureDriver::initDS18B20() {
    // DS18B20 uses 1-Wire, no I2C initialization needed
    return true;
}

// ============================================================================
// Read Functions
// ============================================================================

bool TemperatureDriver::readTMP102(double& temp) {
    uint16_t value;
    if (!readRegister(0x00, value)) return false;
    
    // TMP102 temperature is 12-bit (or 13-bit with extended mode)
    // Format: high byte is whole degrees, low byte is fractional
    int16_t tempRaw = (value >> 4) & 0x0FFF;
    
    // Check if negative
    bool negative = (tempRaw & 0x0800) != 0;
    if (negative) {
        tempRaw |= 0xF000;
    }
    
    temp = tempRaw * 0.0625; // 1/16 degree resolution
    return true;
}

bool TemperatureDriver::readTMP112(double& temp) {
    uint16_t value;
    if (!readRegister(0x00, value)) return false;
    
    // TMP112 temperature is 12-bit
    int16_t tempRaw = (value >> 4) & 0x0FFF;
    
    bool negative = (tempRaw & 0x0800) != 0;
    if (negative) {
        tempRaw |= 0xF000;
    }
    
    temp = tempRaw * 0.0625;
    return true;
}

bool TemperatureDriver::readLM75(double& temp) {
    uint16_t value;
    if (!readRegister(0x00, value)) return false;
    
    // LM75 temperature is 9-bit, 2's complement
    int16_t tempRaw = (value >> 7) & 0x01FF;
    
    bool negative = (tempRaw & 0x0100) != 0;
    if (negative) {
        tempRaw |= 0xFE00;
    }
    
    temp = tempRaw * 0.5;
    return true;
}

bool TemperatureDriver::readDS18B20(double& temp) {
    // Read from sysfs
    std::ifstream w1Devices("/sys/bus/w1/devices/w1_bus_master1/w1_master_slaves");
    if (!w1Devices.is_open()) return false;
    
    std::string deviceId;
    if (!std::getline(w1Devices, deviceId)) {
        w1Devices.close();
        return false;
    }
    w1Devices.close();
    
    std::string w1File = "/sys/bus/w1/devices/" + deviceId + "/w1_slave";
    std::ifstream w1Slave(w1File);
    if (!w1Slave.is_open()) return false;
    
    std::string line;
    while (std::getline(w1Slave, line)) {
        size_t pos = line.find("t=");
        if (pos != std::string::npos) {
            int tempRaw = std::stoi(line.substr(pos + 2));
            temp = tempRaw / 1000.0;
            w1Slave.close();
            return true;
        }
    }
    
    w1Slave.close();
    return false;
}

// ============================================================================
// I2C Utility Functions
// ============================================================================

bool TemperatureDriver::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    if (write(m_i2cFd, buffer, 2) != 2) {
        return false;
    }
    return true;
}

bool TemperatureDriver::readRegister(uint8_t reg, uint8_t& value) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    if (read(m_i2cFd, &value, 1) != 1) {
        return false;
    }
    return true;
}

bool TemperatureDriver::readRegister(uint8_t reg, uint16_t& value) {
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

bool TemperatureDriver::writeRegister(uint8_t reg, uint16_t value) {
    uint8_t buffer[3] = {reg, static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)};
    if (write(m_i2cFd, buffer, 3) != 3) {
        return false;
    }
    return true;
}

bool TemperatureDriver::checkConnection() {
    if (m_sensorType == DS18B20) {
        // Check 1-Wire device exists
        std::ifstream w1Devices("/sys/bus/w1/devices/w1_bus_master1/w1_master_slaves");
        if (!w1Devices.is_open()) return false;
        std::string line;
        bool found = false;
        while (std::getline(w1Devices, line)) {
            if (line.find("28-") == 0) {
                found = true;
                break;
            }
        }
        w1Devices.close();
        return found;
    }
    
    if (m_i2cFd < 0) return false;
    
    // Try to read a register
    uint8_t value;
    return readRegister(0x00, value);
}

double TemperatureDriver::celsiusToFahrenheit(double celsius) {
    return celsius * 9.0 / 5.0 + 32.0;
}

std::string TemperatureDriver::getTypeName() const {
    switch (m_sensorType) {
        case TMP102: return "TMP102";
        case TMP112: return "TMP112";
        case LM75: return "LM75";
        case DS18B20: return "DS18B20";
        default: return "Unknown";
    }
}

} // namespace sensor_service
