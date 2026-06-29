#include "device-manager/DeviceManagerEvents.h"
#include <map>
#include <string>

using namespace device_manager;

// Static member initialization
const std::map<DeviceManagerEvents::EventType, std::string> DeviceManagerEvents::eventTypeNames = {
    // Device lifecycle
    {EventType::DEVICE_ADDED, "DEVICE_ADDED"},
    {EventType::DEVICE_REMOVED, "DEVICE_REMOVED"},
    {EventType::DEVICE_UPDATED, "DEVICE_UPDATED"},
    {EventType::DEVICE_ENABLED, "DEVICE_ENABLED"},
    {EventType::DEVICE_DISABLED, "DEVICE_DISABLED"},
    {EventType::DEVICE_STARTED, "DEVICE_STARTED"},
    {EventType::DEVICE_STOPPED, "DEVICE_STOPPED"},
    {EventType::DEVICE_RESET, "DEVICE_RESET"},
    
    // Device status
    {EventType::DEVICE_STATUS_CHANGED, "DEVICE_STATUS_CHANGED"},
    {EventType::DEVICE_ERROR, "DEVICE_ERROR"},
    {EventType::DEVICE_WARNING, "DEVICE_WARNING"},
    {EventType::DEVICE_INFO, "DEVICE_INFO"},
    {EventType::DEVICE_ONLINE, "DEVICE_ONLINE"},
    {EventType::DEVICE_OFFLINE, "DEVICE_OFFLINE"},
    
    // Device data
    {EventType::DEVICE_DATA_AVAILABLE, "DEVICE_DATA_AVAILABLE"},
    {EventType::DEVICE_DATA_STREAM_STARTED, "DEVICE_DATA_STREAM_STARTED"},
    {EventType::DEVICE_DATA_STREAM_STOPPED, "DEVICE_DATA_STREAM_STOPPED"},
    {EventType::DEVICE_DATA_THRESHOLD_EXCEEDED, "DEVICE_DATA_THRESHOLD_EXCEEDED"},
    {EventType::DEVICE_DATA_TIMEOUT, "DEVICE_DATA_TIMEOUT"},
    
    // Device command
    {EventType::DEVICE_COMMAND_STARTED, "DEVICE_COMMAND_STARTED"},
    {EventType::DEVICE_COMMAND_COMPLETED, "DEVICE_COMMAND_COMPLETED"},
    {EventType::DEVICE_COMMAND_FAILED, "DEVICE_COMMAND_FAILED"},
    {EventType::DEVICE_COMMAND_TIMEOUT, "DEVICE_COMMAND_TIMEOUT"},
    
    // Device config
    {EventType::DEVICE_CONFIG_CHANGED, "DEVICE_CONFIG_CHANGED"},
    {EventType::DEVICE_CONFIG_LOADED, "DEVICE_CONFIG_LOADED"},
    {EventType::DEVICE_CONFIG_SAVED, "DEVICE_CONFIG_SAVED"},
    {EventType::DEVICE_CONFIG_ERROR, "DEVICE_CONFIG_ERROR"},
    
    // Device calibration
    {EventType::DEVICE_CALIBRATION_STARTED, "DEVICE_CALIBRATION_STARTED"},
    {EventType::DEVICE_CALIBRATION_COMPLETE, "DEVICE_CALIBRATION_COMPLETE"},
    {EventType::DEVICE_CALIBRATION_FAILED, "DEVICE_CALIBRATION_FAILED"},
    
    // Device test
    {EventType::DEVICE_SELF_TEST_STARTED, "DEVICE_SELF_TEST_STARTED"},
    {EventType::DEVICE_SELF_TEST_COMPLETE, "DEVICE_SELF_TEST_COMPLETE"},
    {EventType::DEVICE_SELF_TEST_FAILED, "DEVICE_SELF_TEST_FAILED"},
    
    // Discovery
    {EventType::DEVICE_DISCOVERY_STARTED, "DEVICE_DISCOVERY_STARTED"},
    {EventType::DEVICE_DISCOVERY_COMPLETE, "DEVICE_DISCOVERY_COMPLETE"},
    {EventType::DEVICE_DISCOVERY_ERROR, "DEVICE_DISCOVERY_ERROR"},
    {EventType::DEVICE_DISCOVERED, "DEVICE_DISCOVERED"},
    
    // Manager
    {EventType::MANAGER_STARTED, "MANAGER_STARTED"},
    {EventType::MANAGER_STOPPED, "MANAGER_STOPPED"},
    {EventType::MANAGER_INITIALIZED, "MANAGER_INITIALIZED"},
    {EventType::MANAGER_SHUTDOWN, "MANAGER_SHUTDOWN"},
    {EventType::MANAGER_ERROR, "MANAGER_ERROR"},
    {EventType::MANAGER_WARNING, "MANAGER_WARNING"},
    {EventType::MANAGER_INFO, "MANAGER_INFO"},
    
    // Security
    {EventType::AUTH_SUCCESS, "AUTH_SUCCESS"},
    {EventType::AUTH_FAILURE, "AUTH_FAILURE"},
    {EventType::PERMISSION_DENIED, "PERMISSION_DENIED"},
    {EventType::SECURITY_VIOLATION, "SECURITY_VIOLATION"},
    
    // System
    {EventType::SYSTEM_LOAD_HIGH, "SYSTEM_LOAD_HIGH"},
    {EventType::SYSTEM_MEMORY_LOW, "SYSTEM_MEMORY_LOW"},
    {EventType::SYSTEM_TEMPERATURE_HIGH, "SYSTEM_TEMPERATURE_HIGH"},
    {EventType::SYSTEM_UP_TIME, "SYSTEM_UP_TIME"},
    
    // Custom
    {EventType::CUSTOM_EVENT, "CUSTOM_EVENT"}
};

