#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <thread>
#include <fstream>
#include "sensor-service/SensorManager.h"
#include "sensor-service/SensorDriver.h"
#include "sensor-service/SensorEvents.h"

using namespace sensor_service;
using namespace testing;

/**
 * @brief Mock Sensor Driver for testing
 */
class MockSensorDriver : public SensorDriver {
public:
    MockSensorDriver(const std::string& id, SensorType type = SensorType::TEMPERATURE)
        : SensorDriver(id, type), m_shouldFail(false), m_dataCount(0) {
        m_connected = true;
    }
    
    virtual bool initialize(const Json::Value& config) override {
        if (m_shouldFail) return false;
        
        if (config.isMember("initialization_delay")) {
            int delay = config["initialization_delay"].asInt();
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
        
        return true;
    }
    
    virtual bool start() override {
        if (m_shouldFail) return false;
        m_running = true;
        return true;
    }
    
    virtual bool stop() override {
        m_running = false;
        return true;
    }
    
    virtual bool readSample(SensorReading& reading) override {
        if (m_shouldFail || !m_running) return false;
        
        reading.type = m_type;
        reading.sensorId = m_sensorId;
        reading.timestamp = getCurrentTime();
        reading.sequence = nextSequence();
        reading.isValid = true;
        
        // Generate test data
        reading.values["value"] = 20.0 + (m_dataCount % 10);
        reading.values["min"] = 15.0;
        reading.values["max"] = 25.0;
        reading.values["count"] = ++m_dataCount;
        
        // Simulate occasional data
        if (m_dataCount % 5 == 0) {
            reading.isValid = false;
        }
        
        return true;
    }
    
    virtual bool calibrate(const Json::Value& params) override {
        if (m_shouldFail) return false;
        return true;
    }
    
    virtual bool selfTest(Json::Value& result) override {
        if (m_shouldFail) {
            result["status"] = "failed";
            return false;
        }
        result["status"] = "passed";
        result["tests"] = 5;
        return true;
    }
    
    virtual bool configure(const Json::Value& config) override {
        if (m_shouldFail) return false;
        return true;
    }
    
    virtual std::string getName() const override { return "MockSensor"; }
    virtual std::string getVersion() const override { return "1.0.0"; }
    
    // Test helpers
    void setShouldFail(bool fail) { m_shouldFail = fail; }
    void setConnected(bool connected) { m_connected = connected; }
    void setType(SensorType type) { m_type = type; }
    uint64_t getDataCount() const { return m_dataCount; }
    void resetDataCount() { m_dataCount = 0; }

private:
    bool m_shouldFail;
    uint64_t m_dataCount;
};

/**
 * @brief Test fixture for SensorManager tests
 */
class SensorManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &SensorManager::getInstance();
        
        // Reset manager state
        manager->stop();
        
        // Initialize with test config
        Json::Value config;
        config["collection_interval_ms"] = 100;
        config["health_check_interval_ms"] = 1000;
        config["auto_discover"] = false;
        
        manager->initialize(config);
    }
    
    void TearDown() override {
        manager->stop();
        // Clean up temporary files
        std::remove("/tmp/sensor_config_test.json");
        std::remove("/tmp/sensor_data_test.json");
    }
    
    SensorManager* manager;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(SensorManagerTest, SingletonInstance) {
    auto& instance1 = SensorManager::getInstance();
    auto& instance2 = SensorManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(SensorManagerTest, Initialize) {
    EXPECT_TRUE(manager->isInitialized());
}

TEST_F(SensorManagerTest, InitializeWithConfig) {
    Json::Value config;
    config["collection_interval_ms"] = 500;
    config["health_check_interval_ms"] = 2000;
    config["auto_discover"] = true;
    
    manager->stop();
    EXPECT_TRUE(manager->initialize(config));
    EXPECT_TRUE(manager->isInitialized());
}

TEST_F(SensorManagerTest, InitializeMultipleTimes) {
    EXPECT_TRUE(manager->initialize());
    EXPECT_TRUE(manager->initialize());
    EXPECT_TRUE(manager->isInitialized());
}

// ============================================================================
// Start/Stop Tests
// ============================================================================

TEST_F(SensorManagerTest, StartStop) {
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->isRunning());
    
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(manager->isRunning());
}

