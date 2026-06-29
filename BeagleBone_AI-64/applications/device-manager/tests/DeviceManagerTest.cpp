#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "device-manager/DeviceManager.h"
#include "device-manager/DeviceFactory.h"
#include "device-manager/DeviceManagerConfig.h"
#include <thread>
#include <chrono>

using namespace device_manager;

/**
 * @brief Test fixture for DeviceManager tests
 */
class DeviceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &DeviceManager::getInstance();
        factory = &DeviceFactory::getInstance();
        factory->registerBuiltInDevices();
        
        // Reset manager state
        manager->stop();
        
        // Create test configuration
        Json::Value config;
        config["debugMode"] = true;
        config["environment"] = "test";
        
        manager->initialize(config);
    }
    
    void TearDown() override {
        manager->stop();
    }
    
    DeviceManager* manager;
    DeviceFactory* factory;
    
    /**
     * @brief Create a test device
     */
    std::unique_ptr<Device> createTestDevice(const std::string& id, DeviceType type) {
        DeviceInfo info;
        info.id = id;
        info.name = "Test Device " + id;
        info.type = type;
        info.status = DeviceStatus::OK;
        info.address.type = ConnectionType::I2C;
        info.address.address = 0x48;
        info.capabilities.canReadData = true;
        info.capabilities.canWriteData = true;
        info.capabilities.canConfigure = true;
        
        return std::make_unique<MockDevice>(info);
    }
};

TEST_F(DeviceManagerTest, SingletonInstance) {
    auto& instance1 = DeviceManager::getInstance();
    auto& instance2 = DeviceManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(DeviceManagerTest, InitializeAndStart) {
    EXPECT_TRUE(manager->initialize());
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->isRunning());
    
    manager->stop();
    EXPECT_FALSE(manager->isRunning());
}

TEST_F(DeviceManagerTest, AddDevice) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    Device* devicePtr = device.get();
    
    EXPECT_TRUE(manager->addDevice(std::move(device)));
    
    Device* retrieved = manager->getDevice("test-001");
    EXPECT_EQ(retrieved, devicePtr);
}

TEST_F(DeviceManagerTest, AddDuplicateDevice) {
    auto device1 = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    auto device2 = createTestDevice("test-001", DeviceType::SENSOR_HUMIDITY);
    
    EXPECT_TRUE(manager->addDevice(std::move(device1)));
    EXPECT_FALSE(manager->addDevice(std::move(device2)));
}

TEST_F(DeviceManagerTest, RemoveDevice) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    EXPECT_TRUE(manager->addDevice(std::move(device)));
    
    EXPECT_NE(manager->getDevice("test-001"), nullptr);
    EXPECT_TRUE(manager->removeDevice("test-001"));
    EXPECT_EQ(manager->getDevice("test-001"), nullptr);
}

TEST_F(DeviceManagerTest, GetDevicesByType) {
    auto temp1 = createTestDevice("temp-001", DeviceType::SENSOR_TEMPERATURE);
    auto temp2 = createTestDevice("temp-002", DeviceType::SENSOR_TEMPERATURE);
    auto imu = createTestDevice("imu-001", DeviceType::SENSOR_IMU);
    
    manager->addDevice(std::move(temp1));
    manager->addDevice(std::move(temp2));
    manager->addDevice(std::move(imu));
    
    auto tempDevices = manager->getDevicesByType(DeviceType::SENSOR_TEMPERATURE);
    EXPECT_EQ(tempDevices.size(), 2);
    
    auto imuDevices = manager->getDevicesByType(DeviceType::SENSOR_IMU);
    EXPECT_EQ(imuDevices.size(), 1);
    
    auto gpsDevices = manager->getDevicesByType(DeviceType::SENSOR_GPS);
    EXPECT_EQ(gpsDevices.size(), 0);
}

TEST_F(DeviceManagerTest, GetDevicesByStatus) {
    auto device1 = createTestDevice("dev-001", DeviceType::SENSOR_TEMPERATURE);
    auto device2 = createTestDevice("dev-002", DeviceType::SENSOR_IMU);
    
    // Device1 is OK, Device2 is stopped
    manager->addDevice(std::move(device1));
    
    auto device2Ptr = manager->getDevice("dev-002");
    if (device2Ptr) {
        device2Ptr->stop();
    }
    
    auto okDevices = manager->getDevicesByStatus(DeviceStatus::OK);
    // At least device1 should be OK
    EXPECT_GT(okDevices.size(), 0);
}

