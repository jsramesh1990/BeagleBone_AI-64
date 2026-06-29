#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <json/json.h>
#include "SensorData.h"
#include "SensorDriver.h"
#include "SensorEvents.h"
#include "SensorConfig.h"

namespace sensor_service {

/**
 * @brief Sensor Service - High-level service interface
 * 
 * The SensorService provides a high-level interface for the sensor system.
 * It wraps the SensorManager and provides additional service-level features:
 * - Service lifecycle management (initialize, start, stop)
 * - High-level sensor operations
 * - Service-level event handling
 * - Integration with external systems (IPC, logging, etc.)
 * 
 * This is the main entry point for external applications to interact with
 * the sensor system.
 * 
 * Usage Example:
 * @code
 * auto& service = SensorService::getInstance();
 * 
 * // Configure and initialize
 * Json::Value config;
 * config["collection_interval_ms"] = 1000;
 * config["auto_discover"] = true;
 * service.initialize(config);
 * 
 * // Start the service
 * service.start();
 * 
 * // Get sensor data
 * SensorReading reading;
 * if (service.readSensor("imu-001", reading)) {
 *     // Use reading
 * }
 * 
 * // Stop the service
 * service.stop();
 * @endcode
 */
class SensorService {
public:
    /**
     * @brief Callback types
     */
    using SensorCallback = std::function<void(const SensorReading&)>;
    using EventCallback = std::function<void(const SensorEvent&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using StatusCallback = std::function<void(bool running)>;
    
    /**
     * @brief Service status
     */
    enum class Status {
        STOPPED,
        INITIALIZING,
        RUNNING,
        ERROR,
        SHUTTING_DOWN
    };
    
    /**
     * @brief Service information
     */
    struct ServiceInfo {
        std::string version;
        std::string name;
        std::string description;
        std::chrono::system_clock::time_point startTime;
        Status status;
        std::map<std::string, std::string> metadata;
        
        ServiceInfo() 
            : version("1.0.0"), 
              name("Sensor Service"),
              description("BeagleBone AI-64 Sensor Service"),
              status(Status::STOPPED) {}
        
        Json::Value toJson() const {
            Json::Value json;
            json["version"] = version;
            json["name"] = name;
            json["description"] = description;
            json["startTime"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                startTime.time_since_epoch()).count();
            json["status"] = static_cast<int>(status);
            json["statusText"] = statusToString(status);
            
            for (const auto& [key, value] : metadata) {
                json["metadata"][key] = value;
            }
            
            return json;
        }
        
        static std::string statusToString(Status status) {
            switch (status) {
                case Status::STOPPED: return "STOPPED";
                case Status::INITIALIZING: return "INITIALIZING";
                case Status::RUNNING: return "RUNNING";
                case Status::ERROR: return "ERROR";
                case Status::SHUTTING_DOWN: return "SHUTTING_DOWN";
                default: return "UNKNOWN";
            }
        }
    };

public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton SensorService
     */
    static SensorService& getInstance();
    
    // ========================================================================
    // Service Lifecycle
    // ========================================================================
    
    /**
     * @brief Initialize the sensor service
     * @param config Configuration JSON with the following options:
     *        - collection_interval_ms: Data collection interval (default: 1000)
     *        - health_check_interval_ms: Health check interval (default: 10000)
     *        - auto_discover: Auto-discover sensors (default: true)
     *        - config_path: Path to load/save configuration
     *        - service_name: Service name (default: "Sensor Service")
     *        - version: Service version (default: "1.0.0")
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
     * @brief Start the sensor service
     * @return true on success
     */
    bool start();
    
    /**
     * @brief Stop the sensor service
     * @return true on success
     */
    bool stop();
    
    /**
     * @brief Restart the sensor service
     * @return true on success
     */
    bool restart();
    
    /**
     * @brief Check if running
     * @return true if running
     */
    bool isRunning() const { return m_status == Status::RUNNING; }
    
    /**
     * @brief Get service status
     * @return Current service status
     */
    Status getStatus() const { return m_status; }
    
    /**
     * @brief Get service info
     * @return Service information
     */
    ServiceInfo getServiceInfo() const;
    
    // ========================================================================
    // Sensor Operations
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
    size_t getSensorCount() const;
    
    /**
     * @brief Read a single sample from a sensor
     * @param sensorId Sensor ID
     * @param reading Output reading
     * @param timeoutMs Timeout in milliseconds
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
    
    /**
     * @brief Get all sensor statuses
     * @return JSON object with all sensor statuses
     */
    Json::Value getAllSensorStatuses() const;
    
    // ========================================================================
    // Data Access
    // ========================================================================
    
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
    
    /**
     * @brief Get historical readings for a sensor
     * @param sensorId Sensor ID
     * @param count Number of readings to retrieve
     * @param since Time since (optional)
     * @return Vector of readings
     */
    std::vector<SensorReading> getHistory(const std::string& sensorId, 
                                         size_t count = 100,
                                         std::chrono::system_clock::time_point since = 
                                             std::chrono::system_clock::time_point());
    
    /**
     * @brief Get sensor value history
     * @param sensorId Sensor ID
     * @param valueName Name of the value
     * @param count Number of values to retrieve
     * @return Vector of (timestamp, value) pairs
     */
    std::vector<std::pair<std::chrono::system_clock::time_point, double>> 
        getValueHistory(const std::string& sensorId, 
                       const std::string& valueName,
                       size_t count = 100);
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * @brief Set global data callback
     * @param callback Callback function called for each sensor reading
     */
    void setDataCallback(SensorCallback callback);
    
    /**
     * @brief Set global event callback
     * @param callback Callback function called for each sensor event
     */
    void setEventCallback(EventCallback callback);
    
    /**
     * @brief Set error callback
     * @param callback Callback function called on errors
     */
    void setErrorCallback(ErrorCallback callback);
    
    /**
     * @brief Set status callback
     * @param callback Callback function called on status changes
     */
    void setStatusCallback(StatusCallback callback);
    
    /**
     * @brief Remove callbacks
     */
    void removeDataCallback();
    void removeEventCallback();
    void removeErrorCallback();
    void removeStatusCallback();
    
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
    void setAutoDiscovery(bool enable);
    
    /**
     * @brief Check if auto-discovery is enabled
     * @return true if enabled
     */
    bool isAutoDiscoveryEnabled() const;
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    /**
     * @brief Get service statistics
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
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Save configuration to file
     * @param path File path
     * @return true on success
     */
    bool saveConfig(const std::string& path) const;
    
    /**
     * @brief Load configuration from file
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
    void setConfig(const Json::Value& config);
    
    // ========================================================================
    // Health & Monitoring
    // ========================================================================
    
    /**
     * @brief Perform health check
     * @return JSON health status
     */
    Json::Value healthCheck() const;
    
    /**
     * @brief Get health status
     * @return true if service is healthy
     */
    bool isHealthy() const;
    
    /**
     * @brief Set health threshold
     * @param threshold Health threshold (0.0-1.0)
     */
    void setHealthThreshold(double threshold) { m_healthThreshold = threshold; }
    
    /**
     * @brief Get health threshold
     * @return Health threshold
     */
    double getHealthThreshold() const { return m_healthThreshold; }
    
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
    
    /**
     * @brief Get service version
     * @return Version string
     */
    std::string getVersion() const { return m_serviceInfo.version; }
    
    /**
     * @brief Get service name
     * @return Service name
     */
    std::string getName() const { return m_serviceInfo.name; }

private:
    // ========================================================================
    // Private Constructor/Destructor
    // ========================================================================
    
    SensorService();
    ~SensorService();
    SensorService(const SensorService&) = delete;
    SensorService& operator=(const SensorService&) = delete;
    
    // ========================================================================
    // Internal Methods
    // ========================================================================
    
    void setStatus(Status newStatus);
    void onSensorData(const SensorReading& reading);
    void onSensorEvent(const SensorEvent& event);
    void onError(const std::string& error);
    void updateHealth();
    void checkTimeouts();
    void cleanup();
    void loadDrivers();
    void saveDrivers();
    bool validateConfig(const Json::Value& config) const;
    
    // ========================================================================
    // Member Variables
    // ========================================================================
    
    // Service state
    std::atomic<Status> m_status;
    std::atomic<bool> m_initialized;
    ServiceInfo m_serviceInfo;
    
    // Configuration
    Json::Value m_config;
    std::string m_configPath;
    int m_collectionIntervalMs;
    int m_healthCheckIntervalMs;
    bool m_autoDiscover;
    double m_healthThreshold;
    int m_maxHistorySize;
    
    // Sensor manager reference
    SensorManager& m_manager;
    
    // Callbacks
    SensorCallback m_dataCallback;
    EventCallback m_eventCallback;
    ErrorCallback m_errorCallback;
    StatusCallback m_statusCallback;
    
    // History
    std::map<std::string, std::vector<SensorReading>> m_history;
    mutable std::mutex m_historyMutex;
    
    // Health
    std::chrono::steady_clock::time_point m_lastHealthCheck;
    double m_healthScore;
    std::map<std::string, bool> m_sensorHealth;
    mutable std::mutex m_healthMutex;
    
    // Timeout tracking
    std::map<std::string, std::chrono::system_clock::time_point> m_lastActivity;
    mutable std::mutex m_activityMutex;
    
    // Friends
    friend class SensorServiceTest;
};

} // namespace sensor_service

#endif // SENSOR_SERVICE_H
