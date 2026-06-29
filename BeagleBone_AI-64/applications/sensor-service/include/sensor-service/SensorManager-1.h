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
#include <chrono>
#include <json/json.h>
#include "SensorData.h"
#include "SensorDriver.h"
#include "SensorEvents.h"
#include "SensorConfig.h"

namespace sensor_service {

/**
 * @brief Sensor Manager - Manages all sensor drivers
 * 
 * The SensorManager is the central component of the sensor service.
 * It handles:
 * - Sensor discovery and registration
 * - Data collection from all sensors
 * - Health monitoring and auto-recovery
 * - Event generation and distribution
 * - Configuration management
 * - Statistics tracking
 * 
 * Thread Safety:
 * - All public methods are thread-safe
 * - Internal threads handle data collection and health monitoring
 * - Callbacks are executed in the context of the calling thread
 * 
 * Usage Example:
 * @code
 * auto& manager = SensorManager::getInstance();
 * 
 * // Initialize with configuration
 * Json::Value config;
 * config["collection_interval_ms"] = 1000;
 * config["auto_discover"] = true;
 * manager.initialize(config);
 * 
 * // Set callbacks
 * manager.setDataCallback([](const SensorReading& reading) {
 *     // Handle sensor data
 * });
 * 
 * // Start the manager
 * manager.start();
 * 
 * // Later, stop the manager
 * manager.stop();
 * @endcode
 */
class SensorManager {
public:
    /**
     * @brief Callback types
     */
    using SensorCallback = std::function<void(const SensorReading&)>;
    using EventCallback = std::function<void(const SensorEvent&)>;
    
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton SensorManager
     */
    static SensorManager& getInstance();
    
    // ========================================================================
    // Lifecycle Management
    // ========================================================================
    
    /**
     * @brief Initialize the sensor manager
     * @param config Configuration JSON with the following options:
     *        - collection_interval_ms: Data collection interval (default: 1000)
     *        - health_check_interval_ms: Health check interval (default: 10000)
     *        - auto_discover: Auto-discover sensors (default: true)
     *        - config_path: Path to load/save configuration
     *        - sensors: Array of sensor configurations
     * @return true on success
     */
    bool initialize(const Json::Value& config = Json::nullValue);
    
    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_initialized; }
    
    /**
     * @brief Start the sensor manager
     * Starts data collection and health monitoring threads
     * @return true on success
     */
    bool start();
    
    /**
     * @brief Stop the sensor manager
     * Stops all threads and sensors
     * @return true on success
     */
    bool stop();
    
    /**
     * @brief Check if running
     * @return true if running
     */
    bool isRunning() const { return m_running; }
    
    // ========================================================================
    // Sensor Registration
    // ========================================================================
    
    /**
     * @brief Register a sensor driver
     * @param driver Sensor driver instance (ownership transferred)
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
     * @return Vector of sensor IDs
     */
    std::vector<std::string> getSensorIds() const;
    
    /**
     * @brief Get sensors by type
     * @param type Sensor type
     * @return Vector of sensor drivers
     */
    std::vector<SensorDriver*> getSensorsByType(SensorType type);
    
    /**
     * @brief Get number of registered sensors
     * @return Sensor count
     */
    size_t getSensorCount() const { return m_sensors.size(); }
    
    /**
     * @brief Check if a sensor is registered
     * @param sensorId Sensor ID
     * @return true if registered
     */
    bool hasSensor(const std::string& sensorId) const {
        return m_sensors.find(sensorId) != m_sensors.end();
    }
    
    // ========================================================================
    // Sensor Operations
    // ========================================================================
    
    /**
     * @brief Read a single sample from a sensor
     * @param sensorId Sensor ID
     * @param reading Output reading
     * @param timeoutMs Timeout in milliseconds (0 = no timeout)
     * @return true on success
     */
    bool readSensor(const std::string& sensorId, SensorReading& reading, 
                    int timeoutMs = 5000);
    
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
     * @param result Test result output
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
     * @brief Enable or disable a sensor
     * @param sensorId Sensor ID
     * @param enabled Enable/disable
     * @return true on success
     */
    bool setSensorEnabled(const std::string& sensorId, bool enabled);
    
    /**
     * @brief Get sensor status
     * @param sensorId Sensor ID
     * @return JSON status information
     */
    Json::Value getSensorStatus(const std::string& sensorId) const;
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * @brief Set global data callback
     * @param callback Callback function called for each sensor reading
     *        The callback is executed in the context of the calling thread
     */
    void setDataCallback(SensorCallback callback);
    
    /**
     * @brief Set global event callback
     * @param callback Callback function called for each sensor event
     *        The callback is executed in the context of the calling thread
     */
    void setEventCallback(EventCallback callback);
    
    /**
     * @brief Remove data callback
     */
    void removeDataCallback() { m_dataCallback = nullptr; }
    
