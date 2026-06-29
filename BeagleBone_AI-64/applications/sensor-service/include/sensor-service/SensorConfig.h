#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <string>
#include <map>
#include <json/json.h>
#include "SensorData.h"

namespace sensor_service {

/**
 * @brief Sensor configuration structure
 */
struct SensorConfig {
    std::string id;
    std::string name;
    SensorType type;
    bool enabled;
    int samplingRate;      // Hz
    int sampleIntervalMs;  // Milliseconds between samples
    int timeoutMs;         // Read timeout
    int retryCount;        // Number of retries on failure
    std::map<std::string, std::string> parameters;
    std::map<std::string, double> thresholds;
    Json::Value customConfig;
    
    SensorConfig() 
        : type(SensorType::UNKNOWN), 
          enabled(true), 
          samplingRate(1),
          sampleIntervalMs(1000),
          timeoutMs(5000),
          retryCount(3) {}
    
    /**
     * @brief Serialize to JSON
     */
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["name"] = name;
        json["type"] = static_cast<int>(type);
        json["enabled"] = enabled;
        json["samplingRate"] = samplingRate;
        json["sampleIntervalMs"] = sampleIntervalMs;
        json["timeoutMs"] = timeoutMs;
        json["retryCount"] = retryCount;
        
        for (const auto& [key, value] : parameters) {
            json["parameters"][key] = value;
        }
        
        for (const auto& [key, value] : thresholds) {
            json["thresholds"][key] = value;
        }
        
        json["custom"] = customConfig;
        
        return json;
    }
    
    /**
     * @brief Deserialize from JSON
     */
    static SensorConfig fromJson(const Json::Value& json) {
        SensorConfig config;
        
        if (json.isMember("id")) config.id = json["id"].asString();
        if (json.isMember("name")) config.name = json["name"].asString();
        if (json.isMember("type")) config.type = static_cast<SensorType>(json["type"].asInt());
        if (json.isMember("enabled")) config.enabled = json["enabled"].asBool();
        if (json.isMember("samplingRate")) config.samplingRate = json["samplingRate"].asInt();
        if (json.isMember("sampleIntervalMs")) config.sampleIntervalMs = json["sampleIntervalMs"].asInt();
        if (json.isMember("timeoutMs")) config.timeoutMs = json["timeoutMs"].asInt();
        if (json.isMember("retryCount")) config.retryCount = json["retryCount"].asInt();
        
        if (json.isMember("parameters")) {
            for (const auto& key : json["parameters"].getMemberNames()) {
                config.parameters[key] = json["parameters"][key].asString();
            }
        }
        
        if (json.isMember("thresholds")) {
            for (const auto& key : json["thresholds"].getMemberNames()) {
                config.thresholds[key] = json["thresholds"][key].asDouble();
            }
        }
        
        if (json.isMember("custom")) {
            config.customConfig = json["custom"];
        }
        
        return config;
    }
    
    /**
     * @brief Validate configuration
     */
    bool validate() const {
        if (id.empty() || name.empty()) return false;
        if (samplingRate <= 0 || samplingRate > 1000) return false;
        if (sampleIntervalMs <= 0 || sampleIntervalMs > 60000) return false;
        if (timeoutMs <= 0 || timeoutMs > 30000) return false;
        if (retryCount < 0 || retryCount > 10) return false;
        return true;
    }
};

} // namespace sensor_service

#endif // SENSOR_CONFIG_H
