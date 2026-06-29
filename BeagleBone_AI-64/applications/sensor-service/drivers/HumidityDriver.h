#ifndef HUMIDITY_DRIVER_H
#define HUMIDITY_DRIVER_H

#include "../include/sensor-service/SensorDriver.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace sensor_service {

/**
 * @brief Humidity sensor driver (DHT11/DHT22/HTU21D)
 * 
 * Supports DHT11, DHT22, and HTU21D humidity sensors
 */
class HumidityDriver : public SensorDriver {
public:
    explicit HumidityDriver(const std::string& sensorId);
    virtual ~HumidityDriver();
    
    // SensorDriver interface
    virtual bool initialize(const Json::Value& config) override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool readSample(SensorReading& reading) override;
    virtual bool calibrate(const Json::Value& params) override;
    virtual bool selfTest(Json::Value& result) override;
    virtual bool configure(const Json::Value& config) override;
    virtual std::string getName() const override { return "Humidity Sensor"; }
    virtual std::string getVersion() const override { return "1.0.0"; }

private:
    // Sensor type
    enum SensorType {
        DHT11,
        DHT22,
        HTU21D,
        UNKNOWN
    };
    
    // Communication
    int m_gpioPin;         // GPIO pin for DHT sensors
    std::string m_i2cDevice;
    int m_i2cAddress;      // For HTU21D
    int m_i2cFd;
    SensorType m_sensorType;
    
    // Configuration
    double m_offset;       // Calibration offset
    bool m_fahrenheit;     // Output in Fahrenheit
    
    // DHT specific
    static const int DHT_TIMEOUT = 100;  // Microseconds
    
    // Sensor specific functions
    bool detectSensor();
    bool initDHT();
    bool initHTU21D();
    
    bool readDHT11(HumidityData& data);
    bool readDHT22(HumidityData& data);
    bool readHTU21D(HumidityData& data);
    
    // DHT communication
    bool readDHT(int& humidity, int& temperature);
    bool readDHTData(uint8_t* data);
    bool waitForPinState(int state, int timeout);
    
    // HTU21D communication
    bool writeRegister(uint8_t reg);
    bool readRegister(uint8_t reg, uint16_t& value);
    
    // Utility functions
    bool checkConnection();
    double calculateDewPoint(double temperature, double humidity);
    double calculateAbsoluteHumidity(double temperature, double humidity);
};

} // namespace sensor_service

#endif // HUMIDITY_DRIVER_H
