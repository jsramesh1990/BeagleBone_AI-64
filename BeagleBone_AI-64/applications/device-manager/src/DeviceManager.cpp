#include "device-manager/DeviceManager.h"
#include "device-manager/DeviceFactory.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

using namespace device_manager;

DeviceManager& DeviceManager::getInstance() {
    static DeviceManager instance;
    return instance;
}

DeviceManager::DeviceManager() {
    stats.startTime = std::chrono::steady_clock::now();
    DeviceFactory::getInstance().registerBuiltInDevices();
}

DeviceManager::~DeviceManager() {
    stop();
}

bool DeviceManager::initialize(const Json::Value& config) {
    if (initialized) {
        return true;
    }
    
    this->config = config;
    
    // Load devices from config if provided
    if (config.isMember("devices")) {
        loadDevicesFromConfig(config["devices"]);
    }
    
    // Load from file if path provided
    if (config.isMember("config_path")) {
        configPath = config["config_path"].asString();
        loadConfig(configPath);
    }
    
    initialized = true;
    return true;
}

bool DeviceManager::start() {
    if (running) {
        return true;
    }
    
    if (!initialized) {
        return false;
    }
    
    running = true;
    monitorThread = std::thread(&DeviceManager::monitorLoop, this);
    return true;
}

bool DeviceManager::stop() {
    if (!running) {
        return true;
    }
    
    running = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    
    // Stop all devices
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        try {
            device->stop();
        } catch (...) {
            // Ignore errors during shutdown
        }
    }
    
    return true;
}

bool DeviceManager::addDevice(std::unique_ptr<Device> device) {
    if (!device) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(devicesMutex);
    
    std::string id = device->getId();
    if (devices.find(id) != devices.end()) {
        return false;
    }
    
    // Initialize the device
    if (!device->initialize()) {
        return false;
    }
    
    // Set callbacks
    device->setEventCallback([this](const DeviceEvent& event) {
        processDeviceEvent(event);
    });
    
    device->setDataCallback([this](const DeviceSample& sample) {
        processDeviceData(sample);
    });
    
    device->setCommandCallback([this](const DeviceCommand& cmd, const DeviceSample& result) {
        processCommandResult(cmd, result);
    });
    
    // Start the device
    if (!device->start()) {
        return false;
    }
    
    devices[id] = std::move(device);
    stats.devicesAdded++;
    
    // Emit device added event
    DeviceEvent event;
    event.type = DeviceEvent::DEVICE_ADDED;
    event.deviceId = id;
    event.timestamp = Device::getCurrentTime();
    event.data["deviceInfo"] = devices[id]->getInfo().toJson();
    emitEvent(event);
    
    return true;
}

bool DeviceManager::removeDevice(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex);
    
    auto it = devices.find(deviceId);
    if (it == devices.end()) {
        return false;
    }
    
    // Stop the device
    try {
        it->second->stop();
    } catch (...) {
        // Ignore errors during removal
    }
    
    // Emit device removed event
    DeviceEvent event;
    event.type = DeviceEvent::DEVICE_REMOVED;
    event.deviceId = deviceId;
    event.timestamp = Device::getCurrentTime();
    emitEvent(event);
    
    devices.erase(it);
    stats.devicesRemoved++;
    
    return true;
}

Device* DeviceManager::getDevice(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex);
    auto it = devices.find(deviceId);
    if (it != devices.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Device*> DeviceManager::getAllDevices() {
    std::vector<Device*> result;
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        result.push_back(device.get());
    }
    return result;
}

std::vector<Device*> DeviceManager::getDevicesByType(DeviceType type) {
    std::vector<Device*> result;
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        if (device->getType() == type) {
            result.push_back(device.get());
        }
    }
    return result;
}

std::vector<Device*> DeviceManager::getDevicesByStatus(DeviceStatus status) {
    std::vector<Device*> result;
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        if (device->getStatus() == status) {
            result.push_back(device.get());
        }
    }
    return result;
}

DeviceInfo DeviceManager::getDeviceInfo(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(devicesMutex);
    auto it = devices.find(deviceId);
    if (it != devices.end()) {
        return it->second->getInfo();
    }
    return DeviceInfo();
}

std::vector<DeviceInfo> DeviceManager::getAllDeviceInfo() {
    std::vector<DeviceInfo> result;
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        result.push_back(device->getInfo());
    }
    return result;
}

int DeviceManager::discoverDevices(DiscoveryCallback callback, 
                                  DeviceType type, int timeoutMs) {
    int discovered = 0;
    
    // Simulate device discovery
    // In real implementation, this would scan buses, enumerate devices, etc.
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        // Check for timeout
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        if (elapsed.count() > timeoutMs) {
            break;
        }
        
        // In real implementation, this would discover actual devices
        // For now, just simulate discovery of known devices
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return discovered;
}

bool DeviceManager::readData(const std::string& deviceId, DeviceSample& data, int timeoutMs) {
    Device* device = getDevice(deviceId);
    if (!device) {
        stats.readErrors++;
        return false;
    }
    
    bool result = device->readData(data, timeoutMs);
    if (!result) {
        stats.readErrors++;
    }
    return result;
}