const std::map<std::string, DeviceManagerEvents::EventType> DeviceManagerEvents::nameEventTypes = []() {
    std::map<std::string, EventType> result;
    for (const auto& [type, name] : eventTypeNames) {
        result[name] = type;
    }
    return result;
}();

const std::map<DeviceManagerEvents::EventType, int> DeviceManagerEvents::eventPriorities = {
    // Highest priority - fatal errors
    {EventType::DEVICE_ERROR, 10},
    {EventType::MANAGER_ERROR, 10},
    {EventType::SECURITY_VIOLATION, 10},
    {EventType::DEVICE_COMMAND_FAILED, 9},
    {EventType::DEVICE_OFFLINE, 9},
    {EventType::MANAGER_SHUTDOWN, 8},
    
    // High priority - warnings
    {EventType::DEVICE_WARNING, 7},
    {EventType::MANAGER_WARNING, 7},
    {EventType::DEVICE_DATA_TIMEOUT, 7},
    {EventType::DEVICE_DATA_THRESHOLD_EXCEEDED, 7},
    {EventType::DEVICE_COMMAND_TIMEOUT, 7},
    {EventType::AUTH_FAILURE, 7},
    {EventType::PERMISSION_DENIED, 7},
    {EventType::DEVICE_CONFIG_ERROR, 7},
    {EventType::DEVICE_CALIBRATION_FAILED, 7},
    {EventType::DEVICE_SELF_TEST_FAILED, 7},
    {EventType::DEVICE_DISCOVERY_ERROR, 7},
    {EventType::SYSTEM_LOAD_HIGH, 7},
    {EventType::SYSTEM_MEMORY_LOW, 7},
    {EventType::SYSTEM_TEMPERATURE_HIGH, 7},
    
    // Medium priority - important events
    {EventType::DEVICE_STATUS_CHANGED, 6},
    {EventType::DEVICE_STARTED, 6},
    {EventType::DEVICE_STOPPED, 6},
    {EventType::DEVICE_RESET, 6},
    {EventType::DEVICE_ONLINE, 6},
    {EventType::DEVICE_ADDED, 5},
    {EventType::DEVICE_REMOVED, 5},
    {EventType::DEVICE_ENABLED, 5},
    {EventType::DEVICE_DISABLED, 5},
    {EventType::DEVICE_UPDATED, 5},
    {EventType::DEVICE_CONFIG_CHANGED, 5},
    {EventType::DEVICE_CONFIG_LOADED, 5},
    {EventType::DEVICE_CONFIG_SAVED, 5},
    {EventType::DEVICE_CALIBRATION_STARTED, 5},
    {EventType::DEVICE_CALIBRATION_COMPLETE, 5},
    {EventType::DEVICE_SELF_TEST_STARTED, 5},
    {EventType::DEVICE_SELF_TEST_COMPLETE, 5},
    {EventType::DEVICE_DISCOVERY_STARTED, 5},
    {EventType::DEVICE_DISCOVERY_COMPLETE, 5},
    {EventType::DEVICE_DISCOVERED, 5},
    {EventType::MANAGER_STARTED, 5},
    {EventType::MANAGER_INITIALIZED, 5},
    {EventType::AUTH_SUCCESS, 5},
    {EventType::SYSTEM_UP_TIME, 5},
    {EventType::DEVICE_COMMAND_STARTED, 4},
    {EventType::DEVICE_COMMAND_COMPLETED, 4},
    {EventType::DEVICE_DATA_STREAM_STARTED, 4},
    {EventType::DEVICE_DATA_STREAM_STOPPED, 4},
    {EventType::DEVICE_DATA_AVAILABLE, 3},
    
    // Low priority - informational
    {EventType::DEVICE_INFO, 2},
    {EventType::MANAGER_INFO, 2},
    {EventType::MANAGER_STOPPED, 2},
    {EventType::CUSTOM_EVENT, 1}
};

