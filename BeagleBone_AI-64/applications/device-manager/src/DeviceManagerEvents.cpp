#include "device-manager/DeviceManagerEvents.h"

namespace device_manager {

std::string DeviceManagerEvents::eventTypeToString(DeviceEvent::Type type) {
    switch (type) {
        case DeviceEvent::DEVICE_ADDED: return "DEVICE_ADDED";
        case DeviceEvent::DEVICE_REMOVED: return "DEVICE_REMOVED";
        case DeviceEvent::DEVICE_CHANGED: return "DEVICE_CHANGED";
        case DeviceEvent::DEVICE_STATUS_CHANGED: return "DEVICE_STATUS_CHANGED";
        case DeviceEvent::DEVICE_DATA_AVAILABLE: return "DEVICE_DATA_AVAILABLE";
        case DeviceEvent::DEVICE_COMMAND_COMPLETED: return "DEVICE_COMMAND_COMPLETED";
        case DeviceEvent::DEVICE_COMMAND_FAILED: return "DEVICE_COMMAND_FAILED";
        case DeviceEvent::DEVICE_CALIBRATION_COMPLETE: return "DEVICE_CALIBRATION_COMPLETE";
        case DeviceEvent::DEVICE_SELF_TEST_COMPLETE: return "DEVICE_SELF_TEST_COMPLETE";
        case DeviceEvent::DEVICE_ERROR: return "DEVICE_ERROR";
        case DeviceEvent::DEVICE_WARNING: return "DEVICE_WARNING";
        case DeviceEvent::DEVICE_INFO: return "DEVICE_INFO";
        case DeviceEvent::DEVICE_STREAM_STARTED: return "DEVICE_STREAM_STARTED";
        case DeviceEvent::DEVICE_STREAM_STOPPED: return "DEVICE_STREAM_STOPPED";
        default: return "UNKNOWN";
    }
}

DeviceEvent::Type DeviceManagerEvents::stringToEventType(const std::string& str) {
    if (str == "DEVICE_ADDED") return DeviceEvent::DEVICE_ADDED;
    if (str == "DEVICE_REMOVED") return DeviceEvent::DEVICE_REMOVED;
    if (str == "DEVICE_CHANGED") return DeviceEvent::DEVICE_CHANGED;
    if (str == "DEVICE_STATUS_CHANGED") return DeviceEvent::DEVICE_STATUS_CHANGED;
    if (str == "DEVICE_DATA_AVAILABLE") return DeviceEvent::DEVICE_DATA_AVAILABLE;
    if (str == "DEVICE_COMMAND_COMPLETED") return DeviceEvent::DEVICE_COMMAND_COMPLETED;
    if (str == "DEVICE_COMMAND_FAILED") return DeviceEvent::DEVICE_COMMAND_FAILED;
    if (str == "DEVICE_CALIBRATION_COMPLETE") return DeviceEvent::DEVICE_CALIBRATION_COMPLETE;
    if (str == "DEVICE_SELF_TEST_COMPLETE") return DeviceEvent::DEVICE_SELF_TEST_COMPLETE;
    if (str == "DEVICE_ERROR") return DeviceEvent::DEVICE_ERROR;
    if (str == "DEVICE_WARNING") return DeviceEvent::DEVICE_WARNING;
    if (str == "DEVICE_INFO") return DeviceEvent::DEVICE_INFO;
    if (str == "DEVICE_STREAM_STARTED") return DeviceEvent::DEVICE_STREAM_STARTED;
    if (str == "DEVICE_STREAM_STOPPED") return DeviceEvent::DEVICE_STREAM_STOPPED;
    return DeviceEvent::DEVICE_INFO;
}

} // namespace device_manager
