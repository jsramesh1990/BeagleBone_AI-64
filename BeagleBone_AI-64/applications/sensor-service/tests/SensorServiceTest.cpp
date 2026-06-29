#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <thread>
#include "sensor-service/SensorService.h"
#include "sensor-service/SensorManager.h"
#include "sensor-service/SensorEvents.h"

using namespace sensor_service;
using namespace testing;

/**
 * @brief Test fixture for SensorService tests
 */
class SensorServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        service = &SensorService::getInstance();
        
        // Reset service state
        service->stop();
        
        // Initialize with test config
        Json::Value config;
        config["collection_interval_ms"] = 100;
        config["health_check_interval_ms"] = 1000;
        config["auto_discover"] = false;
        config["service_name"] = "Test Service";
        config["version"] = "1.0.0-test";
        
        service->initialize(config);
    }
    
    void TearDown() override {
        service->stop();
        service->removeDataCallback();
        service->removeEventCallback();
        service->removeErrorCallback();
        service->removeStatusCallback();
    }
    
    SensorService* service;
};

// ============================================================================
// Service Lifecycle Tests
// ============================================================================

TEST_F(SensorServiceTest, SingletonInstance) {
    auto& instance1 = SensorService::getInstance();
    auto& instance2 = SensorService::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(SensorServiceTest, Initialize) {
    EXPECT_TRUE(service->isInitialized());
    EXPECT_EQ(service->getStatus(), SensorService::Status::STOPPED);
}

TEST_F(SensorServiceTest, InitializeWithConfig) {
    Json::Value config;
    config["collection_interval_ms"] = 500;
    config["health_check_interval_ms"] = 2000;
    config["auto_discover"] = true;
    config["service_name"] = "My Sensor Service";
    config["version"] = "2.0.0";
    
    service->stop();
    EXPECT_TRUE(service->initialize(config));
    EXPECT_TRUE(service->isInitialized());
    
    auto info = service->getServiceInfo();
    EXPECT_EQ(info.name, "My Sensor Service");
    EXPECT_EQ(info.version, "2.0.0");
}

TEST_F(SensorServiceTest, InitializeMultipleTimes) {
    EXPECT_TRUE(service->initialize());
    EXPECT_TRUE(service->initialize());
    EXPECT_TRUE(service->isInitialized());
}

TEST_F(SensorServiceTest, StartStop) {
    EXPECT_TRUE(service->start());
    EXPECT_TRUE(service->isRunning());
    EXPECT_EQ(service->getStatus(), SensorService::Status::RUNNING);
    
    EXPECT_TRUE(service->stop());
    EXPECT_FALSE(service->isRunning());
    EXPECT_EQ(service->getStatus(), SensorService::Status::STOPPED);
}

TEST_F(SensorServiceTest, Restart) {
    EXPECT_TRUE(service->start());
    EXPECT_TRUE(service->isRunning());
    
    EXPECT_TRUE(service->restart());
    EXPECT_TRUE(service->isRunning());
    
    service->stop();
}

TEST_F(SensorServiceTest, StartWithoutInitialization) {
    service->stop();
    // Re-initialize with false
    service->m_initialized = false;
    EXPECT_FALSE(service->start());
    service->m_initialized = true;
}

TEST_F(SensorServiceTest, StopWhenNotRunning) {
    EXPECT_TRUE(service->stop());
    EXPECT_FALSE(service->isRunning());
}

// ============================================================================
// Service Info Tests
// ============================================================================

TEST_F(SensorServiceTest, GetServiceInfo) {
    auto info = service->getServiceInfo();
    EXPECT_EQ(info.version, "1.0.0-test");
    EXPECT_EQ(info.name, "Test Service");
    EXPECT_EQ(info.status, SensorService::Status::STOPPED);
    EXPECT_TRUE(info.metadata.empty());
    
    // Start service
    service->start();
    info = service->getServiceInfo();
    EXPECT_EQ(info.status, SensorService::Status::RUNNING);
    
    service->stop();
}

TEST_F(SensorServiceTest, ServiceInfoToJson) {
    auto info = service->getServiceInfo();
    Json::Value json = info.toJson();
    
    EXPECT_TRUE(json.isObject());
    EXPECT_TRUE(json.isMember("version"));
    EXPECT_TRUE(json.isMember("name"));
    EXPECT_TRUE(json.isMember("description"));
    EXPECT_TRUE(json.isMember("status"));
    EXPECT_TRUE(json.isMember("statusText"));
    EXPECT_TRUE(json.isMember("startTime"));
    
    EXPECT_EQ(json["version"].asString(), "1.0.0-test");
    EXPECT_EQ(json["name"].asString(), "Test Service");
    EXPECT_EQ(json["statusText"].asString(), "STOPPED");
}

TEST_F(SensorServiceTest, ServiceMetadata) {
    auto& info = service->m_serviceInfo;
    info.metadata["key1"] = "value1";
    info.metadata["key2"] = "value2";
    
    auto retrieved = service->getServiceInfo();
    EXPECT_EQ(retrieved.metadata["key1"], "value1");
    EXPECT_EQ(retrieved.metadata["key2"], "value2");
}

// ============================================================================
// Sensor Operations Tests
// ============================================================================

TEST_F(SensorServiceTest, RegisterSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    EXPECT_TRUE(service->registerSensor(std::move(driver)));
    EXPECT_EQ(service->getSensorCount(), 1);
    EXPECT_TRUE(service->getSensor("test-001") != nullptr);
}

TEST_F(SensorServiceTest, RegisterMultipleSensors) {
    auto driver1 = std::make_unique<MockSensorDriver>("test-001");
    auto driver2 = std::make_unique<MockSensorDriver>("test-002");
    auto driver3 = std::make_unique<MockSensorDriver>("test-003");
    
    EXPECT_TRUE(service->registerSensor(std::move(driver1)));
    EXPECT_TRUE(service->registerSensor(std::move(driver2)));
    EXPECT_TRUE(service->registerSensor(std::move(driver3)));
    
    EXPECT_EQ(service->getSensorCount(), 3);
}

TEST_F(SensorServiceTest, UnregisterSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    EXPECT_TRUE(service->registerSensor(std::move(driver)));
    EXPECT_TRUE(service->getSensor("test-001") != nullptr);
    
    EXPECT_TRUE(service->unregisterSensor("test-001"));
    EXPECT_EQ(service->getSensorCount(), 0);
    EXPECT_TRUE(service->getSensor("test-001") == nullptr);
}

TEST_F(SensorServiceTest, GetSensorsByType) {
    auto driver1 = std::make_unique<MockSensorDriver>("temp-001", SensorType::TEMPERATURE);
    auto driver2 = std::make_unique<MockSensorDriver>("temp-002", SensorType::TEMPERATURE);
    auto driver3 = std::make_unique<MockSensorDriver>("imu-001", SensorType::IMU_ACCELEROMETER);
    
    service->registerSensor(std::move(driver1));
    service->registerSensor(std::move(driver2));
    service->registerSensor(std::move(driver3));
    
    auto tempSensors = service->getSensorsByType(SensorType::TEMPERATURE);
    EXPECT_EQ(tempSensors.size(), 2);
    
    auto imuSensors = service->getSensorsByType(SensorType::IMU_ACCELEROMETER);
    EXPECT_EQ(imuSensors.size(), 1);
}

// ============================================================================
// Data Access Tests
// ============================================================================

TEST_F(SensorServiceTest, ReadSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    SensorReading reading;
    EXPECT_TRUE(service->readSensor("test-001", reading));
    EXPECT_EQ(reading.sensorId, "test-001");
    EXPECT_TRUE(reading.isValid);
    EXPECT_GT(reading.values.size(), 0);
    
    service->stop();
}

TEST_F(SensorServiceTest, ReadAllSensors) {
    auto driver1 = std::make_unique<MockSensorDriver>("test-001");
    auto driver2 = std::make_unique<MockSensorDriver>("test-002");
    auto driver3 = std::make_unique<MockSensorDriver>("test-003");
    
    service->registerSensor(std::move(driver1));
    service->registerSensor(std::move(driver2));
    service->registerSensor(std::move(driver3));
    service->start();
    
    auto readings = service->readAllSensors();
    EXPECT_EQ(readings.size(), 3);
    
    for (const auto& reading : readings) {
        EXPECT_TRUE(reading.isValid);
        EXPECT_GT(reading.values.size(), 0);
    }
    
    service->stop();
}

TEST_F(SensorServiceTest, GetLastReading) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    SensorReading reading1;
    service->readSensor("test-001", reading1);
    
    auto reading2 = service->getLastReading("test-001");
    EXPECT_EQ(reading1.sensorId, reading2.sensorId);
    EXPECT_EQ(reading1.values["value"].asDouble(), reading2.values["value"].asDouble());
    
    service->stop();
}

