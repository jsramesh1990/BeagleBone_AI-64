#include "sensor-service/SensorManager.h"
#include "sensor-service/SensorEvents.h"
#include "logger/Logger.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>

// Include drivers
#include "drivers/IMUDriver.h"
#include "drivers/GPSDriver.h"
#include "drivers/TemperatureDriver.h"
#include "drivers/PressureDriver.h"
#include "drivers/HumidityDriver.h"

using namespace common;

namespace sensor_service {

// ============================================================================
// Singleton Implementation
// ============================================================================

SensorManager& SensorManager::getInstance() {
    static SensorManager instance;
    return instance;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

SensorManager::SensorManager() 
    : m_running(false), 
      m_initialized(false),
      m_collectionIntervalMs(1000),
      m_healthCheckIntervalMs(10000),
      m_autoDiscover(true) {
    
    m_stats.startTime = std::chrono::steady_clock::now();
    LOG_DEBUG("SensorManager created");
}

SensorManager::~SensorManager() {
    stop();
    LOG_DEBUG("SensorManager destroyed");
}

// ============================================================================
// Initialization
// ============================================================================

bool SensorManager::initialize(const Json::Value& config) {
    if (m_initialized) {
        LOG_WARN("SensorManager already initialized");
        return true;
    }
    
    m_config = config;
    
    // Load configuration
    if (config.isMember("collection_interval_ms")) {
        m_collectionIntervalMs = config["collection_interval_ms"].asInt();
    }
    if (config.isMember("health_check_interval_ms")) {
        m_healthCheckIntervalMs = config["health_check_interval_ms"].asInt();
    }
    if (config.isMember("auto_discover")) {
        m_autoDiscover = config["auto_discover"].asBool();
    }
    if (config.isMember("config_path")) {
        m_configPath = config["config_path"].asString();
    }
    
    // Load drivers from config if available
    if (config.isMember("sensors") && config["sensors"].isArray()) {
        loadDriversFromConfig(config["sensors"]);
    }
    
    // Load from file if path provided
    if (!m_configPath.empty()) {
        loadConfig(m_configPath);
    }
    
    m_initialized = true;
    LOG_INFO("SensorManager initialized");
    return true;
}

bool SensorManager::loadDriversFromConfig(const Json::Value& config) {
    if (!config.isArray()) {
        return false;
    }
    
    for (const auto& sensorConfig : config) {
        std::string type = sensorConfig.get("type", "unknown").asString();
        std::string id = sensorConfig.get("id", "").asString();
        
        if (id.empty()) {
            LOG_WARN("Sensor config missing ID, skipping");
            continue;
        }
        
        std::unique_ptr<SensorDriver> driver;
        
        // Create appropriate driver based on type
        if (type == "imu" || type == "IMU") {
            driver = std::make_unique<IMUDriver>(id);
        } else if (type == "gps" || type == "GPS") {
            driver = std::make_unique<GPSDriver>(id);
        } else if (type == "temperature" || type == "Temperature") {
            driver = std::make_unique<TemperatureDriver>(id);
        } else if (type == "pressure" || type == "Pressure") {
            driver = std::make_unique<PressureDriver>(id);
        } else if (type == "humidity" || type == "Humidity") {
            driver = std::make_unique<HumidityDriver>(id);
        } else {
            LOG_WARN("Unknown sensor type: " + type);
            continue;
        }
        
        // Initialize the driver
        if (driver->initialize(sensorConfig)) {
            registerSensor(std::move(driver));
            LOG_INFO("Loaded sensor: " + id + " (" + type + ")");
        } else {
            LOG_ERROR("Failed to initialize sensor: " + id);
        }
    }
    
    return true;
}

Json::Value SensorManager::saveDriversToConfig() const {
    Json::Value config(Json::arrayValue);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [id, driver] : m_sensors) {
        Json::Value sensorConfig;
        sensorConfig["id"] = id;
        sensorConfig["type"] = driver->getName();
        sensorConfig["enabled"] = driver->isConnected();
        config.append(sensorConfig);
    }
    
    return config;
}

// ============================================================================
// Start / Stop
// ============================================================================

bool SensorManager::start() {
    if (m_running) {
        LOG_WARN("SensorManager already running");
        return true;
    }
    
    if (!m_initialized) {
        LOG_ERROR("SensorManager not initialized");
        return false;
    }
    
    // Start all sensors
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, driver] : m_sensors) {
        if (driver->isConnected()) {
            if (!driver->start()) {
                LOG_ERROR("Failed to start sensor: " + id);
            }
        }
    }
    
    m_running = true;
    
    // Start collection thread
    m_collectionThread = std::thread(&SensorManager::dataCollectionLoop, this);
    m_healthThread = std::thread(&SensorManager::checkSensorHealth, this);
    
    emitEvent(SensorEvent{
        SensorEventType::MANAGER_STARTED,
        "",
        std::chrono::system_clock::now(),
        Json::nullValue,
        "Sensor Manager started",
        5,
        "manager"
    });
    
    LOG_INFO("SensorManager started");
    return true;
}

