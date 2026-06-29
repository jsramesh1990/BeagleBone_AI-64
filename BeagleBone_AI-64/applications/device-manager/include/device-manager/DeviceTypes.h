#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <json/json.h>
#include <chrono>

namespace device_manager {

/**
 * @brief Device type enumeration
 */
enum class DeviceType {
    SENSOR_IMU,
    SENSOR_GPS,
    SENSOR_TEMPERATURE,
    SENSOR_PRESSURE,
    SENSOR_HUMIDITY,
    SENSOR_MAGNETOMETER,
    SENSOR_GYROSCOPE,
    SENSOR_ACCELEROMETER,
    
    ACTUATOR_LED,
    ACTUATOR_MOTOR,
    ACTUATOR_RELAY,
    ACTUATOR_BUZZER,
    ACTUATOR_SERVO,
    
    COMMUNICATION_WIFI,
    COMMUNICATION_BLUETOOTH,
    COMMUNICATION_ETHERNET,
    COMMUNICATION_SERIAL,
    COMMUNICATION_CAN,
    COMMUNICATION_I2C,
    COMMUNICATION_SPI,
    
    STORAGE_SD_CARD,
    STORAGE_USB,
    STORAGE_EMMC,
    
    DISPLAY_LCD,
    DISPLAY_OLED,
    DISPLAY_HDMI,
    
    INPUT_BUTTON,
    INPUT_KEYPAD,
    INPUT_TOUCH,
    INPUT_MICROPHONE,
    
    OUTPUT_AUDIO,
    OUTPUT_SPEAKER,
    
    UNKNOWN
};

/**
 * @brief Device status
 */
enum class DeviceStatus {
    OK,
    ERROR,
    WARNING,
    DISCONNECTED,
    INITIALIZING,
    STOPPED,
    UNKNOWN,
    NOT_FOUND,
    PERMISSION_DENIED,
    TIMEOUT
};

/**
 * @brief Device connection type
 */
enum class ConnectionType {
    I2C,
    SPI,
    UART,
    GPIO,
    USB,
    ETHERNET,
    WIFI,
    BLUETOOTH,
    CAN,
    PARALLEL,
    PCIE,
    MIPI,
    SDIO,
    PIPE,
    MEMORY_MAPPED
};

/**
 * @brief Device capability flags
 */
struct DeviceCapabilities {
    bool canReadData;
    bool canWriteData;
    bool canControl;
    bool canConfigure;
    bool canCalibrate;
    bool canSelfTest;
    bool canPowerOff;
    bool canReset;
    bool canInterrupt;
    bool canDma;
    bool canFifo;
    bool canStream;
    
    DeviceCapabilities() 
        : canReadData(false), canWriteData(false), canControl(false),
          canConfigure(false), canCalibrate(false), canSelfTest(false),
          canPowerOff(false), canReset(false), canInterrupt(false),
          canDma(false), canFifo(false), canStream(false) {}
};

/**
 * @brief Device address information
 */
struct DeviceAddress {
    ConnectionType type;
    std::string bus;
    uint32_t address;
    uint32_t subAddress;
    std::string path;
    std::string name;
    std::map<std::string, std::string> params;
    
    DeviceAddress() : type(ConnectionType::I2C), address(0), subAddress(0) {}
    
    Json::Value toJson() const {
        Json::Value json;
        json["type"] = static_cast<int>(type);
        json["bus"] = bus;
        json["address"] = address;
        json["subAddress"] = subAddress;
        json["path"] = path;
        json["name"] = name;
        
        for (const auto& [key, value] : params) {
            json["params"][key] = value;
        }
        
        return json;
    }
    
    static DeviceAddress fromJson(const Json::Value& json) {
        DeviceAddress addr;
        if (json.isMember("type")) {
            addr.type = static_cast<ConnectionType>(json["type"].asInt());
        }
        if (json.isMember("bus")) {
            addr.bus = json["bus"].asString();
        }
        if (json.isMember("address")) {
            addr.address = json["address"].asUInt();
        }
        if (json.isMember("subAddress")) {
            addr.subAddress = json["subAddress"].asUInt();
        }
        if (json.isMember("path")) {
            addr.path = json["path"].asString();
        }
        if (json.isMember("name")) {
            addr.name = json["name"].asString();
        }
        if (json.isMember("params")) {
            for (const auto& key : json["params"].getMemberNames()) {
                addr.params[key] = json["params"][key].asString();
            }
        }
        return addr;
    }
};

/**
 * @brief Device information
 */
struct DeviceInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string manufacturer;
    std::string model;
    std::string version;
    std::string serialNumber;
    DeviceType type;
    DeviceStatus status;
    DeviceCapabilities capabilities;
    DeviceAddress address;
    std::map<std::string, Json::Value> properties;
    std::map<std::string, Json::Value> metadata;
    std::chrono::system_clock::time_point discoveredAt;
    std::chrono::system_clock::time_point lastSeenAt;
    int priority;
    bool enabled;
    bool authenticated;
    