    /**
     * @brief Remove event callback
     */
    void removeEventCallback() { m_eventCallback = nullptr; }
    
    // ========================================================================
    // Discovery
    // ========================================================================
    
    /**
     * @brief Discover sensors on the system
     * @param timeoutMs Discovery timeout in milliseconds
     * @return Number of sensors discovered
     */
    int discoverSensors(int timeoutMs = 10000);
    
    /**
     * @brief Enable or disable auto-discovery
     * @param enable Enable auto-discovery
     */
    void setAutoDiscovery(bool enable) { m_autoDiscover = enable; }
    
    /**
     * @brief Check if auto-discovery is enabled
     * @return true if enabled
     */
    bool isAutoDiscoveryEnabled() const { return m_autoDiscover; }
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    /**
     * @brief Get sensor statistics
     * @return JSON statistics including:
     *        - readings_total: Total readings
     *        - errors: Error count
     *        - timeouts: Timeout count
     *        - sensor_count: Number of sensors
     *        - uptime_seconds: Uptime in seconds
     *        - sensors: Per-sensor statistics
     */
    Json::Value getStats() const;
    
    /**
     * @brief Reset statistics
     */
    void resetStats();
    
    /**
     * @brief Get last reading for a sensor
     * @param sensorId Sensor ID
     * @return Last reading (empty if none)
     */
    SensorReading getLastReading(const std::string& sensorId) const;
    
    /**
     * @brief Get all last readings
     * @return Map of sensor ID to last reading
     */
    std::map<std::string, SensorReading> getAllLastReadings() const;
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Save sensor configuration to file
     * @param path File path
     * @return true on success
     */
    bool saveConfig(const std::string& path) const;
    
    /**
     * @brief Load sensor configuration from file
     * @param path File path
     * @return true on success
     */
    bool loadConfig(const std::string& path);
    
    /**
     * @brief Get current configuration
     * @return Configuration JSON
     */
    Json::Value getConfig() const { return m_config; }
    
    /**
     * @brief Set configuration
     * @param config Configuration JSON
     */
    void setConfig(const Json::Value& config) { m_config = config; }
    
    /**
     * @brief Get configuration path
     * @return Configuration file path
     */
    std::string getConfigPath() const { return m_configPath; }
    
    /**
     * @brief Set configuration path
     * @param path Configuration file path
     */
    void setConfigPath(const std::string& path) { m_configPath = path; }
    
    // ========================================================================
    // Utility Functions
    // ========================================================================
    
    /**
     * @brief Get sensor type name
     * @param type Sensor type
     * @return Human-readable sensor type name
     */
    static std::string sensorTypeToString(SensorType type);
    
    /**
     * @brief Get sensor type from name
     * @param name Sensor type name
     * @return Sensor type
     */
    static SensorType sensorTypeFromString(const std::string& name);

private:
    // ========================================================================
    // Private Constructor/Destructor
    // ========================================================================
    
    SensorManager();
    ~SensorManager();
    SensorManager(const SensorManager&) = delete;
    SensorManager& operator=(const SensorManager&) = delete;
    
    // ========================================================================
    // Internal Methods
    // ========================================================================
    
    void dataCollectionLoop();
    void checkSensorHealth();
    void processSensorData(const SensorReading& reading);
    void emitEvent(const SensorEvent& event);
    void updateStats(const SensorReading& reading);
    
    bool loadDriversFromConfig(const Json::Value& config);
    Json::Value saveDriversToConfig() const;
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    
    // State
    std::map<std::string, std::unique_ptr<SensorDriver>> m_sensors;
    std::map<std::string, SensorReading> m_lastReadings;
    std::map<std::string, std::map<std::string, double>> m_lastValues;
    mutable std::mutex m_mutex;
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;
    
    // Threads
    std::thread m_collectionThread;
    std::thread m_healthThread;
    
    // Configuration
    Json::Value m_config;
    std::string m_configPath;
    int m_collectionIntervalMs;
    int m_healthCheckIntervalMs;
    bool m_autoDiscover;
    
    // Callbacks
    SensorCallback m_dataCallback;
    EventCallback m_eventCallback;
    
    // Statistics
    struct Stats {
        std::atomic<uint64_t> readingsTotal{0};
        std::atomic<uint64_t> errors{0};
        std::atomic<uint64_t> timeouts{0};
        std::chrono::steady_clock::time_point startTime;
        std::map<SensorType, std::atomic<uint64_t>> readingsByType;
        
        Stats() {
            startTime = std::chrono::steady_clock::now();
        }
    };
    Stats m_stats;
    mutable std::mutex m_statsMutex;
    
    // Friends
    friend class SensorManagerTest;
};

} // namespace sensor_service

#endif // SENSOR_MANAGER_H