bool SensorManager::stop() {
    if (!m_running) {
        return true;
    }
    
    m_running = false;
    
    // Wait for threads
    if (m_collectionThread.joinable()) {
        m_collectionThread.join();
    }
    if (m_healthThread.joinable()) {
        m_healthThread.join();
    }
    
    // Stop all sensors
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, driver] : m_sensors) {
        if (driver->isRunning()) {
            driver->stop();
        }
    }
    
    emitEvent(SensorEvent{
        SensorEventType::MANAGER_STOPPED,
        "",
        std::chrono::system_clock::now(),
        Json::nullValue,
        "Sensor Manager stopped",
        5,
        "manager"
    });
    
    LOG_INFO("SensorManager stopped");
    return true;
}

// ============================================================================
// Sensor Registration
// ============================================================================

bool SensorManager::registerSensor(std::unique_ptr<SensorDriver> driver) {
    if (!driver) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string sensorId = driver->getSensorId();
    if (m_sensors.find(sensorId) != m_sensors.end()) {
        LOG_WARN("Sensor already registered: " + sensorId);
        return false;
    }
    
    // Set callbacks
    driver->setDataCallback([this](const SensorReading& reading) {
        processSensorData(reading);
    });
    
    driver->setErrorCallback([this](const std::string& error) {
        LOG_ERROR("Sensor error: " + error);
        SensorEvent event;
        event.type = SensorEventType::SENSOR_ERROR;
        event.message = error;
        event.severity = 9;
        event.category = "error";
        emitEvent(event);
    });
    
    driver->setStatusCallback([this](bool connected) {
        if (connected) {
            SensorEvent event;
            event.type = SensorEventType::SENSOR_CONNECTED;
            event.message = "Sensor connected";
            event.severity = 5;
            event.category = "status";
            emitEvent(event);
        } else {
            SensorEvent event;
            event.type = SensorEventType::SENSOR_DISCONNECTED;
            event.message = "Sensor disconnected";
            event.severity = 8;
            event.category = "status";
            emitEvent(event);
        }
    });
    
    m_sensors[sensorId] = std::move(driver);
    
    SensorEvent event;
    event.type = SensorEventType::SENSOR_ADDED;
    event.sensorId = sensorId;
    event.message = "Sensor registered: " + sensorId;
    event.severity = 5;
    event.category = "lifecycle";
    emitEvent(event);
    
    LOG_INFO("Sensor registered: " + sensorId);
    return true;
}

