#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QTimer>
#include <QFile>
#include <QTemporaryFile>
#include <QDebug>
#include "gui-app/DataManager.h"

using namespace testing;

/**
 * @brief Test fixture for DataManager tests
 */
class DataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        dataManager = new DataManager();
        // Wait for initialization
        QTest::qWait(10);
    }
    
    void TearDown() override {
        dataManager->stopUpdates();
        delete dataManager;
    }
    
    DataManager* dataManager;
    
    /**
     * @brief Generate test data with specific values
     */
    QVariantMap generateTestData(const QString& deviceId = "sensor-001",
                                 double temperature = 25.0,
                                 double humidity = 60.0,
                                 double pressure = 1013.0) {
        QVariantMap data;
        data["deviceId"] = deviceId;
        data["type"] = "temperature";
        data["temperature"] = temperature;
        data["humidity"] = humidity;
        data["pressure"] = pressure;
        data["isValid"] = true;
        data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return data;
    }
};

// ============================================================================
// Constructor and Initialization Tests
// ============================================================================

TEST_F(DataManagerTest, Constructor) {
    EXPECT_NE(dataManager, nullptr);
    EXPECT_EQ(dataManager->dataPoints(), 0);
    EXPECT_TRUE(dataManager->currentData().isEmpty());
    EXPECT_TRUE(dataManager->deviceStatus().isEmpty());
}

TEST_F(DataManagerTest, StartUpdates) {
    QSignalSpy spy(dataManager, &DataManager::dataUpdated);
    
    dataManager->startUpdates();
    EXPECT_TRUE(dataManager->m_updateTimer.isActive());
    
    // Wait for some updates
    QTest::qWait(1500);
    EXPECT_GT(spy.count(), 0);
}

TEST_F(DataManagerTest, StopUpdates) {
    dataManager->startUpdates();
    EXPECT_TRUE(dataManager->m_updateTimer.isActive());
    
    dataManager->stopUpdates();
    EXPECT_FALSE(dataManager->m_updateTimer.isActive());
}

// ============================================================================
// Data Processing Tests
// ============================================================================

TEST_F(DataManagerTest, HandleDeviceData) {
    QSignalSpy dataSpy(dataManager, &DataManager::sensorDataReceived);
    QSignalSpy updateSpy(dataManager, &DataManager::dataUpdated);
    
    QVariantMap testData = generateTestData("test-001", 25.5, 65.0, 1015.0);
    dataManager->handleDeviceData("test-001", testData);
    
    // Wait for async processing
    QTest::qWait(100);
    
    EXPECT_GT(dataSpy.count(), 0);
    EXPECT_GT(updateSpy.count(), 0);
    
    // Check that data was stored
    QVariantList history = dataManager->getHistory("test-001", 10);
    EXPECT_GT(history.size(), 0);
}

TEST_F(DataManagerTest, ProcessMultipleDevices) {
    QVariantMap data1 = generateTestData("device-001", 22.0, 55.0, 1012.0);
    QVariantMap data2 = generateTestData("device-002", 28.0, 70.0, 1010.0);
    QVariantMap data3 = generateTestData("device-003", 20.0, 45.0, 1014.0);
    
    dataManager->handleDeviceData("device-001", data1);
    dataManager->handleDeviceData("device-002", data2);
    dataManager->handleDeviceData("device-003", data3);
    
    QTest::qWait(100);
    
    QStringList deviceIds = dataManager->getDeviceIds();
    EXPECT_EQ(deviceIds.size(), 3);
    EXPECT_TRUE(deviceIds.contains("device-001"));
    EXPECT_TRUE(deviceIds.contains("device-002"));
    EXPECT_TRUE(deviceIds.contains("device-003"));
}

TEST_F(DataManagerTest, HandleInvalidData) {
    QVariantMap invalidData;
    invalidData["deviceId"] = "test-001";
    invalidData["isValid"] = false;
    
    dataManager->handleDeviceData("test-001", invalidData);
    QTest::qWait(100);
    
    QVariantMap info = dataManager->getDeviceInfo("test-001");
    EXPECT_TRUE(info.contains("isValid"));
    EXPECT_FALSE(info["isValid"].toBool());
}