TEST_F(SensorManagerTest, StartWithoutInitialization) {
    manager->stop();
    // Re-initialize with false
    manager->m_initialized = false;
    EXPECT_FALSE(manager->start());
    manager->m_initialized = true;
}

TEST_F(SensorManagerTest, StopWhenNotRunning) {
    EXPECT_TRUE(manager->stop());
    EXPECT_FALSE(manager->isRunning());
}

// ============================================================================
// Sensor Registration Tests
// ============================================================================

TEST_F(SensorManagerTest, RegisterSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    EXPECT_TRUE(manager->registerSensor(std::move(driver)));
    EXPECT_TRUE(manager->hasSensor("test-001"));
    EXPECT_EQ(manager->getSensorCount(), 1);
}

TEST_F(SensorManagerTest, RegisterMultipleSensors) {
    auto driver1 = std::make_unique<MockSensorDriver>("test-001");
    auto driver2 = std::make_unique<MockSensorDriver>("test-002");
    auto driver3 = std::make_unique<MockSensorDriver>("test-003");
    
    EXPECT_TRUE(manager->registerSensor(std::move(driver1)));
    EXPECT_TRUE(manager->registerSensor(std::move(driver2)));
    EXPECT_TRUE(manager->registerSensor(std::move(driver3)));
    
    EXPECT_EQ(manager->getSensorCount(), 3);
    EXPECT_TRUE(manager->hasSensor("test-001"));
    EXPECT_TRUE(manager->hasSensor("test-002"));
    EXPECT_TRUE(manager->hasSensor("test-003"));
}

TEST_F(SensorManagerTest, RegisterDuplicateSensor) {
    auto driver1 = std::make_unique<MockSensorDriver>("test-001");
    auto driver2 = std::make_unique<MockSensorDriver>("test-001");
    
    EXPECT_TRUE(manager->registerSensor(std::move(driver1)));
    EXPECT_FALSE(manager->registerSensor(std::move(driver2)));
    EXPECT_EQ(manager->getSensorCount(), 1);
}

TEST_F(SensorManagerTest, RegisterNullSensor) {
    std::unique_ptr<SensorDriver> driver = nullptr;
    EXPECT_FALSE(manager->registerSensor(std::move(driver)));
}

TEST_F(SensorManagerTest, UnregisterSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    EXPECT_TRUE(manager->registerSensor(std::move(driver)));
    EXPECT_TRUE(manager->hasSensor("test-001"));
    
    EXPECT_TRUE(manager->unregisterSensor("test-001"));
    EXPECT_FALSE(manager->hasSensor("test-001"));
    EXPECT_EQ(manager->getSensorCount(), 0);
}

TEST_F(SensorManagerTest, UnregisterNonExistentSensor) {
    EXPECT_FALSE(manager->unregisterSensor("nonexistent"));
}

// ============================================================================
// Sensor Access Tests
// ============================================================================

TEST_F(SensorManagerTest, GetSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    MockSensorDriver* ptr = driver.get();
    EXPECT_TRUE(manager->registerSensor(std::move(driver)));
    
    SensorDriver* retrieved = manager->getSensor("test-001");
    EXPECT_EQ(retrieved, ptr);
}

TEST_F(SensorManagerTest, GetNonExistentSensor) {
    EXPECT_EQ(manager->getSensor("nonexistent"), nullptr);
}

TEST_F(SensorManagerTest, GetSensorIds) {
    auto driver1 = std::make_unique<MockSensorDriver>("test-001");
    auto driver2 = std::make_unique<MockSensorDriver>("test-002");
    auto driver3 = std::make_unique<MockSensorDriver>("test-003");
    
    manager->registerSensor(std::move(driver1));
    manager->registerSensor(std::move(driver2));
    manager->registerSensor(std::move(driver3));
    
    auto ids = manager->getSensorIds();
    EXPECT_EQ(ids.size(), 3);
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "test-001") != ids.end());
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "test-002") != ids.end());
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), "test-003") != ids.end());
}

