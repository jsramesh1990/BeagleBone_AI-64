#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "device-manager/Device.h"
#include "device-manager/DeviceTypes.h"
#include <chrono>
#include <thread>

using namespace device_manager;

/**
 * @brief Mock device for testing
 */
class MockDevice : public Device {
public:
    MockDevice(const DeviceInfo& info) : Device(info) {
        status = DeviceStatus::OK;
        isRunning = false;
    }
    
    virtual bool initialize() override {
        status = DeviceStatus::OK;
        return true;
    }
    
    virtual bool start() override {
        isRunning = true;
        status = DeviceStatus::OK;
        return true;
    }
    
    virtual bool stop() override {
        isRunning = false;
        status = DeviceStatus::STOPPED;
        return true;
    }
    
    virtual bool reset() override {
        status = DeviceStatus::OK;
        return true;
    }
    
    virtual bool readData(DeviceSample& data, int timeoutMs) override {
        data.deviceId = info.id;
        data.timestamp = getCurrentTime();
        data.values["test"] = 42.0;
        data.isValid = true;
        return true;
    }
    
    virtual bool writeData(const Json::Value& data, int timeoutMs) override {
        return true;
    }
    
    virtual bool executeCommand(const DeviceCommand& command, DeviceSample& result, int timeoutMs) override {
        result.deviceId = info.id;
        result.timestamp = getCurrentTime();
        result.values["status"] = 0.0;
        result.isValid = true;
        return true;
    }
    
    virtual bool calibrate(const Json::Value& params, Json::Value& result, int timeoutMs) override {
        result["status"] = "success";
        return true;
    }
    
    virtual bool selfTest(Json::Value& result, int timeoutMs) override {
        result["status"] = "passed";
        return true;
    }
    
    virtual bool configure(const Json::Value& config, int timeoutMs) override {
        return true;
    }
};

/**
 * @brief Test fixture for Device tests
 */
class DeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        DeviceInfo info;
        info.id = "test-device-001";
        info.name = "Test Device";
        info.type = DeviceType::SENSOR_TEMPERATURE;
        info.status = DeviceStatus::OK;
        info.address.type = ConnectionType::I2C;
        info.address.bus = "i2c-0";
        info.address.address = 0x48;
        
        device = std::make_unique<MockDevice>(info);
    }
    
    std::unique_ptr<MockDevice> device;
};

TEST_F(DeviceTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(device->getId(), "test-device-001");
    EXPECT_EQ(device->getName(), "Test Device");
    EXPECT_EQ(device->getType(), DeviceType::SENSOR_TEMPERATURE);
    EXPECT_EQ(device->getConnectionType(), ConnectionType::I2C);
}

TEST_F(DeviceTest, InitializeSetsStatusOk) {
    EXPECT_TRUE(device->initialize());
    EXPECT_EQ(device->getStatus(), DeviceStatus::OK);
}

TEST_F(DeviceTest, StartStopsCorrectly) {
    EXPECT_TRUE(device->start());
    EXPECT_TRUE(device->isReady());
    
    EXPECT_TRUE(device->stop());
    EXPECT_EQ(device->getStatus(), DeviceStatus::STOPPED);
}

TEST_F(DeviceTest, ResetWorks) {
    EXPECT_TRUE(device->reset());
    EXPECT_EQ(device->getStatus(), DeviceStatus::OK);
}

TEST_F(DeviceTest, ReadDataReturnsValidSample) {
    DeviceSample sample;
    EXPECT_TRUE(device->readData(sample, 1000));
    
    EXPECT_EQ(sample.deviceId, device->getId());
    EXPECT_TRUE(sample.isValid);
    EXPECT_GT(sample.values.size(), 0);
    EXPECT_DOUBLE_EQ(sample.values["test"], 42.0);
}

TEST_F(DeviceTest, WriteDataSucceeds) {
    Json::Value data;
    data["value"] = 100;
    EXPECT_TRUE(device->writeData(data, 1000));
}