const std::map<DeviceManagerEvents::EventType, std::string> DeviceManagerEvents::eventCategories = {
    {EventType::DEVICE_ADDED, CATEGORY_DEVICE},
    {EventType::DEVICE_REMOVED, CATEGORY_DEVICE},
    {EventType::DEVICE_UPDATED, CATEGORY_DEVICE},
    {EventType::DEVICE_ENABLED, CATEGORY_DEVICE},
    {EventType::DEVICE_DISABLED, CATEGORY_DEVICE},
    {EventType::DEVICE_STARTED, CATEGORY_DEVICE},
    {EventType::DEVICE_STOPPED, CATEGORY_DEVICE},
    {EventType::DEVICE_RESET, CATEGORY_DEVICE},
    {EventType::DEVICE_STATUS_CHANGED, CATEGORY_DEVICE},
    {EventType::DEVICE_ERROR, CATEGORY_DEVICE},
    {EventType::DEVICE_WARNING, CATEGORY_DEVICE},
    {EventType::DEVICE_INFO, CATEGORY_DEVICE},
    {EventType::DEVICE_ONLINE, CATEGORY_DEVICE},
    {EventType::DEVICE_OFFLINE, CATEGORY_DEVICE},
    {EventType::DEVICE_DATA_AVAILABLE, CATEGORY_DATA},
    {EventType::DEVICE_DATA_STREAM_STARTED, CATEGORY_DATA},
    {EventType::DEVICE_DATA_STREAM_STOPPED, CATEGORY_DATA},
    {EventType::DEVICE_DATA_THRESHOLD_EXCEEDED, CATEGORY_DATA},
    {EventType::DEVICE_DATA_TIMEOUT, CATEGORY_DATA},
    {EventType::DEVICE_COMMAND_STARTED, CATEGORY_COMMAND},
    {EventType::DEVICE_COMMAND_COMPLETED, CATEGORY_COMMAND},
    {EventType::DEVICE_COMMAND_FAILED, CATEGORY_COMMAND},
    {EventType::DEVICE_COMMAND_TIMEOUT, CATEGORY_COMMAND},
    {EventType::DEVICE_CONFIG_CHANGED, CATEGORY_CONFIG},
    {EventType::DEVICE_CONFIG_LOADED, CATEGORY_CONFIG},
    {EventType::DEVICE_CONFIG_SAVED, CATEGORY_CONFIG},
    {EventType::DEVICE_CONFIG_ERROR, CATEGORY_CONFIG},
    {EventType::DEVICE_CALIBRATION_STARTED, CATEGORY_CALIBRATION},
    {EventType::DEVICE_CALIBRATION_COMPLETE, CATEGORY_CALIBRATION},
    {EventType::DEVICE_CALIBRATION_FAILED, CATEGORY_CALIBRATION},
    {EventType::DEVICE_SELF_TEST_STARTED, CATEGORY_TEST},
    {EventType::DEVICE_SELF_TEST_COMPLETE, CATEGORY_TEST},
    {EventType::DEVICE_SELF_TEST_FAILED, CATEGORY_TEST},
    {EventType::DEVICE_DISCOVERY_STARTED, CATEGORY_DISCOVERY},
    {EventType::DEVICE_DISCOVERY_COMPLETE, CATEGORY_DISCOVERY},
    {EventType::DEVICE_DISCOVERY_ERROR, CATEGORY_DISCOVERY},
    {EventType::DEVICE_DISCOVERED, CATEGORY_DISCOVERY},
    {EventType::MANAGER_STARTED, CATEGORY_MANAGER},
    {EventType::MANAGER_STOPPED, CATEGORY_MANAGER},
    {EventType::MANAGER_INITIALIZED, CATEGORY_MANAGER},
    {EventType::MANAGER_SHUTDOWN, CATEGORY_MANAGER},
    {EventType::MANAGER_ERROR, CATEGORY_MANAGER},
    {EventType::MANAGER_WARNING, CATEGORY_MANAGER},
    {EventType::MANAGER_INFO, CATEGORY_MANAGER},
    {EventType::AUTH_SUCCESS, CATEGORY_SECURITY},
    {EventType::AUTH_FAILURE, CATEGORY_SECURITY},
    {EventType::PERMISSION_DENIED, CATEGORY_SECURITY},
    {EventType::SECURITY_VIOLATION, CATEGORY_SECURITY},
    {EventType::SYSTEM_LOAD_HIGH, CATEGORY_SYSTEM},
    {EventType::SYSTEM_MEMORY_LOW, CATEGORY_SYSTEM},
    {EventType::SYSTEM_TEMPERATURE_HIGH, CATEGORY_SYSTEM},
    {EventType::SYSTEM_UP_TIME, CATEGORY_SYSTEM},
    {EventType::CUSTOM_EVENT, "custom"}
};

