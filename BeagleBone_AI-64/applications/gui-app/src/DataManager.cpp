#include "gui-app/DataManager.h"
#include "logger/Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>

using namespace common;

DataManager::DataManager(QObject* parent) 
    : QObject(parent),
      m_dataPoints(0) {
    
    m_updateTimer.setInterval(1000);
    connect(&m_updateTimer, &QTimer::timeout, this, &DataManager::updateData);
    
    // Initialize thread pool
    m_threadPool = std::make_unique<common::utils::ThreadPool>(2);
}

DataManager::~DataManager() {
    stopUpdates();
}

void DataManager::startUpdates() {
    m_updateTimer.start();
    LOG_INFO("DataManager: Starting updates");
}

void DataManager::stopUpdates() {
    m_updateTimer.stop();
    LOG_INFO("DataManager: Stopping updates");
}

void DataManager::updateData() {
    // This would normally fetch data from device manager or sensor service
    // For now, we simulate data
    
    QVariantMap data;
    data["temperature"] = 22.5 + (qrand() % 100) / 50.0;
    data["humidity"] = 45 + (qrand() % 20);
    data["pressure"] = 1013 + (qrand() % 10);
    data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Update current data
    m_currentData = data;
    emit dataUpdated(m_currentData);
    
    // Process in background
    m_threadPool->submit([this, data]() {
        // Process data in background thread
        SensorData sensorData;
        sensorData.deviceId = "sensor-001";
        sensorData.type = "temperature";
        sensorData.timestamp = QDateTime::currentDateTime();
        sensorData.values = data;
        sensorData.isValid = true;
        
        processSensorData(sensorData);
    });
}

void DataManager::processSensorData(const SensorData& data) {
    QMutexLocker locker(&m_mutex);
    
    // Store in history
    auto& history = m_history[data.deviceId];
    history.push_back(data);
    
    // Limit history size
    if (history.size() > MAX_HISTORY_SIZE) {
        history.erase(history.begin(), history.begin() + (history.size() - MAX_HISTORY_SIZE));
    }
    
    m_dataPoints = history.size();
    emit dataPointsChanged();
    
    // Update last update time
    m_lastUpdate[data.deviceId] = QDateTime::currentDateTime();
}

void DataManager::handleDeviceData(const QString& deviceId, const QVariantMap& data) {
    SensorData sensorData;
    sensorData.deviceId = deviceId;
    sensorData.type = data.value("type", "unknown").toString();
    sensorData.timestamp = QDateTime::currentDateTime();
    sensorData.values = data;
    sensorData.isValid = data.value("isValid", true).toBool();
    
    processSensorData(sensorData);
    emit sensorDataReceived(deviceId, data);
}

void DataManager::handleDeviceStatus(const QString& deviceId, int status) {
    m_deviceStatus[deviceId] = status;
    emit statusUpdated(m_deviceStatus);
}

QVariantList DataManager::getHistory(const QString& deviceId, int count) {
    QMutexLocker locker(&m_mutex);
    
    QVariantList result;
    auto it = m_history.find(deviceId);
    if (it != m_history.end()) {
        const auto& history = it.value();
        int start = std::max(0, static_cast<int>(history.size()) - count);
        for (int i = start; i < history.size(); ++i) {
            QVariantMap item;
            item["timestamp"] = history[i].timestamp;
            item["values"] = history[i].values;
            result.append(item);
        }
    }
    return result;
}

QVariantMap DataManager::getDeviceInfo(const QString& deviceId) {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap info;
    auto it = m_history.find(deviceId);
    if (it != m_history.end() && !it.value().isEmpty()) {
        const auto& lastData = it.value().back();
        info["deviceId"] = deviceId;
        info["type"] = lastData.type;
        info["lastUpdate"] = lastData.timestamp;
        info["isValid"] = lastData.isValid;
        info["dataPoints"] = static_cast<int>(it.value().size());
    }
    return info;
}

QStringList DataManager::getDeviceIds() const {
    QMutexLocker locker(&m_mutex);
    return m_history.keys();
}

void DataManager::clearHistory() {
    QMutexLocker locker(&m_mutex);
    m_history.clear();
    m_dataPoints = 0;
    emit dataPointsChanged();
}

void DataManager::saveData(const QString& filename) {
    QMutexLocker locker(&m_mutex);
    
    QJsonObject root;
    for (auto it = m_history.begin(); it != m_history.end(); ++it) {
        QJsonArray deviceData;
        for (const auto& data : it.value()) {
            QJsonObject item;
            item["timestamp"] = data.timestamp.toString(Qt::ISODate);
            item["type"] = data.type;
            item["isValid"] = data.isValid;
            
            QJsonObject values;
            for (auto vit = data.values.begin(); vit != data.values.end(); ++vit) {
                values[vit.key()] = QJsonValue::fromVariant(vit.value());
            }
            item["values"] = values;
            deviceData.append(item);
        }
        root[it.key()] = deviceData;
    }
    
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << QJsonDocument(root).toJson(QJsonDocument::Indented);
        file.close();
        LOG_INFO("Data saved to: " + filename.toStdString());
    } else {
        LOG_ERROR("Failed to save data to: " + filename.toStdString());
        emit error("Failed to save data");
    }
}

void DataManager::loadData(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Failed to load data from: " + filename.toStdString());
        emit error("Failed to load data");
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit error("Invalid data format");
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    m_history.clear();
    
    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        QString deviceId = it.key();
        QJsonArray deviceData = it.value().toArray();
        
        for (const auto& item : deviceData) {
            QJsonObject obj = item.toObject();
            SensorData sensorData;
            sensorData.deviceId = deviceId;
            sensorData.type = obj["type"].toString("unknown");
            sensorData.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
            sensorData.isValid = obj["isValid"].toBool(true);
            
            QJsonObject values = obj["values"].toObject();
            for (auto vit = values.begin(); vit != values.end(); ++vit) {
                sensorData.values[vit.key()] = vit.value().toVariant();
            }
            
            m_history[deviceId].push_back(sensorData);
        }
    }
    
    m_dataPoints = m_history.size();
    emit dataPointsChanged();
    LOG_INFO("Data loaded from: " + filename.toStdString());
}
