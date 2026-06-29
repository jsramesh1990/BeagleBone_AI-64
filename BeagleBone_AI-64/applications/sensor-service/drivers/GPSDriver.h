#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include "../include/sensor-service/SensorDriver.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

namespace sensor_service {

/**
 * @brief GPS sensor driver (NMEA protocol)
 * 
 * Supports standard NMEA GPS receivers over UART
 */
class GPSDriver : public SensorDriver {
public:
    explicit GPSDriver(const std::string& sensorId);
    virtual ~GPSDriver();
    
    // SensorDriver interface
    virtual bool initialize(const Json::Value& config) override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool readSample(SensorReading& reading) override;
    virtual bool calibrate(const Json::Value& params) override;
    virtual bool selfTest(Json::Value& result) override;
    virtual bool configure(const Json::Value& config) override;
    virtual std::string getName() const override { return "GPS (NMEA)"; }
    virtual std::string getVersion() const override { return "1.0.0"; }

private:
    // Serial communication
    int m_serialFd;
    std::string m_serialDevice;
    int m_baudRate;
    
    // GPS state
    struct GPSState {
        bool hasFix;
        int satellites;
        double latitude;
        double longitude;
        double altitude;
        double speed;
        double heading;
        double horizontalAccuracy;
        double verticalAccuracy;
        std::string timestamp;
        std::chrono::system_clock::time_point lastUpdate;
    } m_gpsState;
    
    // NMEA parsing
    bool parseNMEA(const std::string& line);
    bool parseGGA(const std::string& fields);
    bool parseGLL(const std::string& fields);
    bool parseRMC(const std::string& fields);
    bool parseVTG(const std::string& fields);
    bool parseGSA(const std::string& fields);
    bool parseGSV(const std::string& fields);
    
    // Utility functions
    double parseNMEACoordinate(const std::string& coord, const std::string& dir);
    bool checkConnection();
    void resetState();
    
    // NMEA constants
    static const int NMEA_MAX_LINE = 256;
    static const int GPS_TIMEOUT_MS = 5000;
};

} // namespace sensor_service

#endif // GPS_DRIVER_H