const std::map<DeviceManagerEvents::EventType, bool> DeviceManagerEvents::errorEvents = {
    {EventType::DEVICE_ERROR, true},
    {EventType::MANAGER_ERROR, true},
    {EventType::SECURITY_VIOLATION, true},
    {EventType::DEVICE_COMMAND_FAILED, true},
    {EventType::DEVICE_CALIBRATION_FAILED, true},
    {EventType::DEVICE_SELF_TEST_FAILED, true},
    {EventType::DEVICE_DISCOVERY_ERROR, true},
    {EventType::DEVICE_CONFIG_ERROR, true},
    {EventType::AUTH_FAILURE, true},
    {EventType::PERMISSION_DENIED, true}
};

const std::map<DeviceManagerEvents::EventType, bool> DeviceManagerEvents::warningEvents = {
    {EventType::DEVICE_WARNING, true},
    {EventType::MANAGER_WARNING, true},
    {EventType::DEVICE_DATA_THRESHOLD_EXCEEDED, true},
    {EventType::DEVICE_DATA_TIMEOUT, true},
    {EventType::DEVICE_COMMAND_TIMEOUT, true},
    {EventType::SYSTEM_LOAD_HIGH, true},
    {EventType::SYSTEM_MEMORY_LOW, true},
    {EventType::SYSTEM_TEMPERATURE_HIGH, true},
    {EventType::DEVICE_OFFLINE, true},
    {EventType::DEVICE_STATUS_CHANGED, true},
    {EventType::DEVICE_STOPPED, true}
};