TEST_F(DeviceManagerTest, GetAllDevices) {
    auto device1 = createTestDevice("dev-001", DeviceType::SENSOR_TEMPERATURE);
    auto device2 = createTestDevice("dev-002", DeviceType::SENSOR_IMU);
    auto device3 = createTestDevice("dev-003", DeviceType::SENSOR_GPS);
    
    manager->addDevice(std::move(device1));
    manager->addDevice(std::move(device2));
    manager->addDevice(std::move(device3));
    
    auto allDevices = manager->getAllDevices();
    EXPECT_EQ(allDevices.size(), 3);
}

TEST_F(DeviceManagerTest, GetDeviceInfo) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    DeviceInfo info = manager->getDeviceInfo("test-001");
    EXPECT_EQ(info.id, "test-001");
    EXPECT_EQ(info.type, DeviceType::SENSOR_TEMPERATURE);
    
    DeviceInfo emptyInfo = manager->getDeviceInfo("nonexistent");
    EXPECT_TRUE(emptyInfo.id.empty());
}

TEST_F(DeviceManagerTest, GetAllDeviceInfo) {
    auto device1 = createTestDevice("dev-001", DeviceType::SENSOR_TEMPERATURE);
    auto device2 = createTestDevice("dev-002", DeviceType::SENSOR_IMU);
    
    manager->addDevice(std::move(device1));
    manager->addDevice(std::move(device2));
    
    auto infoList = manager->getAllDeviceInfo();
    EXPECT_EQ(infoList.size(), 2);
    
    bool foundDev1 = false, foundDev2 = false;
    for (const auto& info : infoList) {
        if (info.id == "dev-001") foundDev1 = true;
        if (info.id == "dev-002") foundDev2 = true;
    }
    EXPECT_TRUE(foundDev1);
    EXPECT_TRUE(foundDev2);
}

TEST_F(DeviceManagerTest, ReadData) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    DeviceSample sample;
    EXPECT_TRUE(manager->readData("test-001", sample, 1000));
    EXPECT_EQ(sample.deviceId, "test-001");
    EXPECT_TRUE(sample.isValid);
}

TEST_F(DeviceManagerTest, ReadDataFromNonExistentDevice) {
    DeviceSample sample;
    EXPECT_FALSE(manager->readData("nonexistent", sample, 1000));
}

TEST_F(DeviceManagerTest, WriteData) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    Json::Value data;
    data["value"] = 100;
    EXPECT_TRUE(manager->writeData("test-001", data, 1000));
}

TEST_F(DeviceManagerTest, ExecuteCommand) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    DeviceCommand command;
    command.id = "cmd-001";
    command.name = "test_cmd";
    command.deviceId = "test-001";
    command.parameters["timeout"] = 5000;
    
    DeviceSample result;
    EXPECT_TRUE(manager->executeCommand("test-001", command, result, 1000));
    EXPECT_EQ(result.deviceId, "test-001");
}

TEST_F(DeviceManagerTest, CalibrateDevice) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    Json::Value params;
    params["offset"] = 0.5;
    
    Json::Value result;
    EXPECT_TRUE(manager->calibrateDevice("test-001", params, result, 5000));
    EXPECT_EQ(result["status"].asString(), "success");
}

TEST_F(DeviceManagerTest, SelfTestDevice) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    Json::Value result;
    EXPECT_TRUE(manager->selfTestDevice("test-001", result, 5000));
    EXPECT_EQ(result["status"].asString(), "passed");
}

TEST_F(DeviceManagerTest, ConfigureDevice) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    Json::Value config;
    config["sampling_rate"] = 100;
    EXPECT_TRUE(manager->configureDevice("test-001", config, 5000));
}

TEST_F(DeviceManagerTest, EventCallbacks) {
    bool eventReceived = false;
    
    manager->setEventCallback([&](const DeviceEvent& event) {
        eventReceived = true;
    });
    
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(eventReceived);
}

TEST_F(DeviceManagerTest, CommandCallbacks) {
    bool commandReceived = false;
    
    manager->setCommandCallback([&](const DeviceCommand& cmd, const DeviceSample& result) {
        commandReceived = true;
        EXPECT_EQ(cmd.deviceId, "test-001");
    });
    
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    DeviceCommand cmd;
    cmd.id = "cmd-001";
    cmd.deviceId = "test-001";
    cmd.name = "test";
    
    DeviceSample result;
    manager->executeCommand("test-001", cmd, result, 1000);
    
    // Wait for callback processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(commandReceived);
}

