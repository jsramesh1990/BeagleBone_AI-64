#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <json/json.h>
#include "Device.h"
#include "DeviceTypes.h"

namespace device_manager {

/**
 * @brief Device Manager - manages all devices
 * 
 * Responsible for device discovery, initialization,
 * monitoring, and lifecycle management
 */
class DeviceManager {
public:
    /**
     * @brief Device discovery callback
     */
    using DiscoveryCallback = std::function<void(const DeviceInfo&)>;
    
    /**
     * @brief Device event callback
     */
    using EventCallback = std::function<void(const DeviceEvent&)>;
    
    /**
     * @brief Device command callback
     */
    using CommandCallback = std::function<void(const DeviceCommand&, const DeviceSample&)>;
    
    /**
     * @brief Get singleton instance
     */
    static DeviceManager& getInstance();
    
    /**
     * @brief Initialize the device manager
     * @param config Configuration JSON
     * @return true on success
     */
    bool initialize(const Json::Value& config = Json::nullValue);
    
    /**
     * @brief Start the device manager
     * @return true on success
     */
    bool start();
    
    /**
     * @brief Stop the device manager
     * @return true on success
     */
    bool stop();
    
    /**
     * @brief Check if running
     */
    bool isRunning() const { return running; }
    
    /**
     * @brief Add device
     * @param device Device to add
     * @return true on success
     */
    bool addDevice(std::unique_ptr<Device> device);
    
    /**
     * @brief Remove device
     * @param deviceId Device ID
     * @return true on success
     */
    bool removeDevice(const std::string& deviceId);
    
    /**
     * @brief Get device by ID
     * @param deviceId Device ID
     * @return Device pointer or nullptr
     */
    Device* getDevice(const std::string& deviceId);
    
    /**
     * @brief Get all devices
     */
    std::vector<Device*> getAllDevices();
    
    /**
     * @brief Get devices by type
     * @param type Device type
     */
    std::vector<Device*> getDevicesByType(DeviceType type);
    
    /**
     * @brief Get devices by status
     * @param status Device status
     */
    std::vector<Device*> getDevicesByStatus(DeviceStatus status);
    
    /**
     * @brief Get device information
     * @param deviceId Device ID
     * @return DeviceInfo or empty
     */
    DeviceInfo getDeviceInfo(const std::string& deviceId);
    
    /**
     * @brief Get all device information
     */
    std::vector<DeviceInfo> getAllDeviceInfo();
    
    /**
     * @brief Discover devices
     * @param callback Callback for discovered devices
     * @param type Filter by type
     * @param timeoutMs Timeout in milliseconds
     * @return Number of devices discovered
     */
    int discoverDevices(DiscoveryCallback callback, 
                        DeviceType type = DeviceType::UNKNOWN,
                        int timeoutMs = 5000);
    
    /**
     * @brief Read data from device
     * @param deviceId Device ID
     * @param data Data sample
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool readData(const std::string& deviceId, DeviceSample& data, int timeoutMs = 5000);
    
    /**
     * @brief Write data to device
     * @param deviceId Device ID
     * @param data Data to write
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool writeData(const std::string& deviceId, const Json::Value& data, int timeoutMs = 5000);
    
    /**
     * @brief Execute command on device
     * @param deviceId Device ID
     * @param command Command to execute
     * @param result Command result
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool executeCommand(const std::string& deviceId, const DeviceCommand& command,
                        DeviceSample& result, int timeoutMs = 5000);
    
    /**
     * @brief Calibrate device
     * @param deviceId Device ID
     * @param params Calibration parameters
     * @param result Calibration result
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool calibrateDevice(const std::string& deviceId, const Json::Value& params,
                         Json::Value& result, int timeoutMs = 5000);
    
    /**
     * @brief Run self-test on device
     * @param deviceId Device ID
     * @param result Test result
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool selfTestDevice(const std::string& deviceId, Json::Value& result, 
                        int timeoutMs = 5000);
    
    /**
     * @brief Configure device
     * @param deviceId Device ID
     * @param config Configuration
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    bool configureDevice(const std::string& deviceId, const Json::Value& config,
                         int timeoutMs = 5000);
    
    /**
     * @brief Set event callback
     * @param callback Event callback
     */
    void setEventCallback(EventCallback callback) { eventCallback = callback; }
    
    /**
     * @brief Set command callback
     * @param callback Command callback
     */
    void setCommandCallback(CommandCallback callback) { commandCallback = callback; }
    
    /**
     * @brief Get statistics
     */
    Json::Value getStats() const;
    
    /**
     * @brief Reset statistics
     */
    void resetStats();
    
    /**
     * @brief Save device configuration
     * @param path File path
     * @return true on success
     */
    bool saveConfig(const std::string& path) const;
    
    /**
     * @brief Load device configuration
     * @param path File path
     * @return true on success
     */
    bool loadConfig(const std::string& path);

private:
    DeviceManager();
    ~DeviceManager();
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;
    
    std::map<std::string, std::unique_ptr<Device>> devices;
    std::map<std::string, DeviceInfo> deviceInfos;
    std::mutex devicesMutex;
    std::atomic<bool> running{false};
    std::atomic<bool> initialized{false};
    std::thread monitorThread;
    
    EventCallback eventCallback;
    CommandCallback commandCallback;
    
    // Statistics
    struct Stats {
        std::atomic<uint64_t> devicesAdded{0};
        std::atomic<uint64_t> devicesRemoved{0};
        std::atomic<uint64_t> devicesDiscovered{0};
        std::atomic<uint64_t> readErrors{0};
        std::atomic<uint64_t> writeErrors{0};
        std::atomic<uint64_t> commandErrors{0};
        std::chrono::steady_clock::time_point startTime;
    };
    Stats stats;
    mutable std::mutex statsMutex;
    
    // Configuration
    Json::Value config;
    std::string configPath;
    
    void monitorLoop();
    void checkDevices();
    void processDeviceEvent(const DeviceEvent& event);
    void processDeviceData(const DeviceSample& sample);
    void processCommandResult(const DeviceCommand& command, const DeviceSample& result);
    void emitEvent(const DeviceEvent& event);
    void updateStats(Device* device);
    std::string generateDeviceId();
    bool loadDevicesFromConfig(const Json::Value& config);
    Json::Value saveDevicesToConfig() const;
};

} // namespace device_manager

#endif // DEVICE_MANAGER_H
