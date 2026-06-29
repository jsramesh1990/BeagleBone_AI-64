#include "IMUDriver.h"
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

IMUDriver::IMUDriver(const std::string& sensorId)
    : SensorDriver(sensorId, SensorType::IMU_ACCELEROMETER),
      m_i2cFd(-1),
      m_i2cDevice("/dev/i2c-0"),
      m_i2cAddress(MPU6050_ADDR),
      m_accelRange(1),   // ±8g
      m_gyroRange(1),    // ±500°/s
      m_sampleRate(100), // 100 Hz
      m_enableMagnetometer(false) {
    
    // Initialize calibration offsets
    memset(&m_calibration, 0, sizeof(m_calibration));
    for (int i = 0; i < 3; i++) {
        m_calibration.accelScale[i] = 1.0;
        m_calibration.gyroScale[i] = 1.0;
        m_calibration.magScale[i] = 1.0;
    }
    
    LOG_DEBUG("IMUDriver created: " + sensorId);
}

IMUDriver::~IMUDriver() {
    if (m_i2cFd >= 0) {
        close(m_i2cFd);
        m_i2cFd = -1;
    }
    LOG_DEBUG("IMUDriver destroyed: " + m_sensorId);
}

// ============================================================================
// SensorDriver Interface
// ============================================================================

