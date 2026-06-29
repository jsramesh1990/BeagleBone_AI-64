#include "device-manager/Device.h"
#include <chrono>
#include <thread>

using namespace device_manager;

Device::Device(const DeviceInfo& deviceInfo) 
    : info(deviceInfo), status(DeviceStatus::INITIALIZING), 
      isRunning(false) {
    lastActivity = std::chrono::steady_clock::now();
}

Device::~Device() {
    if (isRunning) {
        stop();
    }
}

Json::Value Device::getProperty(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = info.properties.find(key);
    if (it != info.properties.end()) {
        return it->second;
    }
    return Json::nullValue;
}

void Device::setProperty(const std::string& key, const Json::Value& value) {
    std::lock_guard<std::mutex> lock(mutex);
    info.properties[key] = value;
}

Json::Value Device::getMetadata(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = info.metadata.find(key);
    if (it != info.metadata.end()) {
        return it->second;
    }
    return Json::nullValue;
}

void Device::setMetadata(const std::string& key, const Json::Value& value) {
    std::lock_guard<std::mutex> lock(mutex);
    info.metadata[key] = value;
}

void Device::setStatus(DeviceStatus newStatus) {
    std::lock_guard<std::mutex> lock(mutex);
    if (status != newStatus) {
        status = newStatus;
        DeviceEvent event;
        event.type = DeviceEvent::DEVICE_STATUS_CHANGED;
        event.deviceId = info.id;
        event.timestamp = getCurrentTime();
        event.data["newStatus"] = static_cast<int>(newStatus);
        event.data["oldStatus"] = static_cast<int>(status);
        emitEvent(event);
    }
}

void Device::emitEvent(const DeviceEvent& event) {
    if (eventCallback) {
        try {
            eventCallback(event);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

void Device::emitData(const DeviceSample& sample) {
    if (dataCallback) {
        try {
            dataCallback(sample);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

void Device::emitCommandResult(const DeviceCommand& command, const DeviceSample& result) {
    if (commandCallback) {
        try {
            commandCallback(command, result);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

std::chrono::system_clock::time_point Device::getCurrentTime() {
    return std::chrono::system_clock::now();
}

bool Device::validateConfig(const Json::Value& config) const {
    // Base implementation - subclasses should override
    return config.isObject();
}

bool Device::waitForReady(int timeoutMs) {
    auto start = std::chrono::steady_clock::now();
    while (status != DeviceStatus::OK && status != DeviceStatus::ERROR) {
        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            if (elapsed.count() > timeoutMs) {
                return false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return status == DeviceStatus::OK;
}
