#include "device-manager/DeviceFactory.h"
#include <iostream>

using namespace device_manager;

// Static member initialization
const std::map<DeviceType, std::string> DeviceFactory::typeNames = {
    {DeviceType::SENSOR_IMU, "IMU"},
    {DeviceType::SENSOR_GPS, "GPS"},
    {DeviceType::SENSOR_TEMPERATURE, "Temperature"},
    {DeviceType::SENSOR_PRESSURE, "Pressure"},
    {DeviceType::SENSOR_HUMIDITY, "Humidity"},
    {DeviceType::SENSOR_MAGNETOMETER, "Magnetometer"},
    {DeviceType::SENSOR_GYROSCOPE, "Gyroscope"},
    {DeviceType::SENSOR_ACCELEROMETER, "Accelerometer"},
    {DeviceType::ACTUATOR_LED, "LED"},
    {DeviceType::ACTUATOR_MOTOR, "Motor"},
    {DeviceType::ACTUATOR_RELAY, "Relay"},
    {DeviceType::ACTUATOR_BUZZER, "Buzzer"},
    {DeviceType::ACTUATOR_SERVO, "Servo"},
    {DeviceType::COMMUNICATION_WIFI, "WiFi"},
    {DeviceType::COMMUNICATION_BLUETOOTH, "Bluetooth"},
    {DeviceType::COMMUNICATION_ETHERNET, "Ethernet"},
    {DeviceType::COMMUNICATION_SERIAL, "Serial"},
    {DeviceType::COMMUNICATION_CAN, "CAN"},
    {DeviceType::COMMUNICATION_I2C, "I2C"},
    {DeviceType::COMMUNICATION_SPI, "SPI"},
    {DeviceType::STORAGE_SD_CARD, "SDCard"},
    {DeviceType::STORAGE_USB, "USB"},
    {DeviceType::STORAGE_EMMC, "eMMC"},
    {DeviceType::DISPLAY_LCD, "LCD"},
    {DeviceType::DISPLAY_OLED, "OLED"},
    {DeviceType::DISPLAY_HDMI, "HDMI"},
    {DeviceType::INPUT_BUTTON, "Button"},
    {DeviceType::INPUT_KEYPAD, "Keypad"},
    {DeviceType::INPUT_TOUCH, "Touch"},
    {DeviceType::INPUT_MICROPHONE, "Microphone"},
    {DeviceType::OUTPUT_AUDIO, "Audio"},
    {DeviceType::OUTPUT_SPEAKER, "Speaker"},
    {DeviceType::UNKNOWN, "Unknown"}
};

const std::map<std::string, DeviceType> DeviceFactory::nameTypes = []() {
    std::map<std::string, DeviceType> result;
    for (const auto& [type, name] : typeNames) {
        result[name] = type;
    }
    return result;
}();

DeviceFactory& DeviceFactory::getInstance() {
    static DeviceFactory instance;
    return instance;
}

void DeviceFactory::registerDevice(DeviceType type, DeviceCreator creator) {
    creators[type] = creator;
}

std::unique_ptr<Device> DeviceFactory::createDevice(const DeviceInfo& info) {
    auto it = creators.find(info.type);
    if (it != creators.end()) {
        return it->second(info);
    }
    return nullptr;
}

std::unique_ptr<Device> DeviceFactory::createDeviceFromJson(const Json::Value& json) {
    DeviceInfo info = DeviceInfo::fromJson(json);
    return createDevice(info);
}

bool DeviceFactory::isSupported(DeviceType type) const {
    return creators.find(type) != creators.end();
}

std::vector<DeviceType> DeviceFactory::getSupportedTypes() const {
    std::vector<DeviceType> types;
    for (const auto& [type, _] : creators) {
        types.push_back(type);
    }
    return types;
}

std::string DeviceFactory::getTypeName(DeviceType type) {
    auto it = typeNames.find(type);
    if (it != typeNames.end()) {
        return it->second;
    }
    return "Unknown";
}

DeviceType DeviceFactory::getTypeFromName(const std::string& name) {
    auto it = nameTypes.find(name);
    if (it != nameTypes.end()) {
        return it->second;
    }
    return DeviceType::UNKNOWN;
}

