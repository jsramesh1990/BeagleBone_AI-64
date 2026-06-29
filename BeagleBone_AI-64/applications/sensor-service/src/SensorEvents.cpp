#include "sensor-service/SensorEvents.h"
#include <map>

namespace sensor_service {

const std::map<SensorEventType, std::string> EVENT_TYPE_NAMES = {
    // Sensor lifecycle
    {SensorEventType::SENSOR_ADDED, "SENSOR_ADDED"},
    {SensorEventType::SENSOR_REMOVED, "SENSOR_REMOVED"},
    {SensorEventType::SENSOR_STARTED, "SENSOR_STARTED"},
    {SensorEventType::SENSOR_STOPPED, "SENSOR_STOPPED"},
    
    // Sensor status
    {SensorEventType::SENSOR_CONNECTED, "SENSOR_CONNECTED"},
    {SensorEventType::SENSOR_DISCONNECTED, "SENSOR_DISCONNECTED"},
    {SensorEventType::SENSOR_ERROR, "SENSOR_ERROR"},
    {SensorEventType::SENSOR_WARNING, "SENSOR_WARNING"},
    {SensorEventType::SENSOR_INFO, "SENSOR_INFO"},
    
    // Sensor data
    {SensorEventType::SENSOR_DATA_AVAILABLE, "SENSOR_DATA_AVAILABLE"},
    {SensorEventType::SENSOR_DATA_TIMEOUT, "SENSOR_DATA_TIMEOUT"},
    {SensorEventType::SENSOR_DATA_THRESHOLD_EXCEEDED, "SENSOR_DATA_THRESHOLD_EXCEEDED"},
    
    // Sensor calibration
    {SensorEventType::SENSOR_CALIBRATION_STARTED, "SENSOR_CALIBRATION_STARTED"},
    {SensorEventType::SENSOR_CALIBRATION_COMPLETE, "SENSOR_CALIBRATION_COMPLETE"},
    {SensorEventType::SENSOR_CALIBRATION_FAILED, "SENSOR_CALIBRATION_FAILED"},
    
    // Sensor test
    {SensorEventType::SENSOR_SELF_TEST_STARTED, "SENSOR_SELF_TEST_STARTED"},
    {SensorEventType::SENSOR_SELF_TEST_COMPLETE, "SENSOR_SELF_TEST_COMPLETE"},
    {SensorEventType::SENSOR_SELF_TEST_FAILED, "SENSOR_SELF_TEST_FAILED"},
    
    // Sensor configuration
    {SensorEventType::SENSOR_CONFIG_CHANGED, "SENSOR_CONFIG_CHANGED"},
    {SensorEventType::SENSOR_CONFIG_LOADED, "SENSOR_CONFIG_LOADED"},
    {SensorEventType::SENSOR_CONFIG_SAVED, "SENSOR_CONFIG_SAVED"},
    {SensorEventType::SENSOR_CONFIG_ERROR, "SENSOR_CONFIG_ERROR"},
    
    // Discovery
    {SensorEventType::SENSOR_DISCOVERY_STARTED, "SENSOR_DISCOVERY_STARTED"},
    {SensorEventType::SENSOR_DISCOVERY_COMPLETE, "SENSOR_DISCOVERY_COMPLETE"},
    {SensorEventType::SENSOR_DISCOVERY_ERROR, "SENSOR_DISCOVERY_ERROR"},
    {SensorEventType::SENSOR_DISCOVERED, "SENSOR_DISCOVERED"},
    
    // Manager events
    {SensorEventType::MANAGER_STARTED, "MANAGER_STARTED"},
    {SensorEventType::MANAGER_STOPPED, "MANAGER_STOPPED"},
    {SensorEventType::MANAGER_INITIALIZED, "MANAGER_INITIALIZED"},
    {SensorEventType::MANAGER_ERROR, "MANAGER_ERROR"},
    {SensorEventType::MANAGER_WARNING, "MANAGER_WARNING"},
    {SensorEventType::MANAGER_INFO, "MANAGER_INFO"}
};

const std::map<SensorEventType, int> EVENT_SEVERITIES = {
    {SensorEventType::SENSOR_ERROR, 10},
    {SensorEventType::MANAGER_ERROR, 10},
    {SensorEventType::SENSOR_DISCONNECTED, 9},
    {SensorEventType::SENSOR_DATA_TIMEOUT, 8},
    {SensorEventType::SENSOR_CALIBRATION_FAILED, 8},
    {SensorEventType::SENSOR_SELF_TEST_FAILED, 8},
    {SensorEventType::SENSOR_WARNING, 7},
    {SensorEventType::MANAGER_WARNING, 7},
    {SensorEventType::SENSOR_DATA_THRESHOLD_EXCEEDED, 7},
    {SensorEventType::SENSOR_CONFIG_ERROR, 7},
    {SensorEventType::SENSOR_DISCOVERY_ERROR, 7},
    {SensorEventType::SENSOR_STOPPED, 6},
    {SensorEventType::SENSOR_REMOVED, 6},
    {SensorEventType::SENSOR_STARTED, 5},
    {SensorEventType::SENSOR_ADDED, 5},
    {SensorEventType::SENSOR_CONNECTED, 5},
    {SensorEventType::MANAGER_STARTED, 5},
    {SensorEventType::MANAGER_INITIALIZED, 5},
    {SensorEventType::SENSOR_CALIBRATION_STARTED, 4},
    {SensorEventType::SENSOR_SELF_TEST_STARTED, 4},
    {SensorEventType::SENSOR_DISCOVERY_STARTED, 4},
    {SensorEventType::SENSOR_DISCOVERY_COMPLETE, 4},
    {SensorEventType::SENSOR_DATA_AVAILABLE, 3},
    {SensorEventType::SENSOR_CONFIG_CHANGED, 3},
    {SensorEventType::SENSOR_CONFIG_LOADED, 3},
    {SensorEventType::SENSOR_CONFIG_SAVED, 3},
    {SensorEventType::SENSOR_CALIBRATION_COMPLETE, 3},
    {SensorEventType::SENSOR_SELF_TEST_COMPLETE, 3},
    {SensorEventType::SENSOR_DISCOVERED, 3},
    {SensorEventType::SENSOR_INFO, 2},
    {SensorEventType::MANAGER_INFO, 2},
    {SensorEventType::MANAGER_STOPPED, 2}
};

const std::map<SensorEventType, std::string> EVENT_CATEGORIES = {
    {SensorEventType::SENSOR_ADDED, "lifecycle"},
    {SensorEventType::SENSOR_REMOVED, "lifecycle"},
    {SensorEventType::SENSOR_STARTED, "lifecycle"},
    {SensorEventType::SENSOR_STOPPED, "lifecycle"},
    {SensorEventType::SENSOR_CONNECTED, "status"},
    {SensorEventType::SENSOR_DISCONNECTED, "status"},
    {SensorEventType::SENSOR_ERROR, "status"},
    {SensorEventType::SENSOR_WARNING, "status"},
    {SensorEventType::SENSOR_INFO, "status"},
    {SensorEventType::SENSOR_DATA_AVAILABLE, "data"},
    {SensorEventType::SENSOR_DATA_TIMEOUT, "data"},
    {SensorEventType::SENSOR_DATA_THRESHOLD_EXCEEDED, "data"},
    {SensorEventType::SENSOR_CALIBRATION_STARTED, "calibration"},
    {SensorEventType::SENSOR_CALIBRATION_COMPLETE, "calibration"},
    {SensorEventType::SENSOR_CALIBRATION_FAILED, "calibration"},
    {SensorEventType::SENSOR_SELF_TEST_STARTED, "test"},
    {SensorEventType::SENSOR_SELF_TEST_COMPLETE, "test"},
    {SensorEventType::SENSOR_SELF_TEST_FAILED, "test"},
    {SensorEventType::SENSOR_CONFIG_CHANGED, "config"},
    {SensorEventType::SENSOR_CONFIG_LOADED, "config"},
    {SensorEventType::SENSOR_CONFIG_SAVED, "config"},
    {SensorEventType::SENSOR_CONFIG_ERROR, "config"},
    {SensorEventType::SENSOR_DISCOVERY_STARTED, "discovery"},
    {SensorEventType::SENSOR_DISCOVERY_COMPLETE, "discovery"},
    {SensorEventType::SENSOR_DISCOVERY_ERROR, "discovery"},
    {SensorEventType::SENSOR_DISCOVERED, "discovery"},
    {SensorEventType::MANAGER_STARTED, "manager"},
    {SensorEventType::MANAGER_STOPPED, "manager"},
    {SensorEventType::MANAGER_INITIALIZED, "manager"},
    {SensorEventType::MANAGER_ERROR, "manager"},
    {SensorEventType::MANAGER_WARNING, "manager"},
    {SensorEventType::MANAGER_INFO, "manager"}
};

std::string SensorEvent::eventTypeToString(SensorEventType type) {
    auto it = EVENT_TYPE_NAMES.find(type);
    return it != EVENT_TYPE_NAMES.end() ? it->second : "UNKNOWN";
}

SensorEventType SensorEvent::stringToEventType(const std::string& str) {
    for (const auto& [type, name] : EVENT_TYPE_NAMES) {
        if (name == str) {
            return type;
        }
    }
    return SensorEventType::MANAGER_INFO;
}

int SensorEvent::getEventSeverity(SensorEventType type) {
    auto it = EVENT_SEVERITIES.find(type);
    return it != EVENT_SEVERITIES.end() ? it->second : 5;
}

std::string SensorEvent::getEventCategory(SensorEventType type) {
    auto it = EVENT_CATEGORIES.find(type);
    return it != EVENT_CATEGORIES.end() ? it->second : "unknown";
}

} // namespace sensor_service
