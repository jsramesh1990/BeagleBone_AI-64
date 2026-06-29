#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "device-manager/DeviceFactory.h"
#include "device-manager/Device.h"

using namespace device_manager;

/**
 * @brief Test fixture for DeviceFactory tests
 */
class DeviceFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        factory = &DeviceFactory::getInstance();
        factory->registerBuiltInDevices();
    }
    
    DeviceFactory* factory;
};

TEST_F(DeviceFactoryTest, SingletonInstance) {
    auto& instance1 = DeviceFactory::getInstance();
    auto& instance2 = DeviceFactory::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(DeviceFactoryTest, RegisterDeviceType) {
    // Create a test device creator
    auto creator = [](const DeviceInfo& info) -> std::unique_ptr<Device> {
        return std::make_unique<MockDevice>(info);
    };
    
    DeviceType testType = DeviceType::SENSOR_HUMIDITY;
    factory->registerDevice(testType, creator);
    
    EXPECT_TRUE(factory->isSupported(testType));
    
    // Create device
    DeviceInfo info;
    info.id = "test-001";
    info.type = testType;
    auto device = factory->createDevice(info);
    EXPECT_NE(device, nullptr);
    EXPECT_EQ(device->getType(), testType);
}

TEST_F(DeviceFactoryTest, BuiltInDevicesRegistered) {
    // Check that all built-in device types are registered
    std::vector<DeviceType> types = {
        DeviceType::SENSOR_IMU,
        DeviceType::SENSOR_GPS,
        DeviceType::SENSOR_TEMPERATURE,
        DeviceType::SENSOR_PRESSURE,
        DeviceType::SENSOR_HUMIDITY,
        DeviceType::ACTUATOR_LED,
        DeviceType::ACTUATOR_MOTOR,
        DeviceType::COMMUNICATION_WIFI,
        DeviceType::COMMUNICATION_BLUETOOTH,
        DeviceType::STORAGE_SD_CARD,
        DeviceType::DISPLAY_LCD,
        DeviceType::INPUT_BUTTON,
        DeviceType::OUTPUT_AUDIO
    };
    
    for (auto type : types) {
        EXPECT_TRUE(factory->isSupported(type)) 
            << "Device type " << static_cast<int>(type) << " not supported";
    }
}

TEST_F(DeviceFactoryTest, CreateDeviceFromInfo) {
    DeviceInfo info;
    info.id = "temp-001";
    info.name = "Temperature Sensor";
    info.type = DeviceType::SENSOR_TEMPERATURE;
    info.manufacturer = "Test Corp";
    info.model = "TMP-100";
    info.address.type = ConnectionType::I2C;
    info.address.address = 0x48;
    
    auto device = factory->createDevice(info);
    EXPECT_NE(device, nullptr);
    EXPECT_EQ(device->getType(), DeviceType::SENSOR_TEMPERATURE);
    EXPECT_EQ(device->getName(), "Temperature Sensor");
    EXPECT_EQ(device->getConnectionType(), ConnectionType::I2C);
}

TEST_F(DeviceFactoryTest, CreateDeviceFromJson) {
    Json::Value json;
    json["id"] = "imu-001";
    json["name"] = "IMU Sensor";
    json["type"] = static_cast<int>(DeviceType::SENSOR_IMU);
    json["manufacturer"] = "Invensense";
    json["model"] = "MPU6050";
    json["address"]["type"] = static_cast<int>(ConnectionType::I2C);
    json["address"]["address"] = 0x68;
    json["address"]["bus"] = "i2c-0";
    
    auto device = factory->createDeviceFromJson(json);
    EXPECT_NE(device, nullptr);
    EXPECT_EQ(device->getType(), DeviceType::SENSOR_IMU);
    EXPECT_EQ(device->getName(), "IMU Sensor");
}

TEST_F(DeviceFactoryTest, UnsupportedDeviceType) {
    DeviceInfo info;
    info.id = "unknown-001";
    info.type = DeviceType::UNKNOWN;
    
    auto device = factory->createDevice(info);
    EXPECT_EQ(device, nullptr);
}

TEST_F(DeviceFactoryTest, GetSupportedTypes) {
    auto types = factory->getSupportedTypes();
    EXPECT_GT(types.size(), 0);
    
    // Check that specific types are in the list
    EXPECT_TRUE(std::find(types.begin(), types.end(), DeviceType::SENSOR_TEMPERATURE) != types.end());
    EXPECT_TRUE(std::find(types.begin(), types.end(), DeviceType::ACTUATOR_LED) != types.end());
}

TEST_F(DeviceFactoryTest, TypeNameConversions) {
    // Test all known types
    std::vector<DeviceType> types = {
        DeviceType::SENSOR_IMU,
        DeviceType::SENSOR_GPS,
        DeviceType::SENSOR_TEMPERATURE,
        DeviceType::SENSOR_PRESSURE,
        DeviceType::SENSOR_HUMIDITY,
        DeviceType::ACTUATOR_LED,
        DeviceType::ACTUATOR_MOTOR,
        DeviceType::COMMUNICATION_WIFI
    };
    
    for (auto type : types) {
        std::string name = DeviceFactory::getTypeName(type);
        EXPECT_NE(name, "Unknown");
        
        DeviceType converted = DeviceFactory::getTypeFromName(name);
        EXPECT_EQ(converted, type);
    }
}

TEST_F(DeviceFactoryTest, CustomDeviceRegistration) {
    struct CustomDevice : public Device {
        CustomDevice(const DeviceInfo& info) : Device(info) {}
        virtual bool initialize() override { return true; }
        virtual bool start() override { return true; }
        virtual bool stop() override { return true; }
        virtual bool reset() override { return true; }
        virtual bool readData(DeviceSample& data, int timeoutMs) override { return true; }
        virtual bool writeData(const Json::Value& data, int timeoutMs) override { return true; }
        virtual bool executeCommand(const DeviceCommand& cmd, DeviceSample& result, int timeoutMs) override { return true; }
        virtual bool calibrate(const Json::Value& params, Json::Value& result, int timeoutMs) override { return true; }
        virtual bool selfTest(Json::Value& result, int timeoutMs) override { return true; }
        virtual bool configure(const Json::Value& config, int timeoutMs) override { return true; }
    };
    
    // Register custom device
    factory->registerDevice(DeviceType::CUSTOM_EVENT, [](const DeviceInfo& info) {
        return std::make_unique<CustomDevice>(info);
    });
    
    EXPECT_TRUE(factory->isSupported(DeviceType::CUSTOM_EVENT));
    
    // Create custom device
    DeviceInfo info;
    info.id = "custom-001";
    info.type = DeviceType::CUSTOM_EVENT;
    info.name = "Custom Device";
    
    auto device = factory->createDevice(info);
    EXPECT_NE(device, nullptr);
    EXPECT_EQ(device->getType(), DeviceType::CUSTOM_EVENT);
}

TEST_F(DeviceFactoryTest, DeviceInfoSerialization) {
    DeviceInfo info;
    info.id = "test-001";
    info.name = "Test Device";
    info.description = "A test device";
    info.manufacturer = "TestCorp";
    info.model = "T-1000";
    info.version = "1.0.0";
    info.serialNumber = "SN123456";
    info.type = DeviceType::SENSOR_TEMPERATURE;
    info.status = DeviceStatus::OK;
    info.priority = 5;
    info.enabled = true;
    info.capabilities.canReadData = true;
    info.capabilities.canConfigure = true;
    info.properties["prop1"] = "value1";
    info.metadata["meta1"] = "data1";
    
    Json::Value json = info.toJson();
    DeviceInfo deserialized = DeviceInfo::fromJson(json);
    
    EXPECT_EQ(deserialized.id, info.id);
    EXPECT_EQ(deserialized.name, info.name);
    EXPECT_EQ(deserialized.description, info.description);
    EXPECT_EQ(deserialized.manufacturer, info.manufacturer);
    EXPECT_EQ(deserialized.model, info.model);
    EXPECT_EQ(deserialized.version, info.version);
    EXPECT_EQ(deserialized.serialNumber, info.serialNumber);
    EXPECT_EQ(deserialized.type, info.type);
    EXPECT_EQ(deserialized.status, info.status);
    EXPECT_EQ(deserialized.priority, info.priority);
    EXPECT_EQ(deserialized.enabled, info.enabled);
    EXPECT_EQ(deserialized.capabilities.canReadData, info.capabilities.canReadData);
    EXPECT_EQ(deserialized.capabilities.canConfigure, info.capabilities.canConfigure);
}

TEST_F(DeviceFactoryTest, MultipleDeviceRegistration) {
    // Register multiple custom devices
    for (int i = 0; i < 5; ++i) {
        DeviceType type = static_cast<DeviceType>(100 + i);
        factory->registerDevice(type, [i](const DeviceInfo& info) {
            // Create device with different behavior based on i
            auto device = std::make_unique<MockDevice>(info);
            device->setProperty("index", i);
            return device;
        });
        
        EXPECT_TRUE(factory->isSupported(type));
    }
    
    // Verify all registered devices work
    for (int i = 0; i < 5; ++i) {
        DeviceType type = static_cast<DeviceType>(100 + i);
        DeviceInfo info;
        info.id = "device-" + std::to_string(i);
        info.type = type;
        
        auto device = factory->createDevice(info);
        EXPECT_NE(device, nullptr);
        EXPECT_EQ(device->getType(), type);
        EXPECT_EQ(device->getProperty("index").asInt(), i);
    }
}

// Test device type name mapping completeness
TEST(DeviceTypeNameTest, AllTypesHaveNames) {
    std::vector<DeviceType> types = {
        DeviceType::SENSOR_IMU,
        DeviceType::SENSOR_GPS,
        DeviceType::SENSOR_TEMPERATURE,
        DeviceType::SENSOR_PRESSURE,
        DeviceType::SENSOR_HUMIDITY,
        DeviceType::SENSOR_MAGNETOMETER,
        DeviceType::SENSOR_GYROSCOPE,
        DeviceType::SENSOR_ACCELEROMETER,
        DeviceType::ACTUATOR_LED,
        DeviceType::ACTUATOR_MOTOR,
        DeviceType::ACTUATOR_RELAY,
        DeviceType::ACTUATOR_BUZZER,
        DeviceType::ACTUATOR_SERVO,
        DeviceType::COMMUNICATION_WIFI,
        DeviceType::COMMUNICATION_BLUETOOTH,
        DeviceType::COMMUNICATION_ETHERNET,
        DeviceType::COMMUNICATION_SERIAL,
        DeviceType::COMMUNICATION_CAN,
        DeviceType::COMMUNICATION_I2C,
        DeviceType::COMMUNICATION_SPI,
        DeviceType::STORAGE_SD_CARD,
        DeviceType::STORAGE_USB,
        DeviceType::STORAGE_EMMC,
        DeviceType::DISPLAY_LCD,
        DeviceType::DISPLAY_OLED,
        DeviceType::DISPLAY_HDMI,
        DeviceType::INPUT_BUTTON,
        DeviceType::INPUT_KEYPAD,
        DeviceType::INPUT_TOUCH,
        DeviceType::INPUT_MICROPHONE,
        DeviceType::OUTPUT_AUDIO,
        DeviceType::OUTPUT_SPEAKER
    };
    
    for (auto type : types) {
        std::string name = DeviceFactory::getTypeName(type);
        EXPECT_NE(name, "Unknown") << "Type " << static_cast<int>(type) << " has no name";
    }
}

#endif // DEVICE_FACTORY_TEST_CPP