TEST_F(SensorServiceTest, GetHistory) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    // Generate multiple readings
    for (int i = 0; i < 50; i++) {
        SensorReading reading;
        service->readSensor("test-001", reading);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto history = service->getHistory("test-001", 20);
    EXPECT_EQ(history.size(), 20);
    
    history = service->getHistory("test-001", 100);
    EXPECT_LE(history.size(), 50);
    
    service->stop();
}

TEST_F(SensorServiceTest, GetValueHistory) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    // Generate multiple readings
    for (int i = 0; i < 30; i++) {
        SensorReading reading;
        service->readSensor("test-001", reading);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto history = service->getValueHistory("test-001", "value", 20);
    EXPECT_EQ(history.size(), 20);
    
    for (const auto& [timestamp, value] : history) {
        EXPECT_GE(value, 15.0);
        EXPECT_LE(value, 25.0);
    }
    
    service->stop();
}

// ============================================================================
// Calibration Tests
// ============================================================================

TEST_F(SensorServiceTest, CalibrateSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    
    Json::Value params;
    params["offset"] = 0.5;
    params["scale"] = 1.2;
    
    EXPECT_TRUE(service->calibrateSensor("test-001", params));
}

TEST_F(SensorServiceTest, SelfTestSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    
    Json::Value result;
    EXPECT_TRUE(service->selfTestSensor("test-001", result));
    EXPECT_EQ(result["status"].asString(), "passed");
}