TEST_F(DataManagerTest, HandleDeviceStatus) {
    QSignalSpy statusSpy(dataManager, &DataManager::statusUpdated);
    
    dataManager->handleDeviceStatus("test-001", 0); // OK status
    QTest::qWait(50);
    
    EXPECT_GT(statusSpy.count(), 0);
    EXPECT_TRUE(dataManager->deviceStatus().contains("test-001"));
    EXPECT_EQ(dataManager->deviceStatus()["test-001"].toInt(), 0);
}

// ============================================================================
// History Management Tests
// ============================================================================

TEST_F(DataManagerTest, GetHistory) {
    // Add multiple data points
    for (int i = 0; i < 50; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i * 0.1, 50.0 + i * 0.2, 1013.0);
        dataManager->handleDeviceData("test-001", data);
    }
    QTest::qWait(100);
    
    // Get last 10 points
    QVariantList history = dataManager->getHistory("test-001", 10);
    EXPECT_EQ(history.size(), 10);
    
    // Get all points
    history = dataManager->getHistory("test-001", 100);
    EXPECT_EQ(history.size(), 50);
}

TEST_F(DataManagerTest, HistoryMaxSize) {
    // Add more than MAX_HISTORY_SIZE points
    for (int i = 0; i < DataManager::MAX_HISTORY_SIZE + 100; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i * 0.1);
        dataManager->handleDeviceData("test-001", data);
    }
    QTest::qWait(200);
    
    QVariantList history = dataManager->getHistory("test-001", DataManager::MAX_HISTORY_SIZE + 50);
    EXPECT_LE(history.size(), DataManager::MAX_HISTORY_SIZE);
}

TEST_F(DataManagerTest, ClearHistory) {
    // Add some data
    for (int i = 0; i < 10; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i);
        dataManager->handleDeviceData("test-001", data);
    }
    QTest::qWait(100);
    
    EXPECT_GT(dataManager->dataPoints(), 0);
    
    dataManager->clearHistory();
    QTest::qWait(50);
    
    EXPECT_EQ(dataManager->dataPoints(), 0);
    QVariantList history = dataManager->getHistory("test-001", 10);
    EXPECT_EQ(history.size(), 0);
}

TEST_F(DataManagerTest, GetDeviceInfo) {
    QVariantMap data = generateTestData("test-001", 25.5, 65.0, 1015.0);
    dataManager->handleDeviceData("test-001", data);
    QTest::qWait(100);
    
    QVariantMap info = dataManager->getDeviceInfo("test-001");
    EXPECT_EQ(info["deviceId"].toString(), "test-001");
    EXPECT_EQ(info["type"].toString(), "temperature");
    EXPECT_TRUE(info.contains("lastUpdate"));
    EXPECT_TRUE(info["isValid"].toBool());
    EXPECT_GT(info["dataPoints"].toInt(), 0);
    
    // Non-existent device
    QVariantMap emptyInfo = dataManager->getDeviceInfo("nonexistent");
    EXPECT_TRUE(emptyInfo.isEmpty());
}

TEST_F(DataManagerTest, GetDeviceIds) {
    QStringList ids = {"dev-001", "dev-002", "dev-003"};
    for (const QString& id : ids) {
        QVariantMap data = generateTestData(id);
        dataManager->handleDeviceData(id, data);
    }
    QTest::qWait(100);
    
    QStringList result = dataManager->getDeviceIds();
    EXPECT_EQ(result.size(), 3);
    for (const QString& id : ids) {
        EXPECT_TRUE(result.contains(id));
    }
}

// ============================================================================
// Current Data Tests
// ============================================================================

