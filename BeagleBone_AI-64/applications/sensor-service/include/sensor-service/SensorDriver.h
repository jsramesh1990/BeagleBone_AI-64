#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <string>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include "SensorData.h"

namespace sensor_service {

/**
 * @brief Base class for all sensor drivers
 */
class SensorDriver {
public:
    using DataCallback = std::function<void(const SensorReading&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using StatusCallback = std::function<void(bool connected)>;
    
    /**
     * @brief Constructor
     * @param sensorId Unique sensor identifier
     * @param type Sensor type
     */
    SensorDriver(const std::string& sensorId, SensorType type);
    
    /**
     * @brief Destructor
     */
    virtual ~SensorDriver();
    
    /**
     * @brief Initialize the sensor
     * @param config Configuration JSON
     * @return true on success
     */
    virtual bool initialize(const Json::Value& config) = 0;
    
    /**
     * @brief Start reading from sensor
     * @return true on success
     */
    virtual bool start() = 0;
    
    /**
     * @brief Stop reading from sensor
     * @return true on success
     */
    virtual bool stop() = 0;
    
    /**
     * @brief Read a single sample from sensor
     * @param reading Output reading
     * @return true on success
     */
    virtual bool readSample(SensorReading& reading) = 0;
    
    /**
     * @brief Calibrate the sensor
     * @param params Calibration parameters
     * @return true on success
     */
    virtual bool calibrate(const Json::Value& params) = 0;
    
    /**
     * @brief Run self-test
     * @param result Test result output
     * @return true if test passed
     */
    virtual bool selfTest(Json::Value& result) = 0;
    
    /**
     * @brief Configure the sensor
     * @param config Configuration JSON
     * @return true on success
     */
    virtual bool configure(const Json::Value& config) = 0;
    
    /**
     * @brief Get sensor ID
     */
    std::string getSensorId() const { return m_sensorId; }
    
    /**
     * @brief Get sensor type
     */
    SensorType getType() const { return m_type; }
    
    /**
     * @brief Check if sensor is connected
     */
    bool isConnected() const { return m_connected; }
    
    /**
     * @brief Check if sensor is running
     */
    bool isRunning() const { return m_running; }
    
    /**
     * @brief Get last reading
     */
    SensorReading getLastReading() const;
    
    /**
     * @brief Set data callback
     */
    void setDataCallback(DataCallback callback) { m_dataCallback = callback; }
    
    /**
     * @brief Set error callback
     */
    void setErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }
    
    /**
     * @brief Set status callback
     */
    void setStatusCallback(StatusCallback callback) { m_statusCallback = callback; }
    
    /**
     * @brief Get sensor name
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Get sensor version
     */
    virtual std::string getVersion() const = 0;

protected:
    /**
     * @brief Update connection status
     */
    void setConnected(bool connected);
    
    /**
     * @brief Emit data reading
     */
    void emitData(const SensorReading& reading);
    
    /**
     * @brief Emit error
     */
    void emitError(const std::string& error);
    
    /**
     * @brief Get current timestamp
     */
    std::chrono::system_clock::time_point getCurrentTime() const;
    
    /**
     * @brief Generate sequence number
     */
    uint64_t nextSequence();

    std::string m_sensorId;
    SensorType m_type;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_running;
    SensorReading m_lastReading;
    mutable std::mutex m_mutex;
    std::atomic<uint64_t> m_sequence;
    
    DataCallback m_dataCallback;
    ErrorCallback m_errorCallback;
    StatusCallback m_statusCallback;
    
    std::thread m_readThread;
    std::atomic<bool> m_shouldStop;
};

} // namespace sensor_service

#endif // SENSOR_DRIVER_H
