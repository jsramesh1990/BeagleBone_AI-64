#ifndef DEVICE_MANAGER_EVENTS_H
#define DEVICE_MANAGER_EVENTS_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <json/json.h>
#include "DeviceTypes.h"

namespace device_manager {

/**
 * @brief Event types for the Device Manager
 */
class DeviceManagerEvents {
public:
    /**
     * @brief Event types
     */
    enum class EventType {
        // Device lifecycle events
        DEVICE_ADDED,
        DEVICE_REMOVED,
        DEVICE_UPDATED,
        DEVICE_ENABLED,
        DEVICE_DISABLED,
        DEVICE_STARTED,
        DEVICE_STOPPED,
        DEVICE_RESET,
        
        // Device status events
        DEVICE_STATUS_CHANGED,
        DEVICE_ERROR,
        DEVICE_WARNING,
        DEVICE_INFO,
        DEVICE_ONLINE,
        DEVICE_OFFLINE,
        
        // Device data events
        DEVICE_DATA_AVAILABLE,
        DEVICE_DATA_STREAM_STARTED,
        DEVICE_DATA_STREAM_STOPPED,
        DEVICE_DATA_THRESHOLD_EXCEEDED,
        DEVICE_DATA_TIMEOUT,
        
        // Device command events
        DEVICE_COMMAND_STARTED,
        DEVICE_COMMAND_COMPLETED,
        DEVICE_COMMAND_FAILED,
        DEVICE_COMMAND_TIMEOUT,
        
        // Device configuration events
        DEVICE_CONFIG_CHANGED,
        DEVICE_CONFIG_LOADED,
        DEVICE_CONFIG_SAVED,
        DEVICE_CONFIG_ERROR,
        
        // Device calibration events
        DEVICE_CALIBRATION_STARTED,
        DEVICE_CALIBRATION_COMPLETE,
        DEVICE_CALIBRATION_FAILED,
        
        // Device test events
        DEVICE_SELF_TEST_STARTED,
        DEVICE_SELF_TEST_COMPLETE,
        DEVICE_SELF_TEST_FAILED,
        
        // Discovery events
        DEVICE_DISCOVERY_STARTED,
        DEVICE_DISCOVERY_COMPLETE,
        DEVICE_DISCOVERY_ERROR,
        DEVICE_DISCOVERED,
        
        // Manager events
        MANAGER_STARTED,
        MANAGER_STOPPED,
        MANAGER_INITIALIZED,
        MANAGER_SHUTDOWN,
        MANAGER_ERROR,
        MANAGER_WARNING,
        MANAGER_INFO,
        
        // Security events
        AUTH_SUCCESS,
        AUTH_FAILURE,
        PERMISSION_DENIED,
        SECURITY_VIOLATION,
        
        // System events
        SYSTEM_LOAD_HIGH,
        SYSTEM_MEMORY_LOW,
        SYSTEM_TEMPERATURE_HIGH,
        SYSTEM_UP_TIME,
        
        // Custom events
        CUSTOM_EVENT
    };
    
    /**
     * @brief Event structure
     */
    struct Event {
        EventType type;
        std::string deviceId;
        std::string source;
        std::string message;
        Json::Value data;
        std::chrono::system_clock::time_point timestamp;
        int severity;  // 0-10, 10 = most severe
        std::string category;
        std::map<std::string, std::string> tags;
        std::string correlationId;
        std::string userId;
        std::string sessionId;
        
        Event() 
            : type(EventType::MANAGER_INFO), 
              severity(5),
              timestamp(std::chrono::system_clock::now()) {}
        
        Json::Value toJson() const {
            Json::Value json;
            json["type"] = static_cast<int>(type);
            json["typeName"] = eventTypeToString(type);
            json["deviceId"] = deviceId;
            json["source"] = source;
            json["message"] = message;
            json["data"] = data;
            json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp.time_since_epoch()).count();
            json["severity"] = severity;
            json["category"] = category;
            json["correlationId"] = correlationId;
            json["userId"] = userId;
            json["sessionId"] = sessionId;
            
            for (const auto& [key, value] : tags) {
                json["tags"][key] = value;
            }
            
            return json;
        }
        
        static Event fromJson(const Json::Value& json) {
            Event event;
            if (json.isMember("type")) {
                event.type = static_cast<EventType>(json["type"].asInt());
            }
            if (json.isMember("deviceId")) {
                event.deviceId = json["deviceId"].asString();
            }
            if (json.isMember("source")) {
                event.source = json["source"].asString();
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
            if (json.isMember("correlationId")) {
                event.correlationId = json["correlationId"].asString();
            }
            if (json.isMember("userId")) {
                event.userId = json["userId"].asString();
            }
            if (json.isMember("sessionId")) {
                event.sessionId = json["sessionId"].asString();
            }
            if (json.isMember("tags")) {
                for (const auto& key : json["tags"].getMemberNames()) {
                    event.tags[key] = json["tags"][key].asString();
                }
            }
            return event;
        }
    };
    
    /**
     * @brief Event callback type
     */
    using EventCallback = std::function<void(const Event&)>;
    
    /**
     * @brief Event filter for subscribing to specific events
     */
    struct EventFilter {
        std::vector<EventType> types;
        std::vector<std::string> deviceIds;
        std::vector<std::string> categories;
        std::vector<std::string> sources;
        std::map<std::string, std::string> tags;
        int minSeverity = 0;
        int maxSeverity = 10;
        std::string correlationId;
        std::string userId;
        
