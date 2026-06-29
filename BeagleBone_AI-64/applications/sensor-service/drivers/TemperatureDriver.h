#ifndef TEMPERATURE_DRIVER_H
#define TEMPERATURE_DRIVER_H

#include "../include/sensor-service/SensorDriver.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace sensor_service {

/**
 * @brief Temperature sensor driver (various I2C sensors)
 * 
 * Supports TMP102, TMP112, LM75, DS18B20
 */
class TemperatureDriver : public SensorDriver {
public:
    explicit TemperatureDriver(const std::string& sensorId);
    virtual ~TemperatureDriver();
    
    // SensorDriver interface
    virtual bool initialize(const Json::Value& config) override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool readSample(SensorReading& reading) override;
    virtual bool calibrate(const Json::Value& params) override;
    virtual bool selfTest(Json::Value& result) override;
    virtual bool configure(const Json::Value& config) override;
    virtual std::string getName() const override { return "Temperature Sensor"; }
    virtual std::string getVersion() const override { return "1.0.0"; }

private:
    // Sensor type
    enum SensorType {
        TMP102,
        TMP112,
        LM75,
        DS18B20,
        UNKNOWN
    };
    
    // I2C communication
    int m_i2cFd;
    std::string m_i2cDevice;
    int m_i2cAddress;
    SensorType m_sensorType;
    
    // Configuration
    int m_resolution;     // 9-12 bits
    double m_offset;      // Calibration offset
    bool m_fahrenheit;    // Output in Fahrenheit
    
    // Sensor specific functions
    bool detectSensor();
    bool initTMP102();
    bool initTMP112();
    bool initLM75();
    bool initDS18B20();
    
    bool readTMP102(double& temp);
    bool readTMP112(double& temp);
    bool readLM75(double& temp);
    bool readDS18B20(double& temp);
    
    // Utility functions
    bool writeRegister(uint8_t reg, uint16_t value);
    bool readRegister(uint8_t reg, uint16_t& value);
    bool checkConnection();
    double celsiusToFahrenheit(double celsius);
};

} // namespace sensor_service

#endif // TEMPERATURE_DRIVER_H
