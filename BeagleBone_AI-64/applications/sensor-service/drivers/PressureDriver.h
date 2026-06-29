#ifndef PRESSURE_DRIVER_H
#define PRESSURE_DRIVER_H

#include "../include/sensor-service/SensorDriver.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace sensor_service {

/**
 * @brief Pressure sensor driver (BMP180/BMP280)
 * 
 * Supports BMP180 and BMP280 pressure sensors
 */
class PressureDriver : public SensorDriver {
public:
    explicit PressureDriver(const std::string& sensorId);
    virtual ~PressureDriver();
    
    // SensorDriver interface
    virtual bool initialize(const Json::Value& config) override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool readSample(SensorReading& reading) override;
    virtual bool calibrate(const Json::Value& params) override;
    virtual bool selfTest(Json::Value& result) override;
    virtual bool configure(const Json::Value& config) override;
    virtual std::string getName() const override { return "Pressure Sensor"; }
    virtual std::string getVersion() const override { return "1.0.0"; }

private:
    // Sensor type
    enum SensorType {
        BMP180,
        BMP280,
        UNKNOWN
    };
    
    // I2C communication
    int m_i2cFd;
    std::string m_i2cDevice;
    int m_i2cAddress;
    SensorType m_sensorType;
    
    // Calibration data
    struct CalibrationData {
        // BMP180
        int16_t AC1, AC2, AC3;
        uint16_t AC4, AC5, AC6;
        int16_t B1, B2;
        int16_t MB, MC, MD;
        
        // BMP280
        uint16_t dig_T1;
        int16_t dig_T2, dig_T3;
        uint16_t dig_P1;
        int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    } m_calibration;
    
    // Configuration
    int m_oversampling;   // 0-3 for BMP180, 0-4 for BMP280
    bool m_seaLevel;      // Calculate sea-level pressure
    
    // Sensor specific functions
    bool detectSensor();
    bool initBMP180();
    bool initBMP280();
    bool readCalibrationBMP180();
    bool readCalibrationBMP280();
    
    bool readBMP180(PressureData& data);
    bool readBMP280(PressureData& data);
    bool readRawBMP180(int32_t& UT, int32_t& UP);
    bool readRawBMP280(int32_t& T, int32_t& P);
    
    // Utility functions
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t& value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t length);
    bool checkConnection();
    double calculateAltitude(double pressure, double seaLevelPressure);
};

} // namespace sensor_service

#endif // PRESSURE_DRIVER_H
