#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <json/json.h>

namespace sensor_service {

/**
 * @brief Sensor types supported by the service
 */
enum class SensorType {
    UNKNOWN = 0,
    IMU_ACCELEROMETER = 1,
    IMU_GYROSCOPE = 2,
    IMU_MAGNETOMETER = 3,
    GPS = 4,
    TEMPERATURE = 5,
    PRESSURE = 6,
    HUMIDITY = 7,
    LIGHT = 8,
    PROXIMITY = 9,
    ULTRASONIC = 10,
    CO2 = 11,
    VOC = 12,
    PARTICLE = 13
};

/**
 * @brief Sensor reading with timestamp
 */
struct SensorReading {
    SensorType type;
    std::string sensorId;
    std::map<std::string, double> values;
    std::chrono::system_clock::time_point timestamp;
    uint64_t sequence;
    bool isValid;
    std::map<std::string, std::string> metadata;
    
    SensorReading() : type(SensorType::UNKNOWN), sequence(0), isValid(true) {}
    
    /**
     * @brief Get a specific value by name
     */
    double getValue(const std::string& name, double defaultValue = 0.0) const {
        auto it = values.find(name);
        return it != values.end() ? it->second : defaultValue;
    }
    
    /**
     * @brief Check if reading contains a specific value
     */
    bool hasValue(const std::string& name) const {
        return values.find(name) != values.end();
    }
    
    /**
     * @brief Serialize to JSON
     */
    Json::Value toJson() const {
        Json::Value json;
        json["type"] = static_cast<int>(type);
        json["sensorId"] = sensorId;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()).count();
        json["sequence"] = sequence;
        json["isValid"] = isValid;
        
        for (const auto& [key, value] : values) {
            json["values"][key] = value;
        }
        
        for (const auto& [key, value] : metadata) {
            json["metadata"][key] = value;
        }
        
        return json;
    }
    
    /**
     * @brief Deserialize from JSON
     */
    static SensorReading fromJson(const Json::Value& json) {
        SensorReading reading;
        if (json.isMember("type")) {
            reading.type = static_cast<SensorType>(json["type"].asInt());
        }
        if (json.isMember("sensorId")) {
            reading.sensorId = json["sensorId"].asString();
        }
        if (json.isMember("timestamp")) {
            int64_t ms = json["timestamp"].asInt64();
            reading.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(ms));
        }
        if (json.isMember("sequence")) {
            reading.sequence = json["sequence"].asUInt64();
        }
        if (json.isMember("isValid")) {
            reading.isValid = json["isValid"].asBool();
        }
        
        if (json.isMember("values")) {
            for (const auto& key : json["values"].getMemberNames()) {
                reading.values[key] = json["values"][key].asDouble();
            }
        }
        
        if (json.isMember("metadata")) {
            for (const auto& key : json["metadata"].getMemberNames()) {
                reading.metadata[key] = json["metadata"][key].asString();
            }
        }
        
        return reading;
    }
};

/**
 * @brief IMU data (accelerometer, gyroscope, magnetometer)
 */
struct IMUData {
    struct Accelerometer {
        double x;
        double y;
        double z;
        double magnitude;
    };
    
    struct Gyroscope {
        double x;
        double y;
        double z;
        double magnitude;
    };
    
    struct Magnetometer {
        double x;
        double y;
        double z;
        double magnitude;
    };
    
    Accelerometer accel;
    Gyroscope gyro;
    Magnetometer mag;
    double temperature;
    double timestamp;
    