    DeviceInfo() 
        : type(DeviceType::UNKNOWN), status(DeviceStatus::UNKNOWN),
          priority(0), enabled(true), authenticated(false) {}
    
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["name"] = name;
        json["description"] = description;
        json["manufacturer"] = manufacturer;
        json["model"] = model;
        json["version"] = version;
        json["serialNumber"] = serialNumber;
        json["type"] = static_cast<int>(type);
        json["status"] = static_cast<int>(status);
        
        json["capabilities"]["canReadData"] = capabilities.canReadData;
        json["capabilities"]["canWriteData"] = capabilities.canWriteData;
        json["capabilities"]["canControl"] = capabilities.canControl;
        json["capabilities"]["canConfigure"] = capabilities.canConfigure;
        json["capabilities"]["canCalibrate"] = capabilities.canCalibrate;
        json["capabilities"]["canSelfTest"] = capabilities.canSelfTest;
        json["capabilities"]["canPowerOff"] = capabilities.canPowerOff;
        json["capabilities"]["canReset"] = capabilities.canReset;
        json["capabilities"]["canInterrupt"] = capabilities.canInterrupt;
        json["capabilities"]["canDma"] = capabilities.canDma;
        json["capabilities"]["canFifo"] = capabilities.canFifo;
        json["capabilities"]["canStream"] = capabilities.canStream;
        
        json["address"] = address.toJson();
        json["priority"] = priority;
        json["enabled"] = enabled;
        json["authenticated"] = authenticated;
        
        for (const auto& [key, value] : properties) {
            json["properties"][key] = value;
        }
        
        for (const auto& [key, value] : metadata) {
            json["metadata"][key] = value;
        }
        
        json["discoveredAt"] = std::chrono::duration_cast<std::chrono::seconds>(
            discoveredAt.time_since_epoch()).count();
        json["lastSeenAt"] = std::chrono::duration_cast<std::chrono::seconds>(
            lastSeenAt.time_since_epoch()).count();
        
        return json;
    }
    
    static DeviceInfo fromJson(const Json::Value& json) {
        DeviceInfo info;
        if (json.isMember("id")) info.id = json["id"].asString();
        if (json.isMember("name")) info.name = json["name"].asString();
        if (json.isMember("description")) info.description = json["description"].asString();
        if (json.isMember("manufacturer")) info.manufacturer = json["manufacturer"].asString();
        if (json.isMember("model")) info.model = json["model"].asString();
        if (json.isMember("version")) info.version = json["version"].asString();
        if (json.isMember("serialNumber")) info.serialNumber = json["serialNumber"].asString();
        if (json.isMember("type")) info.type = static_cast<DeviceType>(json["type"].asInt());
        if (json.isMember("status")) info.status = static_cast<DeviceStatus>(json["status"].asInt());
        if (json.isMember("priority")) info.priority = json["priority"].asInt();
        if (json.isMember("enabled")) info.enabled = json["enabled"].asBool();
        if (json.isMember("authenticated")) info.authenticated = json["authenticated"].asBool();
        
        if (json.isMember("capabilities")) {
            auto& caps = json["capabilities"];
            if (caps.isMember("canReadData")) info.capabilities.canReadData = caps["canReadData"].asBool();
            if (caps.isMember("canWriteData")) info.capabilities.canWriteData = caps["canWriteData"].asBool();
            if (caps.isMember("canControl")) info.capabilities.canControl = caps["canControl"].asBool();
            if (caps.isMember("canConfigure")) info.capabilities.canConfigure = caps["canConfigure"].asBool();
            if (caps.isMember("canCalibrate")) info.capabilities.canCalibrate = caps["canCalibrate"].asBool();
            if (caps.isMember("canSelfTest")) info.capabilities.canSelfTest = caps["canSelfTest"].asBool();
            if (caps.isMember("canPowerOff")) info.capabilities.canPowerOff = caps["canPowerOff"].asBool();
            if (caps.isMember("canReset")) info.capabilities.canReset = caps["canReset"].asBool();
            if (caps.isMember("canInterrupt")) info.capabilities.canInterrupt = caps["canInterrupt"].asBool();
            if (caps.isMember("canDma")) info.capabilities.canDma = caps["canDma"].asBool();
            if (caps.isMember("canFifo")) info.capabilities.canFifo = caps["canFifo"].asBool();
            if (caps.isMember("canStream")) info.capabilities.canStream = caps["canStream"].asBool();
        }
        
        if (json.isMember("address")) {
            info.address = DeviceAddress::fromJson(json["address"]);
        }
        
        if (json.isMember("properties")) {
            for (const auto& key : json["properties"].getMemberNames()) {
                info.properties[key] = json["properties"][key];
            }
        }
        
        if (json.isMember("metadata")) {
            for (const auto& key : json["metadata"].getMemberNames()) {
                info.metadata[key] = json["metadata"][key];
            }
        }
        
        if (json.isMember("discoveredAt")) {
            int64_t seconds = json["discoveredAt"].asInt64();
            info.discoveredAt = std::chrono::system_clock::time_point(
                std::chrono::seconds(seconds));
        }
        
        if (json.isMember("lastSeenAt")) {
            int64_t seconds = json["lastSeenAt"].asInt64();
            info.lastSeenAt = std::chrono::system_clock::time_point(
                std::chrono::seconds(seconds));
        }
        
        return info;
    }
};