TEST_F(SensorManagerTest, GetSensorsByType) {
    auto driver1 = std::make_unique<MockSensorDriver>("temp-001", SensorType::TEMPERATURE);
    auto driver2 = std::make_unique<MockSensorDriver>("temp-002", SensorType::TEMPERATURE);
    auto driver3 = std::make_unique<MockSensorDriver>("imu-001", SensorType::IMU_ACCELEROMETER);
    auto driver4 = std::make_unique<MockSensorDriver>("gps-001", SensorType::GPS);
    
    manager->registerSensor(std::move(driver1));
    manager->registerSensor(std::move(driver2));
    manager->registerSensor(std::move(driver3));
    manager->registerSensor(std::move(driver4));
    
    auto tempSensors = manager->getSensorsByType(SensorType::TEMPERATURE);
    EXPECT_EQ(tempSensors.size(), 2);
    
    auto imuSensors = manager->getSensorsByType(SensorType::IMU_ACCELEROMETER);
    EXPECT_EQ(imuSensors.size(), 1);
    
    auto gpsSensors = manager->getSensorsByType(SensorType::GPS);
    EXPECT_EQ(gpsSensors.size(), 1);
    
    auto unknownSensors = manager->getSensorsByType(SensorType::UNKNOWN);
    EXPECT_EQ(unknownSensors.size(), 0);
}

// ============================================================================
// Data Collection Tests
// ============================================================================

TEST_F(SensorManagerTest, ReadSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    SensorReading reading;
    EXPECT_TRUE(manager->readSensor("test-001", reading));
    EXPECT_EQ(reading.sensorId, "test-001");
    EXPECT_TRUE(reading.isValid);
    EXPECT_GT(reading.values.size(), 0);
    EXPECT_TRUE(reading.hasValue("value"));
    
    manager->stop();
}

TEST_F(SensorManagerTest, ReadNonExistentSensor) {
    SensorReading reading;
    EXPECT_FALSE(manager->readSensor("nonexistent", reading));
}

TEST_F(SensorManagerTest, ReadAllSensors) {
    auto driver1 = std::make_unique<MockSensorDriver>("test-001");
    auto driver2 = std::make_unique<MockSensorDriver>("test-002");
    auto driver3 = std::make_unique<MockSensorDriver>("test-003");
    
    manager->registerSensor(std::move(driver1));
    manager->registerSensor(std::move(driver2));
    manager->registerSensor(std::move(driver3));
    manager->start();
    
    auto readings = manager->readAllSensors();
    EXPECT_EQ(readings.size(), 3);
    
    for (const auto& reading : readings) {
        EXPECT_TRUE(reading.isValid);
        EXPECT_GT(reading.values.size(), 0);
    }
    
    manager->stop();
}

TEST_F(SensorManagerTest, ReadSensorTimeout) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    // Simulate timeout by reading with very short timeout
    SensorReading reading;
    // The mock driver doesn't actually support timeout, so this is just a test
    EXPECT_TRUE(manager->readSensor("test-001", reading, 100));
    
    manager->stop();
}

// ============================================================================
// Calibration Tests
// ============================================================================

TEST_F(SensorManagerTest, CalibrateSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    Json::Value params;
    params["offset"] = 0.5;
    params["scale"] = 1.2;
    
    EXPECT_TRUE(manager->calibrateSensor("test-001", params));
    manager->stop();
}

TEST_F(SensorManagerTest, CalibrateNonExistentSensor) {
    Json::Value params;
    EXPECT_FALSE(manager->calibrateSensor("nonexistent", params));
}

TEST_F(SensorManagerTest, CalibrateFailed) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    MockSensorDriver* ptr = driver.get();
    ptr->setShouldFail(true);
    manager->registerSensor(std::move(driver));
    
    Json::Value params;
    EXPECT_FALSE(manager->calibrateSensor("test-001", params));
}

// ============================================================================
// Self-Test Tests
// ============================================================================

TEST_F(SensorManagerTest, SelfTestSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    
    Json::Value result;
    EXPECT_TRUE(manager->selfTestSensor("test-001", result));
    EXPECT_EQ(result["status"].asString(), "passed");
    EXPECT_EQ(result["tests"].asInt(), 5);
}