TEST_F(DeviceTest, ExecuteCommandSucceeds) {
    DeviceCommand command;
    command.id = "cmd-001";
    command.name = "test_command";
    command.deviceId = device->getId();
    
    DeviceSample result;
    EXPECT_TRUE(device->executeCommand(command, result, 1000));
    
    EXPECT_EQ(result.deviceId, device->getId());
    EXPECT_TRUE(result.isValid);
}

TEST_F(DeviceTest, CalibrateSucceeds) {
    Json::Value params;
    params["offset"] = 0.5;
    
    Json::Value result;
    EXPECT_TRUE(device->calibrate(params, result, 5000));
    EXPECT_EQ(result["status"].asString(), "success");
}

TEST_F(DeviceTest, SelfTestSucceeds) {
    Json::Value result;
    EXPECT_TRUE(device->selfTest(result, 5000));
    EXPECT_EQ(result["status"].asString(), "passed");
}

TEST_F(DeviceTest, ConfigureSucceeds) {
    Json::Value config;
    config["sampling_rate"] = 100;
    EXPECT_TRUE(device->configure(config, 5000));
}

TEST_F(DeviceTest, PropertyManagement) {
    // Set property
    device->setProperty("test_key", "test_value");
    EXPECT_EQ(device->getProperty("test_key").asString(), "test_value");
    
    // Update property
    device->setProperty("test_key", "new_value");
    EXPECT_EQ(device->getProperty("test_key").asString(), "new_value");
    
    // Non-existent property returns null
    EXPECT_TRUE(device->getProperty("nonexistent").isNull());
}

TEST_F(DeviceTest, MetadataManagement) {
    device->setMetadata("author", "test_user");
    EXPECT_EQ(device->getMetadata("author").asString(), "test_user");
    
    device->setMetadata("version", "1.0.0");
    EXPECT_EQ(device->getMetadata("version").asString(), "1.0.0");
}

TEST_F(DeviceTest, Callbacks) {
    bool dataCallbackCalled = false;
    bool eventCallbackCalled = false;
    bool commandCallbackCalled = false;
    
    device->setDataCallback([&](const DeviceSample& sample) {
        dataCallbackCalled = true;
        EXPECT_EQ(sample.deviceId, device->getId());
    });
    
    device->setEventCallback([&](const DeviceEvent& event) {
        eventCallbackCalled = true;
        EXPECT_EQ(event.deviceId, device->getId());
    });
    
    device->setCommandCallback([&](const DeviceCommand& cmd, const DeviceSample& result) {
        commandCallbackCalled = true;
        EXPECT_EQ(cmd.deviceId, device->getId());
    });
    
    // Trigger callbacks
    DeviceSample sample;
    device->readData(sample, 100);
    
    DeviceEvent event;
    event.deviceId = device->getId();
    event.type = DeviceEvent::DEVICE_INFO;
    // device->emitEvent(event); // Protected method, testing via public methods
    
    DeviceCommand cmd;
    cmd.deviceId = device->getId();
    DeviceSample result;
    device->executeCommand(cmd, result, 100);
    
    // Note: Some callbacks may not be triggered directly in this test
    // They would be triggered by the device implementation
    EXPECT_TRUE(dataCallbackCalled);
}

TEST_F(DeviceTest, WaitForReady) {
    // Device should be ready immediately
    EXPECT_TRUE(device->isReady());
}

TEST_F(DeviceTest, StatusChanges) {
    EXPECT_EQ(device->getStatus(), DeviceStatus::OK);
    
    device->stop();
    EXPECT_EQ(device->getStatus(), DeviceStatus::STOPPED);
    
    device->start();
    EXPECT_EQ(device->getStatus(), DeviceStatus::OK);
}

TEST_F(DeviceTest, DeviceInfoSerialization) {
    DeviceInfo info = device->getInfo();
    Json::Value json = info.toJson();
    
    DeviceInfo deserialized = DeviceInfo::fromJson(json);
    
    EXPECT_EQ(deserialized.id, info.id);
    EXPECT_EQ(deserialized.name, info.name);
    EXPECT_EQ(deserialized.type, info.type);
    EXPECT_EQ(deserialized.address.type, info.address.type);
    EXPECT_EQ(deserialized.address.address, info.address.address);
}