TEST_F(DataManagerTest, CurrentDataUpdates) {
    QSignalSpy spy(dataManager, &DataManager::dataUpdated);
    
    QVariantMap testData = generateTestData("test-001", 30.0, 75.0, 1020.0);
    dataManager->handleDeviceData("test-001", testData);
    QTest::qWait(100);
    
    // Check that currentData contains the latest data
    QVariantMap current = dataManager->currentData();
    EXPECT_FALSE(current.isEmpty());
    EXPECT_EQ(current["temperature"].toDouble(), 30.0);
    EXPECT_EQ(current["humidity"].toDouble(), 75.0);
    EXPECT_EQ(current["pressure"].toDouble(), 1020.0);
}

TEST_F(DataManagerTest, MultipleDataUpdates) {
    QSignalSpy spy(dataManager, &DataManager::dataUpdated);
    
    // Send multiple updates with different values
    for (int i = 0; i < 5; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i * 2.0);
        dataManager->handleDeviceData("test-001", data);
        QTest::qWait(50);
    }
    
    // Check that we received all updates
    EXPECT_GE(spy.count(), 5);
    
    // Check that currentData has the latest value
    QVariantMap current = dataManager->currentData();
    EXPECT_EQ(current["temperature"].toDouble(), 28.0);
}

// ============================================================================
// Save/Load Tests
// ============================================================================

TEST_F(DataManagerTest, SaveData) {
    // Add test data
    for (int i = 0; i < 10; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i);
        dataManager->handleDeviceData("test-001", data);
    }
    QTest::qWait(100);
    
    QTemporaryFile tempFile;
    tempFile.open();
    QString filename = tempFile.fileName();
    tempFile.close();
    
    // Save data
    dataManager->saveData(filename);
    
    // Verify file exists and has content
    QFile file(filename);
    EXPECT_TRUE(file.exists());
    EXPECT_GT(file.size(), 0);
    file.remove();
}

TEST_F(DataManagerTest, LoadData) {
    // First, save some data
    for (int i = 0; i < 10; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i);
        dataManager->handleDeviceData("test-001", data);
    }
    QTest::qWait(100);
    
    QTemporaryFile tempFile;
    tempFile.open();
    QString filename = tempFile.fileName();
    tempFile.close();
    
    dataManager->saveData(filename);
    
    // Clear history
    dataManager->clearHistory();
    EXPECT_EQ(dataManager->dataPoints(), 0);
    
    // Load data back
    dataManager->loadData(filename);
    QTest::qWait(100);
    
    // Check that data was restored
    QVariantList history = dataManager->getHistory("test-001", 10);
    EXPECT_GT(history.size(), 0);
    
    QFile::remove(filename);
}

TEST_F(DataManagerTest, LoadInvalidData) {
    QSignalSpy errorSpy(dataManager, &DataManager::error);
    
    // Try to load non-existent file
    dataManager->loadData("/tmp/nonexistent_file_12345.json");
    QTest::qWait(100);
    
    EXPECT_GT(errorSpy.count(), 0);
}