bool DeviceManager::writeData(const std::string& deviceId, const Json::Value& data, int timeoutMs) {
    Device* device = getDevice(deviceId);
    if (!device) {
        stats.writeErrors++;
        return false;
    }
    
    bool result = device->writeData(data, timeoutMs);
    if (!result) {
        stats.writeErrors++;
    }
    return result;
}

bool DeviceManager::executeCommand(const std::string& deviceId, const DeviceCommand& command,
                                  DeviceSample& result, int timeoutMs) {
    Device* device = getDevice(deviceId);
    if (!device) {
        stats.commandErrors++;
        return false;
    }
    
    bool success = device->executeCommand(command, result, timeoutMs);
    if (!success) {
        stats.commandErrors++;
    }
    return success;
}

bool DeviceManager::calibrateDevice(const std::string& deviceId, const Json::Value& params,
                                   Json::Value& result, int timeoutMs) {
    Device* device = getDevice(deviceId);
    if (!device) {
        return false;
    }
    
    return device->calibrate(params, result, timeoutMs);
}

bool DeviceManager::selfTestDevice(const std::string& deviceId, Json::Value& result, int timeoutMs) {
    Device* device = getDevice(deviceId);
    if (!device) {
        return false;
    }
    
    return device->selfTest(result, timeoutMs);
}

bool DeviceManager::configureDevice(const std::string& deviceId, const Json::Value& config, int timeoutMs) {
    Device* device = getDevice(deviceId);
    if (!device) {
        return false;
    }
    
    return device->configure(config, timeoutMs);
}

void DeviceManager::monitorLoop() {
    while (running) {
        checkDevices();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void DeviceManager::checkDevices() {
    std::lock_guard<std::mutex> lock(devicesMutex);
    
    for (auto& [id, device] : devices) {
        // Check device status
        DeviceStatus status = device->getStatus();
        if (status == DeviceStatus::ERROR || status == DeviceStatus::DISCONNECTED) {
            // Attempt to recover
            try {
                device->reset();
                device->start();
            } catch (...) {
                // Recovery failed
            }
        }
    }
}

void DeviceManager::processDeviceEvent(const DeviceEvent& event) {
    emitEvent(event);
}

void DeviceManager::processDeviceData(const DeviceSample& sample) {
    // Forward data to callbacks
    // In real implementation, this would handle data processing, storage, etc.
}

void DeviceManager::processCommandResult(const DeviceCommand& command, const DeviceSample& result) {
    if (commandCallback) {
        try {
            commandCallback(command, result);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

void DeviceManager::emitEvent(const DeviceEvent& event) {
    if (eventCallback) {
        try {
            eventCallback(event);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

Json::Value DeviceManager::getStats() const {
    Json::Value statsJson;
    statsJson["devices_added"] = stats.devicesAdded.load();
    statsJson["devices_removed"] = stats.devicesRemoved.load();
    statsJson["devices_discovered"] = stats.devicesDiscovered.load();
    statsJson["read_errors"] = stats.readErrors.load();
    statsJson["write_errors"] = stats.writeErrors.load();
    statsJson["command_errors"] = stats.commandErrors.load();
    statsJson["total_devices"] = devices.size();
    
    auto now = std::chrono::steady_clock::now();
    statsJson["uptime_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(
        now - stats.startTime).count();
    
    return statsJson;
}

void DeviceManager::resetStats() {
    std::lock_guard<std::mutex> lock(statsMutex);
    stats.devicesAdded = 0;
    stats.devicesRemoved = 0;
    stats.devicesDiscovered = 0;
    stats.readErrors = 0;
    stats.writeErrors = 0;
    stats.commandErrors = 0;
    stats.startTime = std::chrono::steady_clock::now();
}

bool DeviceManager::saveConfig(const std::string& path) const {
    Json::Value config = saveDevicesToConfig();
    
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(config, &file);
    file.close();
    
    return true;
}

bool DeviceManager::loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    Json::Value config;
    Json::CharReaderBuilder builder;
    std::string errs;
    if (!Json::parseFromStream(builder, file, &config, &errs)) {
        return false;
    }
    file.close();
    
    if (config.isMember("devices")) {
        return loadDevicesFromConfig(config["devices"]);
    }
    
    return true;
}

bool DeviceManager::loadDevicesFromConfig(const Json::Value& config) {
    if (!config.isArray()) {
        return false;
    }
    
    auto& factory = DeviceFactory::getInstance();
    
    for (const auto& deviceConfig : config) {
        DeviceInfo info = DeviceInfo::fromJson(deviceConfig);
        auto device = factory.createDevice(info);
        if (device) {
            addDevice(std::move(device));
        }
    }
    
    return true;
}

Json::Value DeviceManager::saveDevicesToConfig() const {
    Json::Value config(Json::arrayValue);
    
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (const auto& [id, device] : devices) {
        config.append(device->getInfo().toJson());
    }
    
    return config;
}