/**
 * @brief Device data sample
 */
struct DeviceSample {
    std::string deviceId;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, Json::Value> data;
    std::map<std::string, double> values;
    uint32_t sequence;
    uint32_t flags;
    bool isValid;
    
    DeviceSample() : sequence(0), flags(0), isValid(true) {}
    
    Json::Value toJson() const {
        Json::Value json;
        json["deviceId"] = deviceId;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()).count();
        json["sequence"] = sequence;
        json["flags"] = flags;
        json["isValid"] = isValid;
        
        for (const auto& [key, value] : data) {
            json["data"][key] = value;
        }
        
        for (const auto& [key, value] : values) {
            json["values"][key] = value;
        }
        
        return json;
    }
    
    static DeviceSample fromJson(const Json::Value& json) {
        DeviceSample sample;
        if (json.isMember("deviceId")) sample.deviceId = json["deviceId"].asString();
        if (json.isMember("timestamp")) {
            int64_t ms = json["timestamp"].asInt64();
            sample.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(ms));
        }
        if (json.isMember("sequence")) sample.sequence = json["sequence"].asUInt();
        if (json.isMember("flags")) sample.flags = json["flags"].asUInt();
        if (json.isMember("isValid")) sample.isValid = json["isValid"].asBool();
        
        if (json.isMember("data")) {
            for (const auto& key : json["data"].getMemberNames()) {
                sample.data[key] = json["data"][key];
            }
        }
        
        if (json.isMember("values")) {
            for (const auto& key : json["values"].getMemberNames()) {
                sample.values[key] = json["values"][key].asDouble();
            }
        }
        
        return sample;
    }
};

/**
 * @brief Device command
 */
struct DeviceCommand {
    std::string id;
    std::string deviceId;
    std::string name;
    std::map<std::string, Json::Value> parameters;
    std::chrono::system_clock::time_point issuedAt;
    std::chrono::system_clock::time_point timeoutAt;
    bool isAsync;
    int priority;
    
    DeviceCommand() : isAsync(false), priority(0) {}
    
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["deviceId"] = deviceId;
        json["name"] = name;
        json["isAsync"] = isAsync;
        json["priority"] = priority;
        
        for (const auto& [key, value] : parameters) {
            json["parameters"][key] = value;
        }
        
        json["issuedAt"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            issuedAt.time_since_epoch()).count();
        json["timeoutAt"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            timeoutAt.time_since_epoch()).count();
        
        return json;
    }
};

/**
 * @brief Device event
 */
struct DeviceEvent {
    enum Type {
        DEVICE_ADDED,
        DEVICE_REMOVED,
        DEVICE_CHANGED,
        DEVICE_STATUS_CHANGED,
        DEVICE_DATA_AVAILABLE,
        DEVICE_COMMAND_COMPLETED,
        DEVICE_COMMAND_FAILED,
        DEVICE_CALIBRATION_COMPLETE,
        DEVICE_SELF_TEST_COMPLETE,
        DEVICE_ERROR,
        DEVICE_WARNING,
        DEVICE_INFO,
        DEVICE_STREAM_STARTED,
        DEVICE_STREAM_STOPPED
    };
    
    Type type;
    std::string deviceId;
    std::chrono::system_clock::time_point timestamp;
    Json::Value data;
    std::string message;
    
    DeviceEvent() : type(DEVICE_INFO) {}
    
    Json::Value toJson() const {
        Json::Value json;
        json["type"] = static_cast<int>(type);
        json["deviceId"] = deviceId;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()).count();
        json["data"] = data;
        json["message"] = message;
        return json;
    }
};

} // namespace device_manager

#endif // DEVICE_TYPES_H