void DeviceFactory::registerBuiltInDevices() {
    registerSensorDevices();
    registerActuatorDevices();
    registerCommunicationDevices();
    registerStorageDevices();
    registerDisplayDevices();
    registerInputDevices();
    registerOutputDevices();
}

void DeviceFactory::registerSensorDevices() {
    // Register sensor devices
    // These would be implemented in separate files
    // For now, we register placeholder creators
    
    registerDevice(DeviceType::SENSOR_IMU, [](const DeviceInfo& info) {
        // return std::make_unique<IMUSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_GPS, [](const DeviceInfo& info) {
        // return std::make_unique<GPSSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_TEMPERATURE, [](const DeviceInfo& info) {
        // return std::make_unique<TemperatureSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_PRESSURE, [](const DeviceInfo& info) {
        // return std::make_unique<PressureSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_HUMIDITY, [](const DeviceInfo& info) {
        // return std::make_unique<HumiditySensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_MAGNETOMETER, [](const DeviceInfo& info) {
        // return std::make_unique<MagnetometerSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_GYROSCOPE, [](const DeviceInfo& info) {
        // return std::make_unique<GyroscopeSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::SENSOR_ACCELEROMETER, [](const DeviceInfo& info) {
        // return std::make_unique<AccelerometerSensor>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}

void DeviceFactory::registerActuatorDevices() {
    registerDevice(DeviceType::ACTUATOR_LED, [](const DeviceInfo& info) {
        // return std::make_unique<LEDDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::ACTUATOR_MOTOR, [](const DeviceInfo& info) {
        // return std::make_unique<MotorDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::ACTUATOR_RELAY, [](const DeviceInfo& info) {
        // return std::make_unique<RelayDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::ACTUATOR_BUZZER, [](const DeviceInfo& info) {
        // return std::make_unique<BuzzerDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::ACTUATOR_SERVO, [](const DeviceInfo& info) {
        // return std::make_unique<ServoDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}

void DeviceFactory::registerCommunicationDevices() {
    registerDevice(DeviceType::COMMUNICATION_WIFI, [](const DeviceInfo& info) {
        // return std::make_unique<WiFiDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::COMMUNICATION_BLUETOOTH, [](const DeviceInfo& info) {
        // return std::make_unique<BluetoothDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::COMMUNICATION_ETHERNET, [](const DeviceInfo& info) {
        // return std::make_unique<EthernetDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::COMMUNICATION_SERIAL, [](const DeviceInfo& info) {
        // return std::make_unique<SerialDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}

void DeviceFactory::registerStorageDevices() {
    registerDevice(DeviceType::STORAGE_SD_CARD, [](const DeviceInfo& info) {
        // return std::make_unique<SDCardDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::STORAGE_USB, [](const DeviceInfo& info) {
        // return std::make_unique<USBDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::STORAGE_EMMC, [](const DeviceInfo& info) {
        // return std::make_unique<EMMCDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}

void DeviceFactory::registerDisplayDevices() {
    registerDevice(DeviceType::DISPLAY_LCD, [](const DeviceInfo& info) {
        // return std::make_unique<LCDDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::DISPLAY_OLED, [](const DeviceInfo& info) {
        // return std::make_unique<OLEDDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::DISPLAY_HDMI, [](const DeviceInfo& info) {
        // return std::make_unique<HDMIDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}

void DeviceFactory::registerInputDevices() {
    registerDevice(DeviceType::INPUT_BUTTON, [](const DeviceInfo& info) {
        // return std::make_unique<ButtonDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::INPUT_KEYPAD, [](const DeviceInfo& info) {
        // return std::make_unique<KeypadDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::INPUT_TOUCH, [](const DeviceInfo& info) {
        // return std::make_unique<TouchDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::INPUT_MICROPHONE, [](const DeviceInfo& info) {
        // return std::make_unique<MicrophoneDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}

void DeviceFactory::registerOutputDevices() {
    registerDevice(DeviceType::OUTPUT_AUDIO, [](const DeviceInfo& info) {
        // return std::make_unique<AudioDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
    
    registerDevice(DeviceType::OUTPUT_SPEAKER, [](const DeviceInfo& info) {
        // return std::make_unique<SpeakerDevice>(info);
        return std::unique_ptr<Device>(nullptr);
    });
}
