#ifndef SENSOR_EVENTS_H
#define SENSOR_EVENTS_H

#include <string>
#include <chrono>
#include <json/json.h>
#include "SensorData.h"

namespace sensor_service {

/**
 * @brief Sensor event types
 */
enum class SensorEventType {
    // Sensor lifecycle
    SENSOR_ADDED,
    SENSOR_REMOVED,
    SENSOR_STARTED,
    SENSOR_STOPPED,
    
    // Sensor status
    SENSOR_CONNECTED,
    SENSOR_DISCONNECTED,
    SENSOR_ERROR,
    SENSOR_WARNING,
    SENSOR_INFO,
    
    // Sensor data
    SENSOR_DATA_AVAILABLE,
    SENSOR_DATA_TIMEOUT,
    SENSOR_DATA_THRESHOLD_EXCEEDED,
    
    // Sensor calibration
    SENSOR_CALIBRATION_STARTED,
    SENSOR_CALIBRATION_COMPLETE,
    SENSOR_CALIBRATION_FAILED,
    
    // Sensor test
    SENSOR_SELF_TEST_STARTED,
    SENSOR_SELF_TEST_COMPLETE,
    SENSOR_SELF_TEST_FAILED,
    
    // Sensor configuration
    SENSOR_CONFIG_CHANGED,
    SENSOR_CONFIG_LOADED,
    SENSOR_CONFIG_SAVED,
    SENSOR_CONFIG_ERROR,
    
    // Discovery
    SENSOR_DISCOVERY_STARTED,
    SENSOR_DISCOVERY_COMPLETE,
    SENSOR_DISCOVERY_ERROR,
    SENSOR_DISCOVERED,
    
    // Manager events
    MANAGER_STARTED,
    MANAGER_STOPPED,
    MANAGER_INITIALIZED,
    MANAGER_ERROR,
    MANAGER_WARNING,
    MANAGER_INFO
};

/**
 * @brief Sensor event structure
 */
struct SensorEvent {
    SensorEventType type;
    std::string sensorId;
    std::chrono::system_clock::time_point timestamp;
    Json::Value data;
    std::string message;
    int severity;  // 0-10, 10 = most severe
    std::string category;
    std::map<std::string, std::string> tags;
    
    SensorEvent() 
        : type(SensorEventType::MANAGER_INFO), 
          severity(5),
          timestamp(std::chrono::system_clock::now()) {}
    
    Json::Value toJson() const {
        Json::Value json;
        json["type"] = static_cast<int>(type);
        json["typeName"] = eventTypeToString(type);
        json["sensorId"] = sensorId;
        json["message"] = message;
        json["data"] = data;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()).count();
        json["severity"] = severity;
        json["category"] = category;
        
        for (const auto& [key, value] : tags) {
            json["tags"][key] = value;
        }
        
        return json;
    }
    
    static SensorEvent fromJson(const Json::Value& json) {
        SensorEvent event;
        if (json.isMember("type")) {
            event.type = static_cast<SensorEventType>(json["type"].asInt());
        }
        if (json.isMember("sensorId")) {
            event.sensorId = json["sensorId"].asString();
        }
        if (json.isMember("message")) {
            event.message = json["message"].asString();
        }
        if (json.isMember("data")) {
            event.data = json["data"];
        }
        if (json.isMember("timestamp")) {
            int64_t ms = json["timestamp"].asInt64();
            event.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(ms));
        }
        if (json.isMember("severity")) {
            event.severity = json["severity"].asInt();
        }
        if (json.isMember("category")) {
            event.category = json["category"].asString();
        }
        if (json.isMember("tags")) {
            for (const auto& key : json["tags"].getMemberNames()) {
                event.tags[key] = json["tags"][key].asString();
            }
        }
        return event;
    }
    
    /**
     * @brief Convert event type to string
     */
    static std::string eventTypeToString(SensorEventType type);
    
    /**
     * @brief Convert string to event type
     */
    static SensorEventType stringToEventType(const std::string& str);
    
    /**
     * @brief Get event severity
     */
    static int getEventSeverity(SensorEventType type);
    
    /**
     * @brief Get event category
     */
    static std::string getEventCategory(SensorEventType type);
};

} // namespace sensor_service

#endif // SENSOR_EVENTS_H