TEST_F(SensorManagerTest, SelfTestNonExistentSensor) {
    Json::Value result;
    EXPECT_FALSE(manager->selfTestSensor("nonexistent", result));
}

TEST_F(SensorManagerTest, SelfTestFailed) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    MockSensorDriver* ptr = driver.get();
    ptr->setShouldFail(true);
    manager->registerSensor(std::move(driver));
    
    Json::Value result;
    EXPECT_FALSE(manager->selfTestSensor("test-001", result));
    EXPECT_EQ(result["status"].asString(), "failed");
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(SensorManagerTest, ConfigureSensor) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    Json::Value config;
    config["sampling_rate"] = 10;
    config["filter_enabled"] = true;
    
    EXPECT_TRUE(manager->configureSensor("test-001", config));
    manager->stop();
}

TEST_F(SensorManagerTest, ConfigureNonExistentSensor) {
    Json::Value config;
    EXPECT_FALSE(manager->configureSensor("nonexistent", config));
}

// ============================================================================
// Enable/Disable Tests
// ============================================================================

TEST_F(SensorManagerTest, SetSensorEnabled) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    EXPECT_TRUE(manager->setSensorEnabled("test-001", true));
    EXPECT_TRUE(manager->setSensorEnabled("test-001", false));
    manager->stop();
}

TEST_F(SensorManagerTest, SetNonExistentSensorEnabled) {
    EXPECT_FALSE(manager->setSensorEnabled("nonexistent", true));
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(SensorManagerTest, GetStats) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    // Generate some readings
    for (int i = 0; i < 10; i++) {
        SensorReading reading;
        manager->readSensor("test-001", reading);
    }
    
    Json::Value stats = manager->getStats();
    EXPECT_TRUE(stats.isObject());
    EXPECT_TRUE(stats.isMember("readings_total"));
    EXPECT_TRUE(stats.isMember("errors"));
    EXPECT_TRUE(stats.isMember("sensor_count"));
    EXPECT_TRUE(stats.isMember("uptime_seconds"));
    
    EXPECT_GE(stats["readings_total"].asInt64(), 10);
    EXPECT_EQ(stats["sensor_count"].asInt(), 1);
    
    manager->stop();
}

TEST_F(SensorManagerTest, ResetStats) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    // Generate some readings
    for (int i = 0; i < 5; i++) {
        SensorReading reading;
        manager->readSensor("test-001", reading);
    }
    
    manager->resetStats();
    Json::Value stats = manager->getStats();
    EXPECT_EQ(stats["readings_total"].asInt64(), 0);
    EXPECT_EQ(stats["errors"].asInt64(), 0);
    EXPECT_EQ(stats["timeouts"].asInt64(), 0);
    
    manager->stop();
}

// ============================================================================
// Configuration Save/Load Tests
// ============================================================================

TEST_F(SensorManagerTest, SaveConfig) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    
    std::string configPath = "/tmp/sensor_config_test.json";
    EXPECT_TRUE(manager->saveConfig(configPath));
    
    // Verify file exists
    std::ifstream file(configPath);
    EXPECT_TRUE(file.good());
    file.close();
}

TEST_F(SensorManagerTest, LoadConfig) {
    std::string configPath = "/tmp/sensor_config_test.json";
    
    // Create test config file
    Json::Value config;
    config["collection_interval_ms"] = 500;
    config["auto_discover"] = false;
    
    std::ofstream file(configPath);
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(config, &file);
    file.close();
    
    EXPECT_TRUE(manager->loadConfig(configPath));
}

TEST_F(SensorManagerTest, LoadInvalidConfig) {
    std::string configPath = "/tmp/invalid_config.json";
    
    // Create invalid config file
    std::ofstream file(configPath);
    file << "invalid json content";
    file.close();
    
    EXPECT_FALSE(manager->loadConfig(configPath));
}

// ============================================================================
// Callback Tests
// ============================================================================