bool SensorManager::unregisterSensor(const std::string& sensorId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_sensors.find(sensorId);
    if (it == m_sensors.end()) {
        LOG_WARN("Sensor not found: " + sensorId);
        return false;
    }
    
    if (m_running && it->second->isRunning()) {
        it->second->stop();
    }
    
    SensorEvent event;
    event.type = SensorEventType::SENSOR_REMOVED;
    event.sensorId = sensorId;
    event.message = "Sensor removed: " + sensorId;
    event.severity = 6;
    event.category = "lifecycle";
    emitEvent(event);
    
    m_sensors.erase(it);
    LOG_INFO("Sensor unregistered: " + sensorId);
    return true;
}

// ============================================================================
// Sensor Access
// ============================================================================

SensorDriver* SensorManager::getSensor(const std::string& sensorId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sensors.find(sensorId);
    return it != m_sensors.end() ? it->second.get() : nullptr;
}

std::vector<std::string> SensorManager::getSensorIds() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> ids;
    ids.reserve(m_sensors.size());
    for (const auto& [id, _] : m_sensors) {
        ids.push_back(id);
    }
    return ids;
}

std::vector<SensorDriver*> SensorManager::getSensorsByType(SensorType type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<SensorDriver*> result;
    for (auto& [id, driver] : m_sensors) {
        if (driver->getType() == type) {
            result.push_back(driver.get());
        }
    }
    return result;
}

// ============================================================================
// Sensor Operations
// ============================================================================

bool SensorManager::readSensor(const std::string& sensorId, 
                              SensorReading& reading, int timeoutMs) {
    SensorDriver* driver = getSensor(sensorId);
    if (!driver) {
        LOG_ERROR("Sensor not found: " + sensorId);
        return false;
    }
    
    if (!driver->isConnected()) {
        LOG_ERROR("Sensor not connected: " + sensorId);
        return false;
    }
    
    bool result = driver->readSample(reading);
    if (result) {
        processSensorData(reading);
    } else {
        m_stats.errors++;
        SensorEvent event;
        event.type = SensorEventType::SENSOR_ERROR;
        event.sensorId = sensorId;
        event.message = "Failed to read sensor: " + sensorId;
        event.severity = 7;
        event.category = "error";
        emitEvent(event);
    }
    
    return result;
}

std::vector<SensorReading> SensorManager::readAllSensors() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<SensorReading> readings;
    readings.reserve(m_sensors.size());
    
    for (auto& [id, driver] : m_sensors) {
        if (driver->isRunning() && driver->isConnected()) {
            SensorReading reading;
            if (driver->readSample(reading)) {
                readings.push_back(reading);
                processSensorData(reading);
            } else {
                m_stats.errors++;
            }
        }
    }
    
    return readings;
}

bool SensorManager::calibrateSensor(const std::string& sensorId, 
                                   const Json::Value& params) {
    SensorDriver* driver = getSensor(sensorId);
    if (!driver) {
        LOG_ERROR("Sensor not found: " + sensorId);
        return false;
    }
    
    SensorEvent startEvent;
    startEvent.type = SensorEventType::SENSOR_CALIBRATION_STARTED;
    startEvent.sensorId = sensorId;
    startEvent.message = "Calibration started for: " + sensorId;
    startEvent.severity = 4;
    startEvent.category = "calibration";
    emitEvent(startEvent);
    
    bool result = driver->calibrate(params);
    
    SensorEvent completeEvent;
    completeEvent.type = result ? SensorEventType::SENSOR_CALIBRATION_COMPLETE 
                               : SensorEventType::SENSOR_CALIBRATION_FAILED;
    completeEvent.sensorId = sensorId;
    completeEvent.message = result ? "Calibration completed for: " + sensorId 
                                   : "Calibration failed for: " + sensorId;
    completeEvent.severity = result ? 3 : 8;
    completeEvent.category = "calibration";
    emitEvent(completeEvent);
    
    return result;
}