        bool matches(const Event& event) const {
            // Check type
            if (!types.empty()) {
                bool typeMatch = false;
                for (const auto& type : types) {
                    if (type == event.type) {
                        typeMatch = true;
                        break;
                    }
                }
                if (!typeMatch) return false;
            }
            
            // Check device ID
            if (!deviceIds.empty()) {
                bool deviceMatch = false;
                for (const auto& id : deviceIds) {
                    if (id == event.deviceId || id == "*") {
                        deviceMatch = true;
                        break;
                    }
                }
                if (!deviceMatch) return false;
            }
            
            // Check category
            if (!categories.empty()) {
                bool categoryMatch = false;
                for (const auto& cat : categories) {
                    if (cat == event.category) {
                        categoryMatch = true;
                        break;
                    }
                }
                if (!categoryMatch) return false;
            }
            
            // Check source
            if (!sources.empty()) {
                bool sourceMatch = false;
                for (const auto& src : sources) {
                    if (src == event.source) {
                        sourceMatch = true;
                        break;
                    }
                }
                if (!sourceMatch) return false;
            }
            
            // Check tags
            if (!tags.empty()) {
                for (const auto& [key, value] : tags) {
                    auto it = event.tags.find(key);
                    if (it == event.tags.end() || it->second != value) {
                        return false;
                    }
                }
            }
            
            // Check severity
            if (event.severity < minSeverity || event.severity > maxSeverity) {
                return false;
            }
            
            // Check correlation ID
            if (!correlationId.empty() && correlationId != event.correlationId) {
                return false;
            }
            
            // Check user ID
            if (!userId.empty() && userId != event.userId) {
                return false;
            }
            
            return true;
        }
    };
    
    /**
     * @brief Event subscription
     */
    struct Subscription {
        EventFilter filter;
        EventCallback callback;
        std::string id;
        bool once;
        
        Subscription() : once(false) {}
        Subscription(const EventFilter& filter, EventCallback callback, bool once = false)
            : filter(filter), callback(callback), once(once) {
            id = generateSubscriptionId();
        }
        
        static std::string generateSubscriptionId() {
            static std::atomic<uint64_t> counter(0);
            return "sub_" + std::to_string(counter++);
        }
    };
    
    /**
     * @brief Event statistics
     */
    struct EventStats {
        uint64_t totalEvents = 0;
        uint64_t errors = 0;
        uint64_t warnings = 0;
        std::map<EventType, uint64_t> eventCounts;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point lastEventTime;
        double eventsPerSecond = 0.0;
        uint64_t droppedEvents = 0;
        uint64_t processedEvents = 0;
        
        EventStats() {
            startTime = std::chrono::steady_clock::now();
            lastEventTime = startTime;
        }
        
        Json::Value toJson() const {
            Json::Value json;
            json["totalEvents"] = totalEvents;
            json["errors"] = errors;
            json["warnings"] = warnings;
            json["droppedEvents"] = droppedEvents;
            json["processedEvents"] = processedEvents;
            json["eventsPerSecond"] = eventsPerSecond;
            
            for (const auto& [type, count] : eventCounts) {
                json["eventCounts"][std::to_string(static_cast<int>(type))] = count;
            }
            
            auto now = std::chrono::steady_clock::now();
            json["uptimeSeconds"] = std::chrono::duration_cast<std::chrono::seconds>(
                now - startTime).count();
            json["lastEventSecondsAgo"] = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastEventTime).count();
            
            return json;
        }
    };
    
    /**
     * @brief Event priority levels
     */
    enum class EventPriority {
        LOWEST = 0,
        LOW = 2,
        NORMAL = 5,
        HIGH = 8,
        HIGHEST = 10
    };
    
    /**
     * @brief Event category helpers
     */
    static constexpr const char* CATEGORY_DEVICE = "device";
    static constexpr const char* CATEGORY_SYSTEM = "system";
    static constexpr const char* CATEGORY_SECURITY = "security";
    static constexpr const char* CATEGORY_MANAGER = "manager";
    static constexpr const char* CATEGORY_COMMAND = "command";
    static constexpr const char* CATEGORY_DATA = "data";
    static constexpr const char* CATEGORY_CONFIG = "config";
    static constexpr const char* CATEGORY_CALIBRATION = "calibration";
    static constexpr const char* CATEGORY_TEST = "test";
    static constexpr const char* CATEGORY_DISCOVERY = "discovery";
    
    /**
     * @brief Helper to create a DeviceEvent from a DeviceEvent::Type
     */
    static DeviceEvent::Type convertToDeviceEventType(EventType type);
    
    /**
     * @brief Helper to create a DeviceManagerEvents::EventType from a DeviceEvent::Type
     */
    static EventType convertFromDeviceEventType(DeviceEvent::Type type);
    
    /**
     * @brief Get event type name
     */
    static std::string eventTypeToString(EventType type);
    
    /**
     * @brief Get event type from string
     */
    static EventType stringToEventType(const std::string& str);
    
    /**
     * @brief Get event priority
     */
    static int getEventPriority(EventType type);
    
    /**
     * @brief Get event category
     */
    static std::string getEventCategory(EventType type);
    
    /**
     * @brief Check if event is error
     */
    static bool isErrorEvent(EventType type);
    
    /**
     * @brief Check if event is warning
     */
    static bool isWarningEvent(EventType type);
    
    /**
     * @brief Check if event is informational
     */
    static bool isInfoEvent(EventType type);
    
    /**
     * @brief Get human-readable description of event
     */
    static std::string getEventDescription(const Event& event);

private:
    static const std::map<EventType, std::string> eventTypeNames;
    static const std::map<std::string, EventType> nameEventTypes;
    static const std::map<EventType, int> eventPriorities;
    static const std::map<EventType, std::string> eventCategories;
    static const std::map<EventType, bool> errorEvents;
    static const std::map<EventType, bool> warningEvents;
};

} // namespace device_manager

#endif // DEVICE_MANAGER_EVENTS_H