// ============================================================================
// Callback Tests
// ============================================================================

TEST_F(SensorServiceTest, DataCallback) {
    bool callbackCalled = false;
    SensorReading lastReading;
    
    service->setDataCallback([&](const SensorReading& reading) {
        callbackCalled = true;
        lastReading = reading;
    });
    
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    SensorReading reading;
    service->readSensor("test-001", reading);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(lastReading.sensorId, "test-001");
    
    service->stop();
    service->removeDataCallback();
}

TEST_F(SensorServiceTest, EventCallback) {
    bool callbackCalled = false;
    SensorEvent lastEvent;
    
    service->setEventCallback([&](const SensorEvent& event) {
        callbackCalled = true;
        lastEvent = event;
    });
    
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(lastEvent.type, SensorEventType::SENSOR_ADDED);
    
    service->removeEventCallback();
}

TEST_F(SensorServiceTest, ErrorCallback) {
    bool callbackCalled = false;
    std::string lastError;
    
    service->setErrorCallback([&](const std::string& error) {
        callbackCalled = true;
        lastError = error;
    });
    
    // Trigger an error by reading a non-existent sensor
    SensorReading reading;
    service->readSensor("nonexistent", reading);
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(lastError.empty());
    
    service->removeErrorCallback();
}

TEST_F(SensorServiceTest, StatusCallback) {
    bool callbackCalled = false;
    bool lastStatus = false;
    
    service->setStatusCallback([&](bool running) {
        callbackCalled = true;
        lastStatus = running;
    });
    
    service->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(lastStatus);
    
    callbackCalled = false;
    service->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(lastStatus);
    
    service->removeStatusCallback();
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(SensorServiceTest, GetStats) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    // Generate some readings
    for (int i = 0; i < 10; i++) {
        SensorReading reading;
        service->readSensor("test-001", reading);
    }
    
    Json::Value stats = service->getStats();
    EXPECT_TRUE(stats.isObject());
    EXPECT_TRUE(stats.isMember("readings_total"));
    EXPECT_TRUE(stats.isMember("errors"));
    EXPECT_TRUE(stats.isMember("sensor_count"));
    EXPECT_TRUE(stats.isMember("uptime_seconds"));
    
    EXPECT_GE(stats["readings_total"].asInt64(), 10);
    EXPECT_EQ(stats["sensor_count"].asInt(), 1);
    
    service->stop();
}

TEST_F(SensorServiceTest, ResetStats) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    // Generate some readings
    for (int i = 0; i < 5; i++) {
        SensorReading reading;
        service->readSensor("test-001", reading);
    }
    
    service->resetStats();
    Json::Value stats = service->getStats();
    EXPECT_EQ(stats["readings_total"].asInt64(), 0);
    
    service->stop();
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(SensorServiceTest, SaveConfig) {
    std::string configPath = "/tmp/sensor_service_config_test.json";
    EXPECT_TRUE(service->saveConfig(configPath));
    
    // Verify file exists
    std::ifstream file(configPath);
    EXPECT_TRUE(file.good());
    file.close();
}