bool SensorManager::selfTestSensor(const std::string& sensorId, Json::Value& result) {
    SensorDriver* driver = getSensor(sensorId);
    if (!driver) {
        LOG_ERROR("Sensor not found: " + sensorId);
        return false;
    }
    
    SensorEvent startEvent;
    startEvent.type = SensorEventType::SENSOR_SELF_TEST_STARTED;
    startEvent.sensorId = sensorId;
    startEvent.message = "Self-test started for: " + sensorId;
    startEvent.severity = 4;
    startEvent.category = "test";
    emitEvent(startEvent);
    
    bool passed = driver->selfTest(result);
    
    SensorEvent completeEvent;
    completeEvent.type = passed ? SensorEventType::SENSOR_SELF_TEST_COMPLETE 
                               : SensorEventType::SENSOR_SELF_TEST_FAILED;
    completeEvent.sensorId = sensorId;
    completeEvent.message = passed ? "Self-test passed for: " + sensorId 
                                   : "Self-test failed for: " + sensorId;
    completeEvent.severity = passed ? 3 : 8;
    completeEvent.category = "test";
    emitEvent(completeEvent);
    
    return passed;
}

bool SensorManager::configureSensor(const std::string& sensorId, 
                                   const Json::Value& config) {
    SensorDriver* driver = getSensor(sensorId);
    if (!driver) {
        LOG_ERROR("Sensor not found: " + sensorId);
        return false;
    }
    
    bool result = driver->configure(config);
    
    if (result) {
        SensorEvent event;
        event.type = SensorEventType::SENSOR_CONFIG_CHANGED;
        event.sensorId = sensorId;
        event.message = "Configuration updated for: " + sensorId;
        event.severity = 3;
        event.category = "config";
        emitEvent(event);
    }
    
    return result;
}

// ============================================================================
// Callbacks
// ============================================================================

void SensorManager::setDataCallback(SensorCallback callback) {
    m_dataCallback = callback;
}

void SensorManager::setEventCallback(EventCallback callback) {
    m_eventCallback = callback;
}

// ============================================================================
// Discovery
// ============================================================================

int SensorManager::discoverSensors(int timeoutMs) {
    if (!m_autoDiscover) {
        LOG_INFO("Auto-discovery disabled");
        return 0;
    }
    
    LOG_INFO("Starting sensor discovery...");
    
    SensorEvent startEvent;
    startEvent.type = SensorEventType::SENSOR_DISCOVERY_STARTED;
    startEvent.message = "Sensor discovery started";
    startEvent.severity = 4;
    startEvent.category = "discovery";
    emitEvent(startEvent);
    
    int discovered = 0;
    
    // Check for common sensors on I2C bus
    std::vector<std::string> i2cBuses = {"/dev/i2c-0", "/dev/i2c-1", "/dev/i2c-2"};
    
    for (const auto& bus : i2cBuses) {
        // Check for I2C devices
        // In a real implementation, this would scan the I2C bus
        // For now, we'll check for known sensors
        
        // Check for IMU (MPU6050/MPU9250 at 0x68)
        // Check for Temperature (TMP102 at 0x48)
        // Check for Pressure (BMP180 at 0x77)
        // Check for Humidity (HTU21D at 0x40)
        
        // Simulate discovery
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        discovered += 2; // Simulate finding some sensors
    }
    
    // Check for GPS on serial ports
    std::vector<std::string> serialPorts = {"/dev/ttyS0", "/dev/ttyUSB0", "/dev/ttyACM0"};
    for (const auto& port : serialPorts) {
        // Check if GPS is available
        // Simulate discovery
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        discovered++;
    }
    
    SensorEvent completeEvent;
    completeEvent.type = SensorEventType::SENSOR_DISCOVERY_COMPLETE;
    completeEvent.message = "Sensor discovery completed, found " + std::to_string(discovered) + " sensors";
    completeEvent.severity = 4;
    completeEvent.category = "discovery";
    completeEvent.data["count"] = discovered;
    emitEvent(completeEvent);
    
    LOG_INFO("Sensor discovery complete: " + std::to_string(discovered) + " sensors found");
    return discovered;
}