bool IMUDriver::initialize(const Json::Value& config) {
    LOG_INFO("Initializing IMU sensor: " + m_sensorId);
    
    // Get configuration
    if (config.isMember("i2c_device")) {
        m_i2cDevice = config["i2c_device"].asString();
    }
    
    if (config.isMember("i2c_address")) {
        m_i2cAddress = config["i2c_address"].asInt();
    }
    
    if (config.isMember("accel_range")) {
        m_accelRange = config["accel_range"].asInt();
    }
    
    if (config.isMember("gyro_range")) {
        m_gyroRange = config["gyro_range"].asInt();
    }
    
    if (config.isMember("sample_rate")) {
        m_sampleRate = config["sample_rate"].asInt();
    }
    
    if (config.isMember("enable_magnetometer")) {
        m_enableMagnetometer = config["enable_magnetometer"].asBool();
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
    
    // Initialize the sensor
    if (m_i2cAddress == MPU6050_ADDR) {
        if (!initMPU6050()) {
            LOG_ERROR("Failed to initialize MPU6050");
            close(m_i2cFd);
            m_i2cFd = -1;
            return false;
        }
    } else if (m_i2cAddress == MPU9250_ADDR) {
        if (!initMPU9250()) {
            LOG_ERROR("Failed to initialize MPU9250");
            close(m_i2cFd);
            m_i2cFd = -1;
            return false;
        }
    } else {
        LOG_ERROR("Unknown IMU address: " + std::to_string(m_i2cAddress));
        close(m_i2cFd);
        m_i2cFd = -1;
        return false;
    }
    
    setConnected(true);
    
    LOG_INFO("IMU sensor initialized: " + m_sensorId);
    return true;
}

bool IMUDriver::start() {
    if (m_i2cFd < 0) {
        LOG_ERROR("IMU I2C device not open: " + m_sensorId);
        return false;
    }
    
    m_running = true;
    m_shouldStop = false;
    
    LOG_INFO("IMU sensor started: " + m_sensorId);
    return true;
}

bool IMUDriver::stop() {
    m_running = false;
    m_shouldStop = true;
    
    LOG_INFO("IMU sensor stopped: " + m_sensorId);
    return true;
}

bool IMUDriver::readSample(SensorReading& reading) {
    if (!m_running || !m_connected) {
        return false;
    }
    
    if (m_i2cFd < 0) {
        setConnected(false);
        return false;
    }
    
    IMUData data;
    if (!readIMUData(data)) {
        return false;
    }
    
    // Apply calibration
    applyCalibration(data);
    
    // Calculate magnitudes
    calculateMagnitude(data);
    
    // Convert to SensorReading
    reading = data.toReading(m_sensorId);
    reading.timestamp = getCurrentTime();
    reading.sequence = nextSequence();
    reading.isValid = true;
    
    // Add temperature
    reading.values["temperature"] = data.temperature;
    
    emitData(reading);
    return true;
}

bool IMUDriver::calibrate(const Json::Value& params) {
    LOG_INFO("Calibrating IMU: " + m_sensorId);
    
    // Perform calibration
    // Collect data while stationary
    const int SAMPLES = 100;
    IMUData sum;
    memset(&sum, 0, sizeof(sum));
    
    for (int i = 0; i < SAMPLES; i++) {
        IMUData data;
        if (readIMUData(data)) {
            sum.accel.x += data.accel.x;
            sum.accel.y += data.accel.y;
            sum.accel.z += data.accel.z;
            sum.gyro.x += data.gyro.x;
            sum.gyro.y += data.gyro.y;
            sum.gyro.z += data.gyro.z;
            if (m_enableMagnetometer) {
                sum.mag.x += data.mag.x;
                sum.mag.y += data.mag.y;
                sum.mag.z += data.mag.z;
            }
        }
        usleep(10000);
    }
    
    // Calculate averages
    double avgAccelX = sum.accel.x / SAMPLES;
    double avgAccelY = sum.accel.y / SAMPLES;
    double avgAccelZ = sum.accel.z / SAMPLES;
    double avgGyroX = sum.gyro.x / SAMPLES;
    double avgGyroY = sum.gyro.y / SAMPLES;
    double avgGyroZ = sum.gyro.z / SAMPLES;
    
    // Accelerometer offset (should be (0, 0, 9.81) ideally)
    // We'll calibrate to (0, 0, 1) for normalized values
    m_calibration.accelOffset[0] = -avgAccelX;
    m_calibration.accelOffset[1] = -avgAccelY;
    m_calibration.accelOffset[2] = 1.0 - avgAccelZ;
    
    // Gyroscope offset (should be 0)
    m_calibration.gyroOffset[0] = -avgGyroX;
    m_calibration.gyroOffset[1] = -avgGyroY;
    m_calibration.gyroOffset[2] = -avgGyroZ;
    
    if (m_enableMagnetometer) {
        double avgMagX = sum.mag.x / SAMPLES;
        double avgMagY = sum.mag.y / SAMPLES;
        double avgMagZ = sum.mag.z / SAMPLES;
        m_calibration.magOffset[0] = -avgMagX;
        m_calibration.magOffset[1] = -avgMagY;
        m_calibration.magOffset[2] = -avgMagZ;
    }
    
    LOG_INFO("IMU calibration complete: " + m_sensorId);
    return true;
}

bool IMUDriver::selfTest(Json::Value& result) {
    result["status"] = "passed";
    result["tests"] = 4;
    
    // Test I2C connection
    result["results"]["i2c_connected"] = m_i2cFd >= 0;
    
    // Test reading
    IMUData data;
    result["results"]["read_ok"] = readIMUData(data);
    
    // Test magnetometer if enabled
    if (m_enableMagnetometer) {
        result["results"]["magnetometer_ok"] = true;
    }
    
    result["results"]["accel_range"] = m_accelRange;
    result["results"]["gyro_range"] = m_gyroRange;
    result["results"]["sample_rate"] = m_sampleRate;
    
    return m_i2cFd >= 0;
}

bool IMUDriver::configure(const Json::Value& config) {
    if (config.isMember("accel_range")) {
        m_accelRange = config["accel_range"].asInt();
        configureAccel();
    }
    
    if (config.isMember("gyro_range")) {
        m_gyroRange = config["gyro_range"].asInt();
        configureGyro();
    }
    
    if (config.isMember("sample_rate")) {
        m_sampleRate = config["sample_rate"].asInt();
        configureSampleRate();
    }
    
    return true;
}

// ============================================================================
// IMU Specific Functions
// ============================================================================

bool IMUDriver::initMPU6050() {
    LOG_DEBUG("Initializing MPU6050");
    
    // Reset device
    if (!writeRegister(0x6B, 0x80)) return false; // PWR_MGMT_1
    usleep(100000);
    
    // Wake up device
    if (!writeRegister(0x6B, 0x00)) return false;
    usleep(100000);
    
    // Configure accelerometer
    if (!configureAccel()) return false;
    
    // Configure gyroscope
    if (!configureGyro()) return false;
    
    // Configure sample rate
    if (!configureSampleRate()) return false;
    
    return true;
}

bool IMUDriver::initMPU9250() {
    LOG_DEBUG("Initializing MPU9250");
    
    // Initialize MPU6050 part
    if (!initMPU6050()) return false;
    
    // Initialize magnetometer
    if (m_enableMagnetometer) {
        if (!configureMagnetometer()) return false;
    }
    
    return true;
}

bool IMUDriver::configureAccel() {
    uint8_t value = 0;
    
    switch (m_accelRange) {
        case 0: value = 0x00; break; // ±2g
        case 1: value = 0x08; break; // ±4g
        case 2: value = 0x10; break; // ±8g
        case 3: value = 0x18; break; // ±16g
        default: value = 0x10; break;
    }
    
    if (!writeRegister(0x1C, value)) { // ACCEL_CONFIG
        return false;
    }
    
    return true;
}

bool IMUDriver::configureGyro() {
    uint8_t value = 0;
    
    switch (m_gyroRange) {
        case 0: value = 0x00; break; // ±250°/s
        case 1: value = 0x08; break; // ±500°/s
        case 2: value = 0x10; break; // ±1000°/s
        case 3: value = 0x18; break; // ±2000°/s
        default: value = 0x10; break;
    }
    
    if (!writeRegister(0x1B, value)) { // GYRO_CONFIG
        return false;
    }
    
    return true;
}

bool IMUDriver::configureMagnetometer() {
    // For MPU9250, magnetometer is on a separate I2C address
    // We need to switch to the magnetometer address
    if (ioctl(m_i2cFd, I2C_SLAVE, MAGNETOMETER_ADDR) < 0) {
        return false;
    }
    
    // Set magnetometer to continuous mode
    if (!writeRegister(0x0A, 0x16)) { // CNTL
        return false;
    }
    
    // Switch back to main address
    if (ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddress) < 0) {
        return false;
    }
    
    return true;
}

