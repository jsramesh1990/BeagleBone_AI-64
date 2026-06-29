#include "sensor-service/SensorService.h"
#include "logger/Logger.h"

using namespace common;

namespace sensor_service {

SensorService& SensorService::getInstance() {
    static SensorService instance;
    return instance;
}

SensorService::SensorService() 
    : m_running(false), m_initialized(false) {}

SensorService::~SensorService() {
    stop();
}

bool SensorService::initialize(const Json::Value& config) {
    if (m_initialized) {
        return true;
    }
    
    m_config = config;
    m_initialized = true;
    
    LOG_INFO("SensorService initialized");
    return true;
}

bool SensorService::start() {
    if (m_running) {
        return true;
    }
    
    if (!m_initialized) {
        LOG_ERROR("SensorService not initialized");
        return false;
    }
    
    m_running = true;
    LOG_INFO("SensorService started");
    return true;
}

bool SensorService::stop() {
    if (!m_running) {
        return true;
    }
    
    m_running = false;
    LOG_INFO("SensorService stopped");
    return true;
}

bool SensorService::registerSensor(std::unique_ptr<SensorDriver> driver) {
    if (!driver) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string id = driver->getSensorId();
    
    if (m_sensors.find(id) != m_sensors.end()) {
        LOG_WARN("Sensor already registered: " + id);
        return false;
    }
    
    // Set callbacks
    driver->setDataCallback([this](const SensorReading& reading) {
        if (m_dataCallback) {
            m_dataCallback(reading);
        }
    });
    
    driver->setErrorCallback([this](const std::string& error) {
        LOG_ERROR("Sensor error: " + error);
    });
    
    driver->setStatusCallback([this](bool connected) {
        // Handle connection status change
    });
    
    m_sensors[id] = std::move(driver);
    LOG_INFO("Sensor registered: " + id);
    return true;
}

bool SensorService::unregisterSensor(const std::string& sensorId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_sensors.find(sensorId);
    if (it == m_sensors.end()) {
        LOG_WARN("Sensor not found: " + sensorId);
        return false;
    }
    
    if (m_running && it->second->isRunning()) {
        it->second->stop();
    }
    
    m_sensors.erase(it);
    LOG_INFO("Sensor unregistered: " + sensorId);
    return true;
}

SensorDriver* SensorService::getSensor(const std::string& sensorId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sensors.find(sensorId);
    return it != m_sensors.end() ? it->second.get() : nullptr;
}

std::vector<std::string> SensorService::getSensorIds() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> ids;
    ids.reserve(m_sensors.size());
    for (const auto& [id, _] : m_sensors) {
        ids.push_back(id);
    }
    return ids;
}

std::vector<SensorDriver*> SensorService::getSensorsByType(SensorType type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<SensorDriver*> result;
    for (auto& [id, driver] : m_sensors) {
        if (driver->getType() == type) {
            result.push_back(driver.get());
        }
    }
    return result;
}

bool SensorService::readSensor(const std::string& sensorId, 
                              SensorReading& reading, int timeoutMs) {
    SensorDriver* driver = getSensor(sensorId);
    if (!driver) {
        LOG_ERROR("Sensor not found: " + sensorId);
        return false;
    }
    
    return driver->readSample(reading);
}

std::vector<SensorReading> SensorService::readAllSensors() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<SensorReading> readings;
    readings.reserve(m_sensors.size());
    
    for (auto& [id, driver] : m_sensors) {
        if (driver->isRunning() && driver->isConnected()) {
            SensorReading reading;
            if (driver->readSample(reading)) {
                readings.push_back(reading);
            }
        }
    }
    
    return readings;
}

void SensorService::setDataCallback(SensorCallback callback) {
    m_dataCallback = callback;
}

void SensorService::setEventCallback(EventCallback callback) {
    m_eventCallback = callback;
}

Json::Value SensorService::getStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    Json::Value stats;
    stats["running"] = m_running;
    stats["sensor_count"] = static_cast<int>(m_sensors.size());
    
    for (const auto& [id, driver] : m_sensors) {
        Json::Value sensorStats;
        sensorStats["connected"] = driver->isConnected();
        sensorStats["running"] = driver->isRunning();
        sensorStats["type"] = static_cast<int>(driver->getType());
        stats["sensors"][id] = sensorStats;
    }
    
    return stats;
}

bool SensorService::saveConfig(const std::string& path) const {
    Json::Value json = m_config;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    Json::Value sensors(Json::arrayValue);
    for (const auto& [id, driver] : m_sensors) {
        Json::Value sensorJson;
        sensorJson["id"] = id;
        sensorJson["type"] = static_cast<int>(driver->getType());
        sensorJson["name"] = driver->getName();
        sensorJson["connected"] = driver->isConnected();
        sensors.append(sensorJson);
    }
    json["sensors"] = sensors;
    
    return true;
}

} // namespace sensor_service