// ============================================================================
// Statistics
// ============================================================================

Json::Value SensorManager::getStats() const {
    Json::Value stats;
    stats["readings_total"] = m_stats.readingsTotal.load();
    stats["errors"] = m_stats.errors.load();
    stats["timeouts"] = m_stats.timeouts.load();
    stats["sensor_count"] = static_cast<int>(m_sensors.size());
    
    auto now = std::chrono::steady_clock::now();
    stats["uptime_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(
        now - m_stats.startTime).count();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [id, driver] : m_sensors) {
        Json::Value sensorStats;
        sensorStats["connected"] = driver->isConnected();
        sensorStats["running"] = driver->isRunning();
        sensorStats["type"] = static_cast<int>(driver->getType());
        stats["sensors"][id] = sensorStats;
    }
    
    return stats;
}

void SensorManager::resetStats() {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats.readingsTotal = 0;
    m_stats.errors = 0;
    m_stats.timeouts = 0;
    m_stats.startTime = std::chrono::steady_clock::now();
}

// ============================================================================
// Configuration
// ============================================================================

bool SensorManager::saveConfig(const std::string& path) const {
    Json::Value json;
    json["collection_interval_ms"] = m_collectionIntervalMs;
    json["health_check_interval_ms"] = m_healthCheckIntervalMs;
    json["auto_discover"] = m_autoDiscover;
    json["sensors"] = saveDriversToConfig();
    
    std::ofstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open config file for writing: " + path);
        return false;
    }
    
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(json, &file);
    file.close();
    
    SensorEvent event;
    event.type = SensorEventType::SENSOR_CONFIG_SAVED;
    event.message = "Configuration saved to: " + path;
    event.severity = 3;
    event.category = "config";
    emitEvent(event);
    
    LOG_INFO("Configuration saved to: " + path);
    return true;
}

bool SensorManager::loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_WARN("Failed to open config file: " + path);
        return false;
    }
    
    Json::Value json;
    Json::CharReaderBuilder builder;
    std::string errs;
    if (!Json::parseFromStream(builder, file, &json, &errs)) {
        LOG_ERROR("Failed to parse config file: " + errs);
        file.close();
        return false;
    }
    file.close();
    
    if (json.isMember("collection_interval_ms")) {
        m_collectionIntervalMs = json["collection_interval_ms"].asInt();
    }
    if (json.isMember("health_check_interval_ms")) {
        m_healthCheckIntervalMs = json["health_check_interval_ms"].asInt();
    }
    if (json.isMember("auto_discover")) {
        m_autoDiscover = json["auto_discover"].asBool();
    }
    
    if (json.isMember("sensors") && json["sensors"].isArray()) {
        loadDriversFromConfig(json["sensors"]);
    }
    
    SensorEvent event;
    event.type = SensorEventType::SENSOR_CONFIG_LOADED;
    event.message = "Configuration loaded from: " + path;
    event.severity = 3;
    event.category = "config";
    emitEvent(event);
    
    LOG_INFO("Configuration loaded from: " + path);
    return true;
}

// ============================================================================
// Internal Threads
// ============================================================================

void SensorManager::dataCollectionLoop() {
    LOG_INFO("Data collection thread started");
    
    while (m_running) {
        auto startTime = std::chrono::steady_clock::now();
        
        // Read all active sensors
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [id, driver] : m_sensors) {
            if (driver->isRunning() && driver->isConnected()) {
                SensorReading reading;
                if (driver->readSample(reading)) {
                    processSensorData(reading);
                }
            }
        }
        
        // Calculate sleep time
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);
        int sleepTime = m_collectionIntervalMs - elapsed.count();
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
    }
    
    LOG_INFO("Data collection thread stopped");
}

