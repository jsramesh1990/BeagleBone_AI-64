#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <json/json.h>
#include "DeviceTypes.h"

namespace device_manager {

/**
 * @brief Base class for all devices
 */
class Device {
public:
    using DataCallback = std::function<void(const DeviceSample&)>;
    using EventCallback = std::function<void(const DeviceEvent&)>;
    using CommandCallback = std::function<void(const DeviceCommand&, const DeviceSample&)>;
    
    /**
     * @brief Constructor
     */
    Device(const DeviceInfo& info);
    
    /**
     * @brief Destructor
     */
    virtual ~Device();
    
    /**
     * @brief Initialize the device
     * @return true on success
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Start the device
     * @return true on success
     */
    virtual bool start() = 0;
    
    /**
     * @brief Stop the device
     * @return true on success
     */
    virtual bool stop() = 0;
    
    /**
     * @brief Reset the device
     * @return true on success
     */
    virtual bool reset() = 0;
    
    /**
     * @brief Read data from device
     * @param data Data sample to fill
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    virtual bool readData(DeviceSample& data, int timeoutMs = 5000) = 0;
    
    /**
     * @brief Write data to device
     * @param data Data to write
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    virtual bool writeData(const Json::Value& data, int timeoutMs = 5000) = 0;
    
    /**
     * @brief Execute command on device
     * @param command Command to execute
     * @param result Result of command
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    virtual bool executeCommand(const DeviceCommand& command, DeviceSample& result, 
                                int timeoutMs = 5000) = 0;
    
    /**
     * @brief Calibrate the device
     * @param params Calibration parameters
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    virtual bool calibrate(const Json::Value& params, Json::Value& result, 
                           int timeoutMs = 5000) = 0;
    
    /**
     * @brief Run self-test
     * @param result Test results
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    virtual bool selfTest(Json::Value& result, int timeoutMs = 5000) = 0;
    
    /**
     * @brief Configure device
     * @param config Configuration parameters
     * @param timeoutMs Timeout in milliseconds
     * @return true on success
     */
    virtual bool configure(const Json::Value& config, int timeoutMs = 5000) = 0;
    
    /**
     * @brief Get device information
     */
    virtual DeviceInfo getInfo() const { return info; }
    
    /**
     * @brief Get device status
     */
    virtual DeviceStatus getStatus() const { return status; }
    
    /**
     * @brief Get device capabilities
     */
    virtual DeviceCapabilities getCapabilities() const { return info.capabilities; }
    
    /**
     * @brief Check if device is ready
     */
    virtual bool isReady() const { return status == DeviceStatus::OK; }
    
    /**
     * @brief Set data callback
     * @param callback Callback function
     */
    virtual void setDataCallback(DataCallback callback) { dataCallback = callback; }
    
    /**
     * @brief Set event callback
     * @param callback Callback function
     */
    virtual void setEventCallback(EventCallback callback) { eventCallback = callback; }
    
    /**
     * @brief Set command callback
     * @param callback Callback function
     */
    virtual void setCommandCallback(CommandCallback callback) { commandCallback = callback; }
    
    /**
     * @brief Get device ID
     */
    virtual std::string getId() const { return info.id; }
    
    /**
     * @brief Get device name
     */
    virtual std::string getName() const { return info.name; }
    
    /**
     * @brief Get device type
     */
    virtual DeviceType getType() const { return info.type; }
    
    /**
     * @brief Get connection type
     */
    virtual ConnectionType getConnectionType() const { return info.address.type; }
    
    /**
     * @brief Get device address
     */
    virtual DeviceAddress getAddress() const { return info.address; }
    
    /**
     * @brief Get property
     * @param key Property key
     * @return Property value (null if not found)
     */
    virtual Json::Value getProperty(const std::string& key) const;
    
    /**
     * @brief Set property
     * @param key Property key
     * @param value Property value
     */
    virtual void setProperty(const std::string& key, const Json::Value& value);
    
    /**
     * @brief Get metadata
     * @param key Metadata key
     * @return Metadata value (null if not found)
     */
    virtual Json::Value getMetadata(const std::string& key) const;
    
    /**
     * @brief Set metadata
     * @param key Metadata key
     * @param value Metadata value
     */
    virtual void setMetadata(const std::string& key, const Json::Value& value);

protected:
    DeviceInfo info;
    DeviceStatus status;
    std::chrono::steady_clock::time_point lastActivity;
    DataCallback dataCallback;
    EventCallback eventCallback;
    CommandCallback commandCallback;
    mutable std::mutex mutex;
    std::atomic<bool> isRunning;
    
    /**
     * @brief Update device status
     * @param newStatus New status
     */
    virtual void setStatus(DeviceStatus newStatus);
    
    /**
     * @brief Emit event
     * @param event Event to emit
     */
    virtual void emitEvent(const DeviceEvent& event);
    
    /**
     * @brief Emit data
     * @param sample Data sample
     */
    virtual void emitData(const DeviceSample& sample);
    
    /**
     * @brief Emit command result
     * @param command Command that was executed
     * @param result Command result
     */
    virtual void emitCommandResult(const DeviceCommand& command, const DeviceSample& result);
    
    /**
     * @brief Get current timestamp
     */
    static std::chrono::system_clock::time_point getCurrentTime();
    
    /**
     * @brief Validate device configuration
     * @param config Configuration to validate
     * @return true if valid
     */
    virtual bool validateConfig(const Json::Value& config) const;
    
    /**
     * @brief Wait for device ready
     * @param timeoutMs Timeout in milliseconds
     * @return true if ready
     */
    virtual bool waitForReady(int timeoutMs = 5000);
};

} // namespace device_manager

#endif // DEVICE_H