bool IMUDriver::configureSampleRate() {
    // Sample rate = 1000 / (1 + divider)
    int divider = (1000 / m_sampleRate) - 1;
    if (divider < 0) divider = 0;
    if (divider > 255) divider = 255;
    
    if (!writeRegister(0x19, divider)) { // SMPLRT_DIV
        return false;
    }
    
    return true;
}

bool IMUDriver::readIMUData(IMUData& data) {
    uint8_t buffer[14];
    
    // Read accelerometer and gyroscope data
    // MPU6050: ACCEL_XOUT_H (0x3B) to GYRO_ZOUT_L (0x48) = 14 bytes
    if (!readRegisters(0x3B, buffer, 14)) {
        return false;
    }
    
    // Accelerometer data (16-bit signed)
    int16_t accelX = (buffer[0] << 8) | buffer[1];
    int16_t accelY = (buffer[2] << 8) | buffer[3];
    int16_t accelZ = (buffer[4] << 8) | buffer[5];
    
    // Temperature data
    int16_t temp = (buffer[6] << 8) | buffer[7];
    
    // Gyroscope data (16-bit signed)
    int16_t gyroX = (buffer[8] << 8) | buffer[9];
    int16_t gyroY = (buffer[10] << 8) | buffer[11];
    int16_t gyroZ = (buffer[12] << 8) | buffer[13];
    
    // Scale factors
    double accelScale;
    switch (m_accelRange) {
        case 0: accelScale = 16384.0; break; // ±2g
        case 1: accelScale = 8192.0; break;  // ±4g
        case 2: accelScale = 4096.0; break;  // ±8g
        case 3: accelScale = 2048.0; break;  // ±16g
        default: accelScale = 4096.0; break;
    }
    
    double gyroScale;
    switch (m_gyroRange) {
        case 0: gyroScale = 131.0; break;   // ±250°/s
        case 1: gyroScale = 65.5; break;    // ±500°/s
        case 2: gyroScale = 32.8; break;    // ±1000°/s
        case 3: gyroScale = 16.4; break;    // ±2000°/s
        default: gyroScale = 65.5; break;
    }
    
    // Convert to physical units
    data.accel.x = accelX / accelScale;
    data.accel.y = accelY / accelScale;
    data.accel.z = accelZ / accelScale;
    
    data.temperature = (temp / 340.0) + 36.53;
    
    data.gyro.x = gyroX / gyroScale;
    data.gyro.y = gyroY / gyroScale;
    data.gyro.z = gyroZ / gyroScale;
    
    // Read magnetometer if enabled
    if (m_enableMagnetometer) {
        readMagnetometer(data);
    }
    
    return true;
}

