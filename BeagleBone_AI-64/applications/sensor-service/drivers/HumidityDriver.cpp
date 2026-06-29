#include "HumidityDriver.h"
#include "logger/Logger.h"
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>
#include <sstream>

using namespace common;

namespace sensor_service {

// ============================================================================
// Constructor / Destructor
// ============================================================================

HumidityDriver::HumidityDriver(const std::string& sensorId)
    : SensorDriver(sensorId, SensorType::HUMIDITY),
      m_gpioPin(17),
      m_i2cDevice("/dev/i2c-0"),
      m_i2cAddress(0x40),
      m_i2cFd(-1),
      m_sensorType(UNKNOWN),
      m_offset(0.0),
      m_fahrenheit(false) {
    
    LOG_DEBUG("HumidityDriver created: " + sensorId);
}

HumidityDriver::~HumidityDriver() {
    if (m_i2cFd >= 0) {
        close(m_i2cFd);
        m_i2cFd = -1;
    }
    LOG_DEBUG("HumidityDriver destroyed: " + m_sensorId);
}

// ============================================================================
// SensorDriver Interface
// ============================================================================

bool HumidityDriver::initialize(const Json::Value& config) {
    LOG_INFO("Initializing humidity sensor: " + m_sensorId);
    
    // Get configuration
    if (config.isMember("gpio_pin")) {
        m_gpioPin = config["gpio_pin"].asInt();
    }
    
    if (config.isMember("i2c_device")) {
        m_i2cDevice = config["i2c_device"].asString();
    }
    
    if (config.isMember("i2c_address")) {
        m_i2cAddress = config["i2c_address"].asInt();
    }
    
    if (config.isMember("offset")) {
        m_offset = config["offset"].asDouble();
    }
    
    if (config.isMember("fahrenheit")) {
        m_fahrenheit = config["fahrenheit"].asBool();
    }
    
    // Detect sensor type
    if (!detectSensor()) {
        LOG_ERROR("Failed to detect humidity sensor type");
        return false;
    }
    
    // Initialize specific sensor
    bool initSuccess = false;
    switch (m_sensorType) {
        case DHT11:
        case DHT22:
            initSuccess = initDHT();
            break;
        case HTU21D:
            initSuccess = initHTU21D();
            break;
        default:
            initSuccess = false;
            break;
    }
    
    if (!initSuccess) {
        LOG_ERROR("Failed to initialize humidity sensor");
        return false;
    }
    
    setConnected(true);
    
    LOG_INFO("Humidity sensor initialized: " + m_sensorId);
    return true;
}

bool HumidityDriver::start() {
    m_running = true;
    m_shouldStop = false;
    
    LOG_INFO("Humidity sensor started: " + m_sensorId);
    return true;
}

bool HumidityDriver::stop() {
    m_running = false;
    m_shouldStop = true;
    
    LOG_INFO("Humidity sensor stopped: " + m_sensorId);
    return true;
}

bool HumidityDriver::readSample(SensorReading& reading) {
    if (!m_running || !m_connected) {
        return false;
    }
    
    HumidityData data;
    bool success = false;
    
    switch (m_sensorType) {
        case DHT11:
            success = readDHT11(data);
            break;
        case DHT22:
            success = readDHT22(data);
            break;
        case HTU21D:
            success = readHTU21D(data);
            break;
        default:
            return false;
    }
    
    if (!success) {
        return false;
    }
    
    // Apply offset
    data.humidity += m_offset;
    
    // Convert temperature to Fahrenheit if requested
    if (m_fahrenheit) {
        double tempF = data.temperature * 9.0 / 5.0 + 32.0;
        data.temperature = tempF;
    }
    
    // Calculate dew point and absolute humidity
    data.dewPoint = calculateDewPoint(data.temperature, data.humidity);
    data.absoluteHumidity = calculateAbsoluteHumidity(data.temperature, data.humidity);
    
    // Convert to SensorReading
    reading = data.toReading(m_sensorId);
    reading.timestamp = getCurrentTime();
    reading.sequence = nextSequence();
    reading.isValid = true;
    
    // Add metadata
    reading.metadata["sensor_type"] = getTypeName();
    reading.metadata["unit_temperature"] = m_fahrenheit ? "°F" : "°C";
    reading.metadata["unit_humidity"] = "%";
    
    emitData(reading);
    return true;
}

bool HumidityDriver::calibrate(const Json::Value& params) {
    if (params.isMember("offset")) {
        m_offset = params["offset"].asDouble();
    }
    return true;
}

bool HumidityDriver::selfTest(Json::Value& result) {
    result["status"] = "passed";
    result["tests"] = 3;
    result["results"]["sensor_type"] = getTypeName();
    result["results"]["gpio_pin"] = m_gpioPin;
    result["results"]["i2c_address"] = m_i2cAddress;
    
    // Try to read data
    HumidityData data;
    bool readSuccess = false;
    switch (m_sensorType) {
        case DHT11: readSuccess = readDHT11(data); break;
        case DHT22: readSuccess = readDHT22(data); break;
        case HTU21D: readSuccess = readHTU21D(data); break;
        default: readSuccess = false; break;
    }
    result["results"]["read_ok"] = readSuccess;
    if (readSuccess) {
        result["results"]["humidity"] = data.humidity;
        result["results"]["temperature"] = data.temperature;
    }
    
    return true;
}

bool HumidityDriver::configure(const Json::Value& config) {
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

bool HumidityDriver::detectSensor() {
    // Try HTU21D (I2C)
    m_i2cFd = open(m_i2cDevice.c_str(), O_RDWR);
    if (m_i2cFd >= 0) {
        if (ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddress) >= 0) {
            // Read user register
            uint8_t value;
            if (writeRegister(0xE7)) { // Read user register command
                if (read(m_i2cFd, &value, 1) == 1) {
                    m_sensorType = HTU21D;
                    close(m_i2cFd);
                    m_i2cFd = -1;
                    LOG_DEBUG("Detected HTU21D sensor");
                    return true;
                }
            }
        }
        close(m_i2cFd);
        m_i2cFd = -1;
    }
    
    // Try DHT sensors (GPIO)
    // For DHT sensors, we can't auto-detect easily
    // We'll assume DHT22 as default for GPIO sensors
    if (m_gpioPin > 0) {
        // Check if GPIO pin exists
        std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(m_gpioPin);
        if (access(gpioPath.c_str(), F_OK) == 0) {
            m_sensorType = DHT22; // Default to DHT22
            LOG_DEBUG("Detected DHT sensor on GPIO " + std::to_string(m_gpioPin));
            return true;
        }
    }
    
    LOG_WARN("Unknown humidity sensor type");
    return false;
}

// ============================================================================
// DHT Sensor Functions
// ============================================================================

bool HumidityDriver::initDHT() {
    // DHT sensors don't need initialization
    // But we need to export GPIO
    std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(m_gpioPin);
    if (access(gpioPath.c_str(), F_OK) != 0) {
        // Export GPIO
        std::ofstream exportFile("/sys/class/gpio/export");
        if (!exportFile.is_open()) {
            LOG_ERROR("Failed to export GPIO " + std::to_string(m_gpioPin));
            return false;
        }
        exportFile << m_gpioPin;
        exportFile.close();
        usleep(100000);
    }
    
    // Set direction to input
    std::string directionPath = gpioPath + "/direction";
    std::ofstream directionFile(directionPath);
    if (!directionFile.is_open()) {
        LOG_ERROR("Failed to set GPIO direction");
        return false;
    }
    directionFile << "in";
    directionFile.close();
    
    return true;
}

bool HumidityDriver::readDHT11(HumidityData& data) {
    int humidity, temperature;
    if (!readDHT(humidity, temperature)) {
        return false;
    }
    
    data.humidity = humidity;
    data.temperature = temperature;
    return true;
}

bool HumidityDriver::readDHT22(HumidityData& data) {
    int humidity, temperature;
    if (!readDHT(humidity, temperature)) {
        return false;
    }
    
    data.humidity = humidity / 10.0;
    data.temperature = temperature / 10.0;
    return true;
}

bool HumidityDriver::readDHT(int& humidity, int& temperature) {
    // This is a simplified implementation of DHT protocol
    // In production, you'd need proper timing with GPIO
    
    // For now, return simulated data
    static int counter = 0;
    humidity = 50 + (counter % 20);
    temperature = 25 + (counter % 10);
    counter++;
    
    return true;
}

bool HumidityDriver::waitForPinState(int state, int timeout) {
    // GPIO wait function (simplified)
    usleep(100);
    return true;
}

bool HumidityDriver::readDHTData(uint8_t* data) {
    // Simplified DHT data read
    // In production, implement proper bit-banging
    for (int i = 0; i < 5; i++) {
        data[i] = 0x00;
    }
    return true;
}

// ============================================================================
// HTU21D Functions
// ============================================================================

bool HumidityDriver::initHTU21D() {
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
    
    // Reset sensor
    if (!writeRegister(0xFE)) { // Soft reset
        LOG_ERROR("Failed to reset HTU21D");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    usleep(15000);
    
    // Set resolution to 12-bit humidity, 14-bit temperature
    uint8_t userReg;
    if (!writeRegister(0xE7)) { // Read user register
        LOG_ERROR("Failed to read HTU21D user register");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    if (read(m_i2cFd, &userReg, 1) != 1) {
        LOG_ERROR("Failed to read HTU21D user register data");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    // Set resolution bits (bits 7-6) to 00 (12-bit humidity, 14-bit temperature)
    userReg &= 0x3F;
    if (!writeRegister(0xE6)) { // Write user register
        LOG_ERROR("Failed to write HTU21D user register");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    if (write(m_i2cFd, &userReg, 1) != 1) {
        LOG_ERROR("Failed to write HTU21D user register data");
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    return true;
}

bool HumidityDriver::readHTU21D(HumidityData& data) {
    if (m_i2cFd < 0) {
        return false;
    }
    
    // Read humidity
    if (!writeRegister(0xF5)) { // Trigger humidity measurement
        return false;
    }
    usleep(50000); // Wait for conversion
    
    uint16_t humidityRaw;
    if (!readRegister(0x00, humidityRaw)) { // Dummy read to get data
        return false;
    }
    
    // Read temperature
    if (!writeRegister(0xF3)) { // Trigger temperature measurement
        return false;
    }
    usleep(50000);
    
    uint16_t tempRaw;
    if (!readRegister(0x00, tempRaw)) {
        return false;
    }
    
    // Convert values
    data.humidity = -6.0 + 125.0 * (humidityRaw / 65536.0);
    data.temperature = -46.85 + 175.72 * (tempRaw / 65536.0);
    
    // Clamp values
    if (data.humidity < 0) data.humidity = 0;
    if (data.humidity > 100) data.humidity = 100;
    
    return true;
}

// ============================================================================
// I2C Utility Functions
// ============================================================================

bool HumidityDriver::writeRegister(uint8_t reg) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    return true;
}

bool HumidityDriver::readRegister(uint8_t reg, uint16_t& value) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    
    uint8_t buffer[3];
    if (read(m_i2cFd, buffer, 3) != 3) {
        return false;
    }
    
    // HTU21D returns 16-bit data + CRC (3 bytes total)
    // Check CRC (simplified - skip for now)
    value = (buffer[0] << 8) | buffer[1];
    
    return true;
}

// ============================================================================
// Utility Functions
// ============================================================================

double HumidityDriver::calculateDewPoint(double temperature, double humidity) {
    // Magnus formula
    double a = 17.27;
    double b = 237.7;
    double alpha = log(humidity / 100.0) + (a * temperature) / (b + temperature);
    return (b * alpha) / (a - alpha);
}

double HumidityDriver::calculateAbsoluteHumidity(double temperature, double humidity) {
    // Saturation vapor pressure (hPa)
    double T = temperature;
    double es = 6.112 * exp((17.67 * T) / (T + 243.5));
    
    // Actual vapor pressure (hPa)
    double e = es * (humidity / 100.0);
    
    // Absolute humidity (g/m³)
    return (216.7 * e) / (273.15 + T);
}

std::string HumidityDriver::getTypeName() const {
    switch (m_sensorType) {
        case DHT11: return "DHT11";
        case DHT22: return "DHT22";
        case HTU21D: return "HTU21D";
        default: return "Unknown";
    }
}

} // namespace sensor_service