/**
 * @brief Convert DeviceEvent::Type to EventType
 */
DeviceEvent::Type DeviceManagerEvents::convertToDeviceEventType(EventType type) {
    switch (type) {
        case EventType::DEVICE_ADDED: return DeviceEvent::DEVICE_ADDED;
        case EventType::DEVICE_REMOVED: return DeviceEvent::DEVICE_REMOVED;
        case EventType::DEVICE_UPDATED: return DeviceEvent::DEVICE_CHANGED;
        case EventType::DEVICE_STATUS_CHANGED: return DeviceEvent::DEVICE_STATUS_CHANGED;
        case EventType::DEVICE_ERROR: return DeviceEvent::DEVICE_ERROR;
        case EventType::DEVICE_WARNING: return DeviceEvent::DEVICE_WARNING;
        case EventType::DEVICE_INFO: return DeviceEvent::DEVICE_INFO;
        case EventType::DEVICE_DATA_AVAILABLE: return DeviceEvent::DEVICE_DATA_AVAILABLE;
        case EventType::DEVICE_DATA_STREAM_STARTED: return DeviceEvent::DEVICE_STREAM_STARTED;
        case EventType::DEVICE_DATA_STREAM_STOPPED: return DeviceEvent::DEVICE_STREAM_STOPPED;
        case EventType::DEVICE_COMMAND_STARTED: return DeviceEvent::DEVICE_COMMAND_COMPLETED;
        case EventType::DEVICE_COMMAND_COMPLETED: return DeviceEvent::DEVICE_COMMAND_COMPLETED;
        case EventType::DEVICE_COMMAND_FAILED: return DeviceEvent::DEVICE_COMMAND_FAILED;
        case EventType::DEVICE_CALIBRATION_STARTED: return DeviceEvent::DEVICE_CALIBRATION_COMPLETE;
        case EventType::DEVICE_CALIBRATION_COMPLETE: return DeviceEvent::DEVICE_CALIBRATION_COMPLETE;
        case EventType::DEVICE_SELF_TEST_STARTED: return DeviceEvent::DEVICE_SELF_TEST_COMPLETE;
        case EventType::DEVICE_SELF_TEST_COMPLETE: return DeviceEvent::DEVICE_SELF_TEST_COMPLETE;
        case EventType::DEVICE_DISCOVERED: return DeviceEvent::DEVICE_ADDED;
        default: return DeviceEvent::DEVICE_INFO;
    }
}

/**
 * @brief Convert DeviceEvent::Type from EventType
 */
DeviceManagerEvents::EventType DeviceManagerEvents::convertFromDeviceEventType(DeviceEvent::Type type) {
    switch (type) {
        case DeviceEvent::DEVICE_ADDED: return EventType::DEVICE_ADDED;
        case DeviceEvent::DEVICE_REMOVED: return EventType::DEVICE_REMOVED;
        case DeviceEvent::DEVICE_CHANGED: return EventType::DEVICE_UPDATED;
        case DeviceEvent::DEVICE_STATUS_CHANGED: return EventType::DEVICE_STATUS_CHANGED;
        case DeviceEvent::DEVICE_ERROR: return EventType::DEVICE_ERROR;
        case DeviceEvent::DEVICE_WARNING: return EventType::DEVICE_WARNING;
        case DeviceEvent::DEVICE_INFO: return EventType::DEVICE_INFO;
        case DeviceEvent::DEVICE_DATA_AVAILABLE: return EventType::DEVICE_DATA_AVAILABLE;
        case DeviceEvent::DEVICE_STREAM_STARTED: return EventType::DEVICE_DATA_STREAM_STARTED;
        case DeviceEvent::DEVICE_STREAM_STOPPED: return EventType::DEVICE_DATA_STREAM_STOPPED;
        case DeviceEvent::DEVICE_COMMAND_COMPLETED: return EventType::DEVICE_COMMAND_COMPLETED;
        case DeviceEvent::DEVICE_COMMAND_FAILED: return EventType::DEVICE_COMMAND_FAILED;
        case DeviceEvent::DEVICE_CALIBRATION_COMPLETE: return EventType::DEVICE_CALIBRATION_COMPLETE;
        case DeviceEvent::DEVICE_SELF_TEST_COMPLETE: return EventType::DEVICE_SELF_TEST_COMPLETE;
        default: return EventType::DEVICE_INFO;
    }
}