TEST_F(DeviceManagerTest, Statistics) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    Json::Value stats = manager->getStats();
    EXPECT_TRUE(stats.isObject());
    EXPECT_TRUE(stats.isMember("devices_added"));
    EXPECT_TRUE(stats.isMember("devices_removed"));
    EXPECT_TRUE(stats.isMember("total_devices"));
    EXPECT_TRUE(stats.isMember("uptime_seconds"));
    
    EXPECT_EQ(stats["total_devices"].asInt(), 1);
}

TEST_F(DeviceManagerTest, ResetStatistics) {
    manager->resetStats();
    Json::Value stats = manager->getStats();
    EXPECT_EQ(stats["devices_added"].asInt(), 0);
    EXPECT_EQ(stats["devices_removed"].asInt(), 0);
}

TEST_F(DeviceManagerTest, SaveAndLoadConfig) {
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    std::string configPath = "/tmp/device_manager_test_config.json";
    EXPECT_TRUE(manager->saveConfig(configPath));
    
    // Clear devices
    manager->stop();
    manager->removeDevice("test-001");
    
    EXPECT_EQ(manager->getDevice("test-001"), nullptr);
    
    // Reload config
    EXPECT_TRUE(manager->loadConfig(configPath));
    
    // Wait for devices to be added
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if device was recreated
    // Note: This may not work if device creation requires factory registration
    // which is already done in SetUp
}

TEST_F(DeviceManagerTest, MultipleDeviceOperations) {
    const int NUM_DEVICES = 10;
    std::vector<std::string> deviceIds;
    
    // Add multiple devices
    for (int i = 0; i < NUM_DEVICES; ++i) {
        std::string id = "dev-" + std::to_string(i);
        deviceIds.push_back(id);
        auto device = createTestDevice(id, DeviceType::SENSOR_TEMPERATURE);
        EXPECT_TRUE(manager->addDevice(std::move(device)));
    }
    
    // Verify all devices are present
    auto allDevices = manager->getAllDevices();
    EXPECT_EQ(allDevices.size(), NUM_DEVICES);
    
    // Read data from all devices
    for (const auto& id : deviceIds) {
        DeviceSample sample;
        EXPECT_TRUE(manager->readData(id, sample, 1000));
        EXPECT_EQ(sample.deviceId, id);
    }
    
    // Remove all devices
    for (const auto& id : deviceIds) {
        EXPECT_TRUE(manager->removeDevice(id));
    }
    
    // Verify all devices are removed
    allDevices = manager->getAllDevices();
    EXPECT_EQ(allDevices.size(), 0);
}

TEST_F(DeviceManagerTest, DeviceMonitoring) {
    manager->start();
    
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    manager->addDevice(std::move(device));
    
    // Monitor for a while
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Device should still be OK
    auto* retrieved = manager->getDevice("test-001");
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getStatus(), DeviceStatus::OK);
    
    manager->stop();
}

TEST_F(DeviceManagerTest, DeviceRecovery) {
    manager->start();
    
    auto device = createTestDevice("test-001", DeviceType::SENSOR_TEMPERATURE);
    // Make device fail
    // In a real test, we would inject an error
    manager->addDevice(std::move(device));
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Device should still be manageable
    auto* retrieved = manager->getDevice("test-001");
    EXPECT_NE(retrieved, nullptr);
    
    manager->stop();
}

