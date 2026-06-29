#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H

#include "../include/sensor-service/SensorDriver.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace sensor_service {

/**
 * @brief IMU sensor driver (MPU6050/MPU9250)
 * 
 * Supports accelerometer, gyroscope, and magnetometer
 * with I2C communication
 */
class IMUDriver : public SensorDriver {
public:
    explicit IMUDriver(const std::string& sensorId);
    virtual ~IMUDriver();
    
    // SensorDriver interface
    virtual bool initialize(const Json::Value& config) override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool readSample(SensorReading& reading) override;
    virtual bool calibrate(const Json::Value& params) override;
    virtual bool selfTest(Json::Value& result) override;
    virtual bool configure(const Json::Value& config) override;
    virtual std::string getName() const override { return "IMU (MPU6050)"; }
    virtual std::string getVersion() const override { return "1.0.0"; }

private:
    // I2C communication
    int m_i2cFd;
    std::string m_i2cDevice;
    int m_i2cAddress;
    
    // Configuration
    int m_accelRange;      // 0: ±2g, 1: ±4g, 2: ±8g, 3: ±16g
    int m_gyroRange;       // 0: ±250°/s, 1: ±500°/s, 2: ±1000°/s, 3: ±2000°/s
    int m_sampleRate;      // Hz
    bool m_enableMagnetometer;
    
    // Calibration offsets
    struct CalibrationOffsets {
        double accelOffset[3];
        double gyroOffset[3];
        double magOffset[3];
        double accelScale[3];
        double gyroScale[3];
        double magScale[3];
    } m_calibration;
    
    // I2C read/write
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t& value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t length);
    
    // Sensor specific functions
    bool initMPU6050();
    bool initMPU9250();
    bool readIMUData(IMUData& data);
    bool readMagnetometer(IMUData& data);
    bool configureAccel();
    bool configureGyro();
    bool configureMagnetometer();
    bool configureSampleRate();
    void applyCalibration(IMUData& data);
    
    // Utility functions
    void calculateMagnitude(IMUData& data);
    bool checkConnection();
    
    static const int MPU6050_ADDR = 0x68;
    static const int MPU9250_ADDR = 0x68;
    static const int MAGNETOMETER_ADDR = 0x0C;
    static const int I2C_DEVICE = 0;
};

} // namespace sensor_service

#endif // IMU_DRIVER_H