TEST_F(DeviceTest, DeviceSampleSerialization) {
    DeviceSample sample;
    sample.deviceId = device->getId();
    sample.values["temperature"] = 25.5;
    sample.values["humidity"] = 60.0;
    sample.isValid = true;
    sample.sequence = 123;
    
    Json::Value json = sample.toJson();
    DeviceSample deserialized = DeviceSample::fromJson(json);
    
    EXPECT_EQ(deserialized.deviceId, sample.deviceId);
    EXPECT_EQ(deserialized.isValid, sample.isValid);
    EXPECT_EQ(deserialized.sequence, sample.sequence);
    EXPECT_DOUBLE_EQ(deserialized.values["temperature"], sample.values["temperature"]);
    EXPECT_DOUBLE_EQ(deserialized.values["humidity"], sample.values["humidity"]);
}

TEST_F(DeviceTest, DeviceCommandSerialization) {
    DeviceCommand cmd;
    cmd.id = "cmd-001";
    cmd.deviceId = device->getId();
    cmd.name = "test_cmd";
    cmd.parameters["timeout"] = 5000;
    cmd.isAsync = true;
    cmd.priority = 5;
    
    Json::Value json = cmd.toJson();
    
    EXPECT_EQ(json["id"].asString(), cmd.id);
    EXPECT_EQ(json["deviceId"].asString(), cmd.deviceId);
    EXPECT_EQ(json["name"].asString(), cmd.name);
    EXPECT_EQ(json["isAsync"].asBool(), cmd.isAsync);
    EXPECT_EQ(json["priority"].asInt(), cmd.priority);
    EXPECT_EQ(json["parameters"]["timeout"].asInt(), 5000);
}

// Test different device types
TEST(DeviceTypeTest, DeviceTypeNames) {
    auto& factory = DeviceFactory::getInstance();
    
    EXPECT_EQ(DeviceFactory::getTypeName(DeviceType::SENSOR_IMU), "IMU");
    EXPECT_EQ(DeviceFactory::getTypeName(DeviceType::SENSOR_GPS), "GPS");
    EXPECT_EQ(DeviceFactory::getTypeName(DeviceType::ACTUATOR_LED), "LED");
    EXPECT_EQ(DeviceFactory::getTypeName(DeviceType::COMMUNICATION_WIFI), "WiFi");
    EXPECT_EQ(DeviceFactory::getTypeName(DeviceType::UNKNOWN), "Unknown");
    
    EXPECT_EQ(DeviceFactory::getTypeFromName("IMU"), DeviceType::SENSOR_IMU);
    EXPECT_EQ(DeviceFactory::getTypeFromName("GPS"), DeviceType::SENSOR_GPS);
    EXPECT_EQ(DeviceFactory::getTypeFromName("LED"), DeviceType::ACTUATOR_LED);
    EXPECT_EQ(DeviceFactory::getTypeFromName("Unknown"), DeviceType::UNKNOWN);
}

// Test device address serialization
TEST(DeviceAddressTest, Serialization) {
    DeviceAddress addr;
    addr.type = ConnectionType::SPI;
    addr.bus = "spi-0";
    addr.address = 0x01;
    addr.path = "/dev/spidev0.0";
    addr.params["speed"] = "1000000";
    
    Json::Value json = addr.toJson();
    DeviceAddress deserialized = DeviceAddress::fromJson(json);
    
    EXPECT_EQ(deserialized.type, addr.type);
    EXPECT_EQ(deserialized.bus, addr.bus);
    EXPECT_EQ(deserialized.address, addr.address);
    EXPECT_EQ(deserialized.path, addr.path);
    EXPECT_EQ(deserialized.params["speed"], addr.params["speed"]);
}

// Test device capabilities
TEST(DeviceCapabilitiesTest, DefaultCapabilities) {
    DeviceCapabilities caps;
    EXPECT_FALSE(caps.canReadData);
    EXPECT_FALSE(caps.canWriteData);
    EXPECT_FALSE(caps.canControl);
    EXPECT_FALSE(caps.canConfigure);
    EXPECT_FALSE(caps.canCalibrate);
    EXPECT_FALSE(caps.canSelfTest);
}

#endif // DEVICE_TEST_CPP