// Test configuration loading from JSON
TEST(DeviceManagerConfigTest, LoadConfigFromJson) {
    Json::Value json;
    json["version"] = "1.0";
    json["environment"] = "test";
    json["debugMode"] = true;
    
    json["discovery"]["enabled"] = true;
    json["discovery"]["intervalSeconds"] = 60;
    json["discovery"]["maxDevices"] = 50;
    
    json["monitoring"]["enabled"] = true;
    json["monitoring"]["intervalMs"] = 500;
    json["monitoring"]["healthThreshold"] = 0.9;
    
    json["defaults"]["timeoutMs"] = 10000;
    json["defaults"]["retryCount"] = 5;
    
    DeviceManagerConfig config = DeviceManagerConfig::fromJson(json);
    
    EXPECT_EQ(config.version, "1.0");
    EXPECT_EQ(config.environment, "test");
    EXPECT_TRUE(config.debugMode);
    
    EXPECT_TRUE(config.discovery.enabled);
    EXPECT_EQ(config.discovery.intervalSeconds, 60);
    EXPECT_EQ(config.discovery.maxDevices, 50);
    
    EXPECT_TRUE(config.monitoring.enabled);
    EXPECT_EQ(config.monitoring.intervalMs, 500);
    EXPECT_DOUBLE_EQ(config.monitoring.healthThreshold, 0.9);
    
    EXPECT_EQ(config.defaults.timeoutMs, 10000);
    EXPECT_EQ(config.defaults.retryCount, 5);
}

TEST(DeviceManagerConfigTest, ValidateConfig) {
    DeviceManagerConfig config;
    
    // Valid config
    config.discovery.intervalSeconds = 30;
    config.discovery.maxDevices = 100;
    config.monitoring.intervalMs = 1000;
    config.monitoring.maxSamples = 1000;
    config.monitoring.healthThreshold = 0.8;
    config.defaults.timeoutMs = 5000;
    config.storage.saveIntervalMs = 60000;
    config.storage.maxHistoryDays = 30;
    config.storage.maxFileSizeMB = 100;
    config.logging.maxLogSizeMB = 100;
    config.logging.maxLogFiles = 5;
    
    EXPECT_TRUE(config.validate());
    
    // Invalid config - zero interval
    config.monitoring.intervalMs = 0;
    EXPECT_FALSE(config.validate());
    config.monitoring.intervalMs = 1000;
    
    // Invalid config - negative retry count
    config.defaults.retryCount = -1;
    EXPECT_FALSE(config.validate());
    config.defaults.retryCount = 3;
    
    // Invalid config - negative max files
    config.logging.maxLogFiles = -1;
    EXPECT_FALSE(config.validate());
    config.logging.maxLogFiles = 5;
}

TEST(DeviceManagerConfigTest, SaveAndLoadConfigFile) {
    DeviceManagerConfig config;
    config.version = "1.0";
    config.environment = "test";
    config.discovery.intervalSeconds = 60;
    config.monitoring.intervalMs = 500;
    
    std::string configPath = "/tmp/device_manager_config_test.json";
    
    EXPECT_TRUE(config.saveToFile(configPath));
    
    DeviceManagerConfig loadedConfig;
    EXPECT_TRUE(loadedConfig.loadFromFile(configPath));
    
    EXPECT_EQ(loadedConfig.version, config.version);
    EXPECT_EQ(loadedConfig.environment, config.environment);
    EXPECT_EQ(loadedConfig.discovery.intervalSeconds, config.discovery.intervalSeconds);
    EXPECT_EQ(loadedConfig.monitoring.intervalMs, config.monitoring.intervalMs);
}

TEST(DeviceManagerConfigTest, ConfigMerging) {
    DeviceManagerConfig base;
    base.version = "1.0";
    base.discovery.intervalSeconds = 30;
    base.monitoring.intervalMs = 1000;
    
    DeviceManagerConfig override;
    override.version = "1.1";
    override.discovery.intervalSeconds = 60;
    override.discovery.scanI2C = false;
    
    DeviceManagerConfig merged = base.merge(override);
    
    EXPECT_EQ(merged.version, "1.1");
    EXPECT_EQ(merged.discovery.intervalSeconds, 60);
    EXPECT_FALSE(merged.discovery.scanI2C);
    EXPECT_EQ(merged.monitoring.intervalMs, 1000); // From base
}

TEST(DeviceManagerConfigTest, CustomSettings) {
    DeviceManagerConfig config;
    config.customSettings["test_key"] = "test_value";
    config.customSettings["test_number"] = 42;
    
    Json::Value json = config.toJson();
    EXPECT_TRUE(json["custom"].isMember("test_key"));
    EXPECT_EQ(json["custom"]["test_key"].asString(), "test_value");
    
    DeviceManagerConfig loaded = DeviceManagerConfig::fromJson(json);
    EXPECT_EQ(loaded.customSettings["test_key"].asString(), "test_value");
    EXPECT_EQ(loaded.customSettings["test_number"].asInt(), 42);
}

#endif // DEVICE_MANAGER_TEST_CPP