    /**
     * @brief Convert to SensorReading
     */
    SensorReading toReading(const std::string& sensorId) const {
        SensorReading reading;
        reading.type = SensorType::IMU_ACCELEROMETER;
        reading.sensorId = sensorId;
        reading.timestamp = std::chrono::system_clock::now();
        
        // Accelerometer
        reading.values["accel_x"] = accel.x;
        reading.values["accel_y"] = accel.y;
        reading.values["accel_z"] = accel.z;
        reading.values["accel_magnitude"] = accel.magnitude;
        
        // Gyroscope
        reading.values["gyro_x"] = gyro.x;
        reading.values["gyro_y"] = gyro.y;
        reading.values["gyro_z"] = gyro.z;
        reading.values["gyro_magnitude"] = gyro.magnitude;
        
        // Magnetometer
        reading.values["mag_x"] = mag.x;
        reading.values["mag_y"] = mag.y;
        reading.values["mag_z"] = mag.z;
        reading.values["mag_magnitude"] = mag.magnitude;
        
        // Temperature
        reading.values["temperature"] = temperature;
        
        return reading;
    }
};

/**
 * @brief GPS data
 */
struct GPSData {
    double latitude;
    double longitude;
    double altitude;
    double speed;
    double heading;
    double horizontalAccuracy;
    double verticalAccuracy;
    int satellites;
    bool hasFix;
    std::string timestamp;
    
    /**
     * @brief Convert to SensorReading
     */
    SensorReading toReading(const std::string& sensorId) const {
        SensorReading reading;
        reading.type = SensorType::GPS;
        reading.sensorId = sensorId;
        reading.timestamp = std::chrono::system_clock::now();
        
        reading.values["latitude"] = latitude;
        reading.values["longitude"] = longitude;
        reading.values["altitude"] = altitude;
        reading.values["speed"] = speed;
        reading.values["heading"] = heading;
        reading.values["horizontal_accuracy"] = horizontalAccuracy;
        reading.values["vertical_accuracy"] = verticalAccuracy;
        reading.values["satellites"] = satellites;
        reading.values["has_fix"] = hasFix ? 1.0 : 0.0;
        
        reading.metadata["gps_timestamp"] = timestamp;
        
        return reading;
    }
};

/**
 * @brief Temperature data
 */
struct TemperatureData {
    double temperature;
    double humidity; // Optional
    double dewPoint;
    double heatIndex;
    
    /**
     * @brief Convert to SensorReading
     */
    SensorReading toReading(const std::string& sensorId) const {
        SensorReading reading;
        reading.type = SensorType::TEMPERATURE;
        reading.sensorId = sensorId;
        reading.timestamp = std::chrono::system_clock::now();
        
        reading.values["temperature"] = temperature;
        reading.values["humidity"] = humidity;
        reading.values["dew_point"] = dewPoint;
        reading.values["heat_index"] = heatIndex;
        
        return reading;
    }
};

/**
 * @brief Pressure data
 */
struct PressureData {
    double pressure;
    double altitude;
    double seaLevelPressure;
    double temperature;
    
    /**
     * @brief Convert to SensorReading
     */
    SensorReading toReading(const std::string& sensorId) const {
        SensorReading reading;
        reading.type = SensorType::PRESSURE;
        reading.sensorId = sensorId;
        reading.timestamp = std::chrono::system_clock::now();
        
        reading.values["pressure"] = pressure;
        reading.values["altitude"] = altitude;
        reading.values["sea_level_pressure"] = seaLevelPressure;
        reading.values["temperature"] = temperature;
        
        return reading;
    }
};

/**
 * @brief Humidity data
 */
struct HumidityData {
    double humidity;
    double temperature;
    double dewPoint;
    double absoluteHumidity;
    
    /**
     * @brief Convert to SensorReading
     */
    SensorReading toReading(const std::string& sensorId) const {
        SensorReading reading;
        reading.type = SensorType::HUMIDITY;
        reading.sensorId = sensorId;
        reading.timestamp = std::chrono::system_clock::now();
        
        reading.values["humidity"] = humidity;
        reading.values["temperature"] = temperature;
        reading.values["dew_point"] = dewPoint;
        reading.values["absolute_humidity"] = absoluteHumidity;
        
        return reading;
    }
};

} // namespace sensor_service

#endif // SENSOR_DATA_H