TEST_F(DataManagerTest, SaveMultipleDevices) {
    // Add data for multiple devices
    QStringList devices = {"dev-001", "dev-002", "dev-003"};
    for (const QString& device : devices) {
        for (int i = 0; i < 5; ++i) {
            QVariantMap data = generateTestData(device, 20.0 + i);
            dataManager->handleDeviceData(device, data);
        }
    }
    QTest::qWait(100);
    
    QTemporaryFile tempFile;
    tempFile.open();
    QString filename = tempFile.fileName();
    tempFile.close();
    
    dataManager->saveData(filename);
    
    // Clear and reload
    dataManager->clearHistory();
    dataManager->loadData(filename);
    QTest::qWait(100);
    
    // Check all devices were restored
    QStringList restored = dataManager->getDeviceIds();
    EXPECT_EQ(restored.size(), 3);
    for (const QString& device : devices) {
        EXPECT_TRUE(restored.contains(device));
        QVariantList history = dataManager->getHistory(device, 10);
        EXPECT_EQ(history.size(), 5);
    }
    
    QFile::remove(filename);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(DataManagerTest, ErrorSignal) {
    QSignalSpy errorSpy(dataManager, &DataManager::error);
    
    // Trigger an error by invalid operation
    dataManager->loadData(""); // Invalid filename
    QTest::qWait(50);
    
    EXPECT_GT(errorSpy.count(), 0);
}

TEST_F(DataManagerTest, TimeoutCheck) {
    // Create a device with old data
    QVariantMap data = generateTestData("test-001", 25.0);
    dataManager->handleDeviceData("test-001", data);
    QTest::qWait(100);
    
    // Wait for timeout check (TIMEOUT_MS is 30000, so we can't wait that long in a test)
    // Instead, we manually trigger the check
    dataManager->checkTimeouts();
    
    // Device should still be considered active
    QVariantMap info = dataManager->getDeviceInfo("test-001");
    EXPECT_TRUE(info["isValid"].toBool());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(DataManagerTest, ConcurrentDataProcessing) {
    // Simulate concurrent data processing
    QList<std::thread> threads;
    QAtomicInt counter = 0;
    
    for (int i = 0; i < 10; ++i) {
        threads.append(std::thread([this, i, &counter]() {
            for (int j = 0; j < 10; ++j) {
                QVariantMap data = generateTestData(
                    "thread-" + QString::number(i),
                    20.0 + j,
                    50.0 + j,
                    1013.0 + j
                );
                dataManager->handleDeviceData("thread-" + QString::number(i), data);
                counter++;
            }
        }));
    }
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    QTest::qWait(200);
    
    // Check that all data was processed
    QStringList deviceIds = dataManager->getDeviceIds();
    EXPECT_EQ(deviceIds.size(), 10);
    for (int i = 0; i < 10; ++i) {
        QString id = "thread-" + QString::number(i);
        QVariantList history = dataManager->getHistory(id, 20);
        EXPECT_EQ(history.size(), 10);
    }
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(DataManagerTest, PerformanceLargeData) {
    QElapsedTimer timer;
    timer.start();
    
    // Add large amount of data
    const int DATA_POINTS = 5000;
    for (int i = 0; i < DATA_POINTS; ++i) {
        QVariantMap data = generateTestData("perf-001", 20.0 + (i % 100) * 0.1);
        dataManager->handleDeviceData("perf-001", data);
    }
    QTest::qWait(500);
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Processed " << DATA_POINTS << " data points in " << elapsed << "ms";
    
    // Should be reasonable performance
    EXPECT_LT(elapsed, 5000); // Less than 5 seconds
}

TEST_F(DataManagerTest, HistoryRetrievalPerformance) {
    // Add data first
    for (int i = 0; i < 1000; ++i) {
        QVariantMap data = generateTestData("perf-002", 20.0 + i * 0.1);
        dataManager->handleDeviceData("perf-002", data);
    }
    QTest::qWait(200);
    
    QElapsedTimer timer;
    timer.start();
    
    // Retrieve history multiple times
    for (int i = 0; i < 100; ++i) {
        QVariantList history = dataManager->getHistory("perf-002", 100);
        EXPECT_EQ(history.size(), 100);
    }
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Retrieved history 100 times in " << elapsed << "ms";
    EXPECT_LT(elapsed, 1000);
}

// ============================================================================
// Edge Cases Tests
// ============================================================================

TEST_F(DataManagerTest, EmptyDeviceId) {
    QVariantMap data = generateTestData("", 25.0);
    dataManager->handleDeviceData("", data);
    QTest::qWait(100);
    
    // Should handle empty device ID gracefully
    QStringList ids = dataManager->getDeviceIds();
    // Empty string should not be used as device ID
    EXPECT_FALSE(ids.contains(""));
}

TEST_F(DataManagerTest, VeryLargeValues) {
    QVariantMap data;
    data["deviceId"] = "test-001";
    data["temperature"] = 1e308; // Max double
    data["humidity"] = -1e308;   // Min double
    data["isValid"] = true;
    
    dataManager->handleDeviceData("test-001", data);
    QTest::qWait(100);
    
    QVariantMap info = dataManager->getDeviceInfo("test-001");
    EXPECT_TRUE(info["isValid"].toBool());
}

TEST_F(DataManagerTest, MissingDataFields) {
    QVariantMap data;
    data["deviceId"] = "test-001";
    // Missing temperature, humidity, etc.
    
    dataManager->handleDeviceData("test-001", data);
    QTest::qWait(100);
    
    // Should handle missing fields gracefully
    QVariantMap current = dataManager->currentData();
    EXPECT_FALSE(current.contains("temperature"));
}

TEST_F(DataManagerTest, RapidUpdateSequence) {
    QSignalSpy spy(dataManager, &DataManager::dataUpdated);
    
    // Send rapid updates
    for (int i = 0; i < 100; ++i) {
        QVariantMap data = generateTestData("test-001", 20.0 + i * 0.01);
        dataManager->handleDeviceData("test-001", data);
    }
    QTest::qWait(200);
    
    // Should receive all updates (might be batched)
    EXPECT_GT(spy.count(), 0);
    
    // Check final value
    QVariantMap current = dataManager->currentData();
    EXPECT_NEAR(current["temperature"].toDouble(), 20.99, 0.01);
}

// ============================================================================
// Memory Tests
// ============================================================================

TEST_F(DataManagerTest, MemoryUsage) {
    // Add a lot of data and check memory doesn't grow indefinitely
    const int BATCHES = 10;
    const int POINTS_PER_BATCH = 100;
    
    for (int batch = 0; batch < BATCHES; ++batch) {
        for (int i = 0; i < POINTS_PER_BATCH; ++i) {
            QVariantMap data = generateTestData("mem-001", 20.0 + i);
            dataManager->handleDeviceData("mem-001", data);
        }
        QTest::qWait(50);
        
        // History should be capped at MAX_HISTORY_SIZE
        QVariantList history = dataManager->getHistory("mem-001", DataManager::MAX_HISTORY_SIZE + 10);
        EXPECT_LE(history.size(), DataManager::MAX_HISTORY_SIZE);
    }
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(DataManagerTest, IntegrationWithDeviceManager) {
    // Simulate integration with device manager
    // This would normally connect to DeviceManager signals
    
    // Create mock device data
    QVariantMap deviceData = generateTestData("integrated-001", 30.0, 80.0, 1025.0);
    deviceData["deviceType"] = "sensor";
    deviceData["status"] = "online";
    
    // Process as if from device manager
    dataManager->handleDeviceData("integrated-001", deviceData);
    dataManager->handleDeviceStatus("integrated-001", 0); // OK status
    
    QTest::qWait(100);
    
    // Verify integration
    QVariantMap info = dataManager->getDeviceInfo("integrated-001");
    EXPECT_EQ(info["deviceId"].toString(), "integrated-001");
    EXPECT_EQ(info["type"].toString(), "temperature");
    
    QVariantMap status = dataManager->deviceStatus();
    EXPECT_TRUE(status.contains("integrated-001"));
    EXPECT_EQ(status["integrated-001"].toInt(), 0);
}

TEST_F(DataManagerTest, DataFlowThroughSystem) {
    // Simulate complete data flow: sensor -> data manager -> UI
    
    QSignalSpy uiUpdateSpy(dataManager, &DataManager::dataUpdated);
    QSignalSpy sensorSpy(dataManager, &DataManager::sensorDataReceived);
    
    // Data arrives from sensor
    QVariantMap sensorData = generateTestData("flow-001", 27.5, 62.0, 1018.0);
    dataManager->handleDeviceData("flow-001", sensorData);
    
    QTest::qWait(100);
    
    // Check that data propagated correctly
    EXPECT_GT(sensorSpy.count(), 0);
    EXPECT_GT(uiUpdateSpy.count(), 0);
    
    // UI should have latest data
    QVariantMap current = dataManager->currentData();
    EXPECT_FALSE(current.isEmpty());
}

#endif // DATA_MANAGER_TEST_CPP