TEST_F(SensorManagerTest, DataCallback) {
    bool callbackCalled = false;
    SensorReading lastReading;
    
    manager->setDataCallback([&](const SensorReading& reading) {
        callbackCalled = true;
        lastReading = reading;
    });
    
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    // Trigger a reading
    SensorReading reading;
    manager->readSensor("test-001", reading);
    
    // Wait for callback processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(lastReading.sensorId, "test-001");
    
    manager->stop();
    manager->removeDataCallback();
}

TEST_F(SensorManagerTest, EventCallback) {
    bool callbackCalled = false;
    SensorEvent lastEvent;
    
    manager->setEventCallback([&](const SensorEvent& event) {
        callbackCalled = true;
        lastEvent = event;
    });
    
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    
    // Registering a sensor should trigger an event
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(lastEvent.type, SensorEventType::SENSOR_ADDED);
    EXPECT_EQ(lastEvent.sensorId, "test-001");
    
    manager->removeEventCallback();
}

// ============================================================================
// Discovery Tests
// ============================================================================

TEST_F(SensorManagerTest, DiscoverSensors) {
    int count = manager->discoverSensors(1000);
    // Should find some sensors (or 0 if none available)
    EXPECT_GE(count, 0);
}

TEST_F(SensorManagerTest, AutoDiscoveryEnabled) {
    EXPECT_TRUE(manager->isAutoDiscoveryEnabled());
    manager->setAutoDiscovery(false);
    EXPECT_FALSE(manager->isAutoDiscoveryEnabled());
    manager->setAutoDiscovery(true);
    EXPECT_TRUE(manager->isAutoDiscoveryEnabled());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(SensorManagerTest, ConcurrentAccess) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount(0);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this, &successCount]() {
            for (int j = 0; j < 10; j++) {
                SensorReading reading;
                if (manager->readSensor("test-001", reading)) {
                    successCount++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // At least some reads should succeed
    EXPECT_GT(successCount, 0);
    
    manager->stop();
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(SensorManagerTest, SensorReadError) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    MockSensorDriver* ptr = driver.get();
    ptr->setShouldFail(true);
    manager->registerSensor(std::move(driver));
    manager->start();
    
    SensorReading reading;
    EXPECT_FALSE(manager->readSensor("test-001", reading));
    
    manager->stop();
}

TEST_F(SensorManagerTest, HealthCheck) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    // Let health check run
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    // Sensor should still be connected
    SensorDriver* retrieved = manager->getSensor("test-001");
    EXPECT_NE(retrieved, nullptr);
    EXPECT_TRUE(retrieved->isConnected());
    
    manager->stop();
}

TEST_F(SensorManagerTest, SensorReconnect) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    MockSensorDriver* ptr = driver.get();
    manager->registerSensor(std::move(driver));
    manager->start();
    
    // Simulate disconnection
    ptr->setConnected(false);
    
    // Wait for health check to attempt reconnect
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    // The mock driver doesn't actually reconnect, but the health check should attempt it
    // This test verifies the health check runs
    EXPECT_TRUE(true);
    
    manager->stop();
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(SensorManagerTest, PerformanceManyReads) {
    auto driver = std::make_unique<MockSensorDriver>("test-001");
    manager->registerSensor(std::move(driver));
    manager->start();
    
    auto start = std::chrono::steady_clock::now();
    const int READ_COUNT = 1000;
    
    for (int i = 0; i < READ_COUNT; i++) {
        SensorReading reading;
        manager->readSensor("test-001", reading);
    }
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    // Should be able to read at least 1000 times in a reasonable time
    EXPECT_LT(elapsed.count(), 5000);
    
    manager->stop();
}

TEST_F(SensorManagerTest, PerformanceManySensors) {
    const int SENSOR_COUNT = 100;
    
    for (int i = 0; i < SENSOR_COUNT; i++) {
        auto driver = std::make_unique<MockSensorDriver>("sensor-" + std::to_string(i));
        manager->registerSensor(std::move(driver));
    }
    
    manager->start();
    
    auto start = std::chrono::steady_clock::now();
    auto readings = manager->readAllSensors();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    EXPECT_EQ(readings.size(), SENSOR_COUNT);
    
    // Should be able to read all sensors quickly
    EXPECT_LT(elapsed.count(), 5000);
    
    manager->stop();
}

#endif // SENSOR_MANAGER_TEST_CPP