/**
 * @brief Get event type name
 */
std::string DeviceManagerEvents::eventTypeToString(EventType type) {
    auto it = eventTypeNames.find(type);
    if (it != eventTypeNames.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

/**
 * @brief Get event type from string
 */
DeviceManagerEvents::EventType DeviceManagerEvents::stringToEventType(const std::string& str) {
    auto it = nameEventTypes.find(str);
    if (it != nameEventTypes.end()) {
        return it->second;
    }
    return EventType::MANAGER_INFO;
}

/**
 * @brief Get event priority
 */
int DeviceManagerEvents::getEventPriority(EventType type) {
    auto it = eventPriorities.find(type);
    if (it != eventPriorities.end()) {
        return it->second;
    }
    return 5; // Default priority
}

/**
 * @brief Get event category
 */
std::string DeviceManagerEvents::getEventCategory(EventType type) {
    auto it = eventCategories.find(type);
    if (it != eventCategories.end()) {
        return it->second;
    }
    return "unknown";
}

/**
 * @brief Check if event is error
 */
bool DeviceManagerEvents::isErrorEvent(EventType type) {
    auto it = errorEvents.find(type);
    if (it != errorEvents.end()) {
        return it->second;
    }
    return false;
}

/**
 * @brief Check if event is warning
 */
bool DeviceManagerEvents::isWarningEvent(EventType type) {
    auto it = warningEvents.find(type);
    if (it != warningEvents.end()) {
        return it->second;
    }
    return false;
}

/**
 * @brief Check if event is informational
 */
bool DeviceManagerEvents::isInfoEvent(EventType type) {
    return !isErrorEvent(type) && !isWarningEvent(type);
}

/**
 * @brief Get human-readable description of event
 */
std::string DeviceManagerEvents::getEventDescription(const Event& event) {
    std::string desc = eventTypeToString(event.type);
    
    if (!event.deviceId.empty()) {
        desc += " [device: " + event.deviceId + "]";
    }
    
    if (!event.message.empty()) {
        desc += ": " + event.message;
    }
    
    if (!event.category.empty()) {
        desc += " (category: " + event.category + ")";
    }
    
    return desc;
}

/**
 * @brief Create DeviceEvent from Event
 */
DeviceEvent DeviceManagerEvents::createDeviceEvent(const Event& event) {
    DeviceEvent deviceEvent;
    deviceEvent.type = convertToDeviceEventType(event.type);
    deviceEvent.deviceId = event.deviceId;
    deviceEvent.timestamp = event.timestamp;
    deviceEvent.data = event.data;
    deviceEvent.message = event.message;
    return deviceEvent;
}

/**
 * @brief Create Event from DeviceEvent
 */
DeviceManagerEvents::Event DeviceManagerEvents::createEvent(const DeviceEvent& deviceEvent) {
    Event event;
    event.type = convertFromDeviceEventType(deviceEvent.type);
    event.deviceId = deviceEvent.deviceId;
    event.timestamp = deviceEvent.timestamp;
    event.data = deviceEvent.data;
    event.message = deviceEvent.message;
    event.category = getEventCategory(event.type);
    event.severity = getEventPriority(event.type);
    return event;
}