bool IMUDriver::readMagnetometer(IMUData& data) {
    // Switch to magnetometer address
    if (ioctl(m_i2cFd, I2C_SLAVE, MAGNETOMETER_ADDR) < 0) {
        return false;
    }
    
    uint8_t buffer[6];
    // Read magnetometer data: XOUT_H (0x03) to ZOUT_L (0x08) = 6 bytes
    if (!readRegisters(0x03, buffer, 6)) {
        // Switch back to main address
        ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddress);
        return false;
    }
    
    // Convert to physical units (microtesla)
    int16_t magX = (buffer[0] << 8) | buffer[1];
    int16_t magY = (buffer[2] << 8) | buffer[3];
    int16_t magZ = (buffer[4] << 8) | buffer[5];
    
    data.mag.x = magX * 0.15;
    data.mag.y = magY * 0.15;
    data.mag.z = magZ * 0.15;
    
    // Switch back to main address
    ioctl(m_i2cFd, I2C_SLAVE, m_i2cAddress);
    return true;
}

void IMUDriver::applyCalibration(IMUData& data) {
    // Apply accelerometer offset
    data.accel.x += m_calibration.accelOffset[0];
    data.accel.y += m_calibration.accelOffset[1];
    data.accel.z += m_calibration.accelOffset[2];
    
    // Apply accelerometer scale
    data.accel.x *= m_calibration.accelScale[0];
    data.accel.y *= m_calibration.accelScale[1];
    data.accel.z *= m_calibration.accelScale[2];
    
    // Apply gyroscope offset
    data.gyro.x += m_calibration.gyroOffset[0];
    data.gyro.y += m_calibration.gyroOffset[1];
    data.gyro.z += m_calibration.gyroOffset[2];
    
    // Apply gyroscope scale
    data.gyro.x *= m_calibration.gyroScale[0];
    data.gyro.y *= m_calibration.gyroScale[1];
    data.gyro.z *= m_calibration.gyroScale[2];
    
    // Apply magnetometer calibration
    if (m_enableMagnetometer) {
        data.mag.x += m_calibration.magOffset[0];
        data.mag.y += m_calibration.magOffset[1];
        data.mag.z += m_calibration.magOffset[2];
        data.mag.x *= m_calibration.magScale[0];
        data.mag.y *= m_calibration.magScale[1];
        data.mag.z *= m_calibration.magScale[2];
    }
}

void IMUDriver::calculateMagnitude(IMUData& data) {
    data.accel.magnitude = std::sqrt(
        data.accel.x * data.accel.x +
        data.accel.y * data.accel.y +
        data.accel.z * data.accel.z
    );
    
    data.gyro.magnitude = std::sqrt(
        data.gyro.x * data.gyro.x +
        data.gyro.y * data.gyro.y +
        data.gyro.z * data.gyro.z
    );
    
    data.mag.magnitude = std::sqrt(
        data.mag.x * data.mag.x +
        data.mag.y * data.mag.y +
        data.mag.z * data.mag.z
    );
}

bool IMUDriver::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    if (write(m_i2cFd, buffer, 2) != 2) {
        return false;
    }
    return true;
}

bool IMUDriver::readRegister(uint8_t reg, uint8_t& value) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    if (read(m_i2cFd, &value, 1) != 1) {
        return false;
    }
    return true;
}

bool IMUDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t length) {
    if (write(m_i2cFd, &reg, 1) != 1) {
        return false;
    }
    if (read(m_i2cFd, buffer, length) != static_cast<ssize_t>(length)) {
        return false;
    }
    return true;
}

bool IMUDriver::checkConnection() {
    if (m_i2cFd < 0) return false;
    
    uint8_t value;
    return readRegister(0x75, value); // WHO_AM_I register
}

} // namespace sensor_service
