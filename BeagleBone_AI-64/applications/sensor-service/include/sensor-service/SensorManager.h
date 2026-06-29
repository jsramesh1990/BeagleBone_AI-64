#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <json/json.h>
#include "SensorData.h"
#include "SensorDriver.h"
#include "SensorEvents.h"

namespace sensor_service {

/**
 * @brief Sensor Manager - manages all sensor drivers
 * 
 * Responsible for sensor discovery, initialization,
 * data collection, and event management
 */
class SensorManager {
public:
    using SensorCallback = std::function<void(const SensorReading&)>;
    using EventCallback = std::function<void(const SensorEvent&)>;
    
    /**
     * @brief Get singleton instance
     */
    static SensorManager& getInstance();
    
    /**
     * @brief Initialize the sensor manager
     * @param config Configuration JSON
     * @return true on success
     */
    bool initialize(const Json::Value& config = Json::nullValue);
    
    /**
     * @brief Start all sensors
     * @return true on success
     */
    bool start();
    
    /**
     * @brief Stop all sensors
     * @return true on success
     */
    bool stop();
    
    /**
     * @brief Register a sensor driver
     * @param driver Sensor driver instance
     * @return true on success
     */
    bool registerSensor(std::unique_ptr<SensorDriver> driver);
    
    /**
     * @brief Unregister a sensor driver
     * @param sensorId Sensor ID
     * @return true on success
     */
    bool unregisterSensor(const std::string& sensorId);
    
    /**
     * @brief Get a sensor driver by ID
     * @param sensorId Sensor ID
     * @return Sensor driver pointer or nullptr
     */
    SensorDriver* getSensor(const std::string& sensorId);
    
    /**
     * @brief Get all sensor IDs
     */
    std::vector<std::string> getSensorIds() const;
    
    /**
     * @brief Get sensors by type
     * @param type Sensor type
     */
    std::vector<SensorDriver*> getSensorsByType(SensorType type);
    
    /**
     * @brief Read a single sample from a sensor
     * @param sensorId Sensor ID
     * @param reading Output reading
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool readSensor(const std::string& sensorId, SensorReading& reading, int timeoutMs = 5000);
    
    /**
     * @brief Read all sensors
     * @return Vector of readings
     */
    std::vector<SensorReading> readAllSensors();
    
    /**
     * @brief Calibrate a sensor
     * @param sensorId Sensor ID
     * @param params Calibration parameters
     * @return true on success
     */
    bool calibrateSensor(const std::string& sensorId, const Json::Value& params);
    
    /**
     * @brief Run self-test on a sensor
     * @param sensorId Sensor ID
     * @param result Test result
     * @return true if test passed
     */
    bool selfTestSensor(const std::string& sensorId, Json::Value& result);
    
    /**
     * @brief Configure a sensor
     * @param sensorId Sensor ID
     * @param config Configuration JSON
     * @return true on success
     */
    bool configureSensor(const std::string& sensorId, const Json::Value& config);
    
    /**
     * @brief Set global data callback
     * @param callback Callback function
     */
    void setDataCallback(SensorCallback callback);
    
    /**
     * @brief Set global event callback
     * @param callback Callback function
     */
    void setEventCallback(EventCallback callback);
    
    /**
     * @brief Get sensor statistics
     */
    Json::Value getStats() const;
    
    /**
     * @brief Reset statistics
     */
    void resetStats();
    
    /**
     * @brief Save sensor configuration
     * @param path File path
     * @return true on success
     */
    bool saveConfig(const std::string& path) const;
    
    /**
     * @brief Load sensor configuration
     * @param path File path
     * @return true on success
     */
    bool loadConfig(const std::string& path);
    
    /**
     * @brief Discover sensors on the system
     * @param timeoutMs Discovery timeout
     * @return Number of sensors discovered
     */
    int discoverSensors(int timeoutMs = 10000);

private:
    SensorManager();
    ~SensorManager();
    SensorManager(const SensorManager&) = delete;
    SensorManager& operator=(const SensorManager&) = delete;
    
    void dataCollectionLoop();
    void checkSensorHealth();
    void processSensorData(const SensorReading& reading);
    void emitEvent(const SensorEvent& event);
    void updateStats(const SensorReading& reading);
    
    bool loadDriversFromConfig(const Json::Value& config);
    Json::Value saveDriversToConfig() const;
    
    std::map<std::string, std::unique_ptr<SensorDriver>> m_sensors;
    std::map<std::string, std::map<std::string, double>> m_lastValues;
    mutable std::mutex m_mutex;
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;
    std::thread m_collectionThread;
    std::thread m_healthThread;
    Json::Value m_config;
    
    SensorCallback m_dataCallback;
    EventCallback m_eventCallback;
    
    // Statistics
    struct Stats {
        std::atomic<uint64_t> readingsTotal{0};
        std::atomic<uint64_t> errors{0};
        std::atomic<uint64_t> timeouts{0};
        std::chrono::steady_clock::time_point startTime;
        std::map<SensorType, std::atomic<uint64_t>> readingsByType;
    };
    Stats m_stats;
    mutable std::mutex m_statsMutex;
    
    // Configuration
    std::string m_configPath;
    int m_collectionIntervalMs;
    int m_healthCheckIntervalMs;
    bool m_autoDiscover;
};

} // namespace sensor_service

#endif // SENSOR_MANAGER_H
