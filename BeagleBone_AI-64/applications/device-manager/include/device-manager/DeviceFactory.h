#ifndef DEVICE_FACTORY_H
#define DEVICE_FACTORY_H

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <json/json.h>
#include "Device.h"
#include "DeviceTypes.h"

namespace device_manager {

/**
 * @brief Device factory for creating device instances
 */
class DeviceFactory {
public:
    using DeviceCreator = std::function<std::unique_ptr<Device>(const DeviceInfo&)>;
    
    /**
     * @brief Get singleton instance
     */
    static DeviceFactory& getInstance();
    
    /**
     * @brief Register a device type
     * @param type Device type
     * @param creator Function to create device
     */
    void registerDevice(DeviceType type, DeviceCreator creator);
    
    /**
     * @brief Create device from info
     * @param info Device information
     * @return Device instance or nullptr if unsupported
     */
    std::unique_ptr<Device> createDevice(const DeviceInfo& info);
    
    /**
     * @brief Create device from JSON
     * @param json JSON configuration
     * @return Device instance or nullptr
     */
    std::unique_ptr<Device> createDeviceFromJson(const Json::Value& json);
    
    /**
     * @brief Check if device type is supported
     * @param type Device type
     * @return true if supported
     */
    bool isSupported(DeviceType type) const;
    
    /**
     * @brief Get list of supported types
     */
    std::vector<DeviceType> getSupportedTypes() const;
    
    /**
     * @brief Get device type name
     */
    static std::string getTypeName(DeviceType type);
    
    /**
     * @brief Get device type from name
     */
    static DeviceType getTypeFromName(const std::string& name);
    
    /**
     * @brief Register built-in devices
     */
    void registerBuiltInDevices();

private:
    DeviceFactory() = default;
    ~DeviceFactory() = default;
    DeviceFactory(const DeviceFactory&) = delete;
    DeviceFactory& operator=(const DeviceFactory&) = delete;
    
    std::map<DeviceType, DeviceCreator> creators;
    static const std::map<DeviceType, std::string> typeNames;
    static const std::map<std::string, DeviceType> nameTypes;
    
    void registerSensorDevices();
    void registerActuatorDevices();
    void registerCommunicationDevices();
    void registerStorageDevices();
    void registerDisplayDevices();
    void registerInputDevices();
    void registerOutputDevices();
};

} // namespace device_manager

#endif // DEVICE_FACTORY_H