TEST_F(SensorServiceTest, LoadConfig) {
    std::string configPath = "/tmp/sensor_service_config_test.json";
    
    // Create test config file
    Json::Value config;
    config["collection_interval_ms"] = 500;
    config["auto_discover"] = false;
    config["service_name"] = "Test Load";
    
    std::ofstream file(configPath);
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(config, &file);
    file.close();
    
    EXPECT_TRUE(service->loadConfig(configPath));
    
    auto loadedConfig = service->getConfig();
    EXPECT_EQ(loadedConfig["collection_interval_ms"].asInt(), 500);
    EXPECT_EQ(loadedConfig["service_name"].asString(), "Test Load");
}

// ============================================================================
// Health Tests
// ============================================================================

TEST_F(SensorServiceTest, HealthCheck) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    Json::Value health = service->healthCheck();
    EXPECT_TRUE(health.isObject());
    EXPECT_TRUE(health.isMember("healthy"));
    EXPECT_TRUE(health["healthy"].asBool());
    EXPECT_TRUE(health.isMember("sensor_count"));
    EXPECT_TRUE(health.isMember("uptime_seconds"));
    
    service->stop();
}

TEST_F(SensorServiceTest, IsHealthy) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    EXPECT_TRUE(service->isHealthy());
    
    service->stop();
}

TEST_F(SensorServiceTest, HealthThreshold) {
    EXPECT_DOUBLE_EQ(service->getHealthThreshold(), 0.8);
    
    service->setHealthThreshold(0.9);
    EXPECT_DOUBLE_EQ(service->getHealthThreshold(), 0.9);
    
    service->setHealthThreshold(0.5);
    EXPECT_DOUBLE_EQ(service->getHealthThreshold(), 0.5);
}

// ============================================================================
// Utility Tests
// ============================================================================

TEST_F(SensorServiceTest, SensorTypeToString) {
    EXPECT_EQ(SensorService::sensorTypeToString(SensorType::TEMPERATURE), "Temperature");
    EXPECT_EQ(SensorService::sensorTypeToString(SensorType::GPS), "GPS");
    EXPECT_EQ(SensorService::sensorTypeToString(SensorType::IMU_ACCELEROMETER), "IMU Accelerometer");
    EXPECT_EQ(SensorService::sensorTypeToString(SensorType::UNKNOWN), "Unknown");
}

TEST_F(SensorServiceTest, SensorTypeFromString) {
    EXPECT_EQ(SensorService::sensorTypeFromString("Temperature"), SensorType::TEMPERATURE);
    EXPECT_EQ(SensorService::sensorTypeFromString("GPS"), SensorType::GPS);
    EXPECT_EQ(SensorService::sensorTypeFromString("IMU Accelerometer"), SensorType::IMU_ACCELEROMETER);
    EXPECT_EQ(SensorService::sensorTypeFromString("Unknown"), SensorType::UNKNOWN);
}

// ============================================================================
// Discovery Tests
// ============================================================================

TEST_F(SensorServiceTest, DiscoverSensors) {
    int count = service->discoverSensors(1000);
    EXPECT_GE(count, 0);
}

TEST_F(SensorServiceTest, AutoDiscovery) {
    EXPECT_TRUE(service->isAutoDiscoveryEnabled());
    service->setAutoDiscovery(false);
    EXPECT_FALSE(service->isAutoDiscoveryEnabled());
    service->setAutoDiscovery(true);
    EXPECT_TRUE(service->isAutoDiscoveryEnabled());
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(SensorServiceTest, ReadNonExistentSensor) {
    SensorReading reading;
    EXPECT_FALSE(service->readSensor("nonexistent", reading));
}

TEST_F(SensorServiceTest, ConfigureNonExistentSensor) {
    Json::Value config;
    EXPECT_FALSE(service->configureSensor("nonexistent", config));
}

TEST_F(SensorServiceTest, CalibrateNonExistentSensor) {
    Json::Value params;
    EXPECT_FALSE(service->calibrateSensor("nonexistent", params));
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(SensorServiceTest, PerformanceManyReads) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    service->registerSensor(std::move(driver));
    service->start();
    
    auto start = std::chrono::steady_clock::now();
    const int READ_COUNT = 500;
    
    for (int i = 0; i < READ_COUNT; i++) {
        SensorReading reading;
        service->readSensor("test-001", reading);
    }
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    // Should be able to read 500 times in a reasonable time
    EXPECT_LT(elapsed.count(), 5000);
    
    service->stop();
}

#endif // SENSOR_SERVICE_TEST_CPP