void SensorManager::checkSensorHealth() {
    LOG_INFO("Health check thread started");
    
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_healthCheckIntervalMs));
        
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [id, driver] : m_sensors) {
            // Check if sensor is still responsive
            if (driver->isRunning() && !driver->isConnected()) {
                LOG_WARN("Sensor disconnected: " + id);
                
                SensorEvent event;
                event.type = SensorEventType::SENSOR_DISCONNECTED;
                event.sensorId = id;
                event.message = "Sensor disconnected: " + id;
                event.severity = 8;
                event.category = "status";
                emitEvent(event);
                
                // Attempt to reconnect
                if (driver->initialize(m_config)) {
                    driver->start();
                    LOG_INFO("Sensor reconnected: " + id);
                    
                    SensorEvent reconnectEvent;
                    reconnectEvent.type = SensorEventType::SENSOR_CONNECTED;
                    reconnectEvent.sensorId = id;
                    reconnectEvent.message = "Sensor reconnected: " + id;
                    reconnectEvent.severity = 5;
                    reconnectEvent.category = "status";
                    emitEvent(reconnectEvent);
                }
            }
        }
    }
    
    LOG_INFO("Health check thread stopped");
}

// ============================================================================
// Data Processing
// ============================================================================

void SensorManager::processSensorData(const SensorReading& reading) {
    // Update statistics
    m_stats.readingsTotal++;
    m_stats.readingsByType[reading.type]++;
    
    // Store last value
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [key, value] : reading.values) {
        m_lastValues[reading.sensorId][key] = value;
    }
    
    // Check thresholds
    // In a real implementation, this would check configured thresholds
    
    // Emit data callback
    if (m_dataCallback) {
        try {
            m_dataCallback(reading);
        } catch (const std::exception& e) {
            LOG_ERROR("Data callback error: " + std::string(e.what()));
        }
    }
    
    // Emit data available event
    SensorEvent event;
    event.type = SensorEventType::SENSOR_DATA_AVAILABLE;
    event.sensorId = reading.sensorId;
    event.message = "Data available from: " + reading.sensorId;
    event.severity = 3;
    event.category = "data";
    event.data = reading.toJson();
    emitEvent(event);
}

void SensorManager::emitEvent(const SensorEvent& event) {
    if (m_eventCallback) {
        try {
            m_eventCallback(event);
        } catch (const std::exception& e) {
            LOG_ERROR("Event callback error: " + std::string(e.what()));
        }
    }
    
    // Log events based on severity
    if (event.severity >= 8) {
        LOG_ERROR("Event: " + SensorEvent::eventTypeToString(event.type) + 
                  " - " + event.message);
    } else if (event.severity >= 6) {
        LOG_WARN("Event: " + SensorEvent::eventTypeToString(event.type) + 
                 " - " + event.message);
    } else {
        LOG_INFO("Event: " + SensorEvent::eventTypeToString(event.type) + 
                 " - " + event.message);
    }
}

void SensorManager::updateStats(const SensorReading& reading) {
    // Already handled in processSensorData
}

// ============================================================================
// Utility Functions
// ============================================================================

std::string sensorTypeToString(SensorType type) {
    switch (type) {
        case SensorType::IMU_ACCELEROMETER: return "IMU Accelerometer";
        case SensorType::IMU_GYROSCOPE: return "IMU Gyroscope";
        case SensorType::IMU_MAGNETOMETER: return "IMU Magnetometer";
        case SensorType::GPS: return "GPS";
        case SensorType::TEMPERATURE: return "Temperature";
        case SensorType::PRESSURE: return "Pressure";
        case SensorType::HUMIDITY: return "Humidity";
        case SensorType::LIGHT: return "Light";
        case SensorType::PROXIMITY: return "Proximity";
        case SensorType::ULTRASONIC: return "Ultrasonic";
        case SensorType::CO2: return "CO2";
        case SensorType::VOC: return "VOC";
        case SensorType::PARTICLE: return "Particle";
        default: return "Unknown";
    }
}

} // namespace sensor_service
