#include "gui-app/DeviceController.h"
#include "device-manager/DeviceManager.h"
#include "device-manager/DeviceTypes.h"
#include "logger/Logger.h"
#include <QTimer>
#include <QDateTime>

using namespace common;
using namespace device_manager;

// ============================================================================
// DeviceController Implementation
// ============================================================================

DeviceController::DeviceController(QObject* parent) 
    : QObject(parent),
      m_isScanning(false) {
    
    m_scanTimer.setInterval(30000); // Scan every 30 seconds
    connect(&m_scanTimer, &QTimer::timeout, this, &DeviceController::onScanTimer);
    
    // Connect to DeviceManager signals
    auto& dm = DeviceManager::getInstance();
    // In a real implementation, we would connect to signals here
    
    LOG_DEBUG("DeviceController created");
}

DeviceController::~DeviceController() {
    m_scanTimer.stop();
    LOG_DEBUG("DeviceController destroyed");
}

// ============================================================================
// Public Methods
// ============================================================================

bool DeviceController::enableDevice(const QString& deviceId) {
    auto& dm = DeviceManager::getInstance();
    Device* device = dm.getDevice(deviceId.toStdString());
    
    if (!device) {
        LOG_ERROR("Device not found: " + deviceId.toStdString());
        return false;
    }
    
    bool result = device->start();
    if (result) {
        LOG_INFO("Device enabled: " + deviceId.toStdString());
        updateDeviceInfo(deviceId);
    }
    
    return result;
}

bool DeviceController::disableDevice(const QString& deviceId) {
    auto& dm = DeviceManager::getInstance();
    Device* device = dm.getDevice(deviceId.toStdString());
    
    if (!device) {
        LOG_ERROR("Device not found: " + deviceId.toStdString());
        return false;
    }
    
    bool result = device->stop();
    if (result) {
        LOG_INFO("Device disabled: " + deviceId.toStdString());
        updateDeviceInfo(deviceId);
    }
    
    return result;
}

bool DeviceController::calibrateDevice(const QString& deviceId, const QVariantMap& params) {
    auto& dm = DeviceManager::getInstance();
    Device* device = dm.getDevice(deviceId.toStdString());
    
    if (!device) {
        LOG_ERROR("Device not found: " + deviceId.toStdString());
        return false;
    }
    
    Json::Value jsonParams;
    // Convert QVariantMap to Json::Value
    for (auto it = params.begin(); it != params.end(); ++it) {
        jsonParams[it.key().toStdString()] = it.value().toString().toStdString();
    }
    
    Json::Value result;
    bool success = device->calibrate(jsonParams, result, 10000);
    
    if (success) {
        LOG_INFO("Device calibrated: " + deviceId.toStdString());
        emit deviceCalibrationComplete(deviceId, true);
    } else {
        LOG_ERROR("Device calibration failed: " + deviceId.toStdString());
        emit deviceCalibrationComplete(deviceId, false);
    }
    
    return success;
}

bool DeviceController::resetDevice(const QString& deviceId) {
    auto& dm = DeviceManager::getInstance();
    Device* device = dm.getDevice(deviceId.toStdString());
    
    if (!device) {
        LOG_ERROR("Device not found: " + deviceId.toStdString());
        return false;
    }
    
    bool result = device->reset();
    if (result) {
        LOG_INFO("Device reset: " + deviceId.toStdString());
        updateDeviceInfo(deviceId);
    }
    
    return result;
}

bool DeviceController::selfTestDevice(const QString& deviceId) {
    auto& dm = DeviceManager::getInstance();
    Device* device = dm.getDevice(deviceId.toStdString());
    
    if (!device) {
        LOG_ERROR("Device not found: " + deviceId.toStdString());
        return false;
    }
    
    Json::Value result;
    bool success = device->selfTest(result, 10000);
    
    if (success) {
        LOG_INFO("Device self-test passed: " + deviceId.toStdString());
        QVariantMap resultMap;
        // Convert Json::Value to QVariantMap
        resultMap["status"] = QString::fromStdString(result["status"].asString());
        emit deviceSelfTestComplete(deviceId, resultMap);
    } else {
        LOG_ERROR("Device self-test failed: " + deviceId.toStdString());
        QVariantMap resultMap;
        resultMap["status"] = "failed";
        emit deviceSelfTestComplete(deviceId, resultMap);
    }
    
    return success;
}

QVariantMap DeviceController::getDeviceInfo(const QString& deviceId) const {
    auto& dm = DeviceManager::getInstance();
    Device* device = dm.getDevice(deviceId.toStdString());
    
    QVariantMap info;
    if (device) {
        auto deviceInfo = device->getInfo();
        info["id"] = QString::fromStdString(deviceInfo.id);
        info["name"] = QString::fromStdString(deviceInfo.name);
        info["type"] = QString::fromStdString(DeviceFactory::getTypeName(deviceInfo.type));
        info["status"] = static_cast<int>(deviceInfo.status);
        info["enabled"] = deviceInfo.enabled;
        info["manufacturer"] = QString::fromStdString(deviceInfo.manufacturer);
        info["model"] = QString::fromStdString(deviceInfo.model);
        info["version"] = QString::fromStdString(deviceInfo.version);
        info["serialNumber"] = QString::fromStdString(deviceInfo.serialNumber);
    }
    
    return info;
}

QVariantMap DeviceController::getDeviceStatus(const QString& deviceId) const {
    QVariantMap status;
    
    auto it = m_devices.find(deviceId);
    if (it != m_devices.end()) {
        status["id"] = deviceId;
        status["name"] = it.value().name;
        status["status"] = it.value().status;
        status["enabled"] = it.value().enabled;
        status["lastSeen"] = it.value().lastSeen;
    }
    
    return status;
}

QVariantList DeviceController::getDeviceList() const {
    QVariantList list;
    
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        QVariantMap info = getDeviceInfo(it.key());
        if (!info.isEmpty()) {
            list.append(info);
        }
    }
    
    return list;
}

void DeviceController::scanForDevices() {
    if (m_isScanning) {
        return;
    }
    
    m_isScanning = true;
    emit scanStarted();
    
    LOG_INFO("Scanning for devices...");
    
    auto& dm = DeviceManager::getInstance();
    
    // In a real implementation, we would trigger device discovery
    // For now, we simulate discovery
    QTimer::singleShot(1000, [this]() {
        // Simulate finding devices
        auto allDevices = dm.getAllDevices();
        
        for (auto* device : allDevices) {
            auto info = device->getInfo();
            QString deviceId = QString::fromStdString(info.id);
            
            if (!m_devices.contains(deviceId)) {
                DeviceInfo devInfo;
                devInfo.id = deviceId;
                devInfo.name = QString::fromStdString(info.name);
                devInfo.type = QString::fromStdString(DeviceFactory::getTypeName(info.type));
                devInfo.status = QString::fromStdString(deviceStatusToString(info.status));
                devInfo.enabled = info.enabled;
                devInfo.lastSeen = QDateTime::currentDateTime();
                
                m_devices[deviceId] = devInfo;
                emit deviceAdded(deviceId, getDeviceInfo(deviceId));
            }
        }
        
        m_isScanning = false;
        emit scanComplete(m_devices.size());
        LOG_INFO("Scan complete: " + QString::number(m_devices.size()) + " devices found");
    });
}

// ============================================================================
// Private Slots
// ============================================================================

void DeviceController::onDeviceAdded(const QString& deviceId, const QVariantMap& info) {
    if (!m_devices.contains(deviceId)) {
        DeviceInfo devInfo;
        devInfo.id = deviceId;
        devInfo.name = info.value("name", "Unknown").toString();
        devInfo.type = info.value("type", "Unknown").toString();
        devInfo.status = "Online";
        devInfo.enabled = true;
        devInfo.lastSeen = QDateTime::currentDateTime();
        
        m_devices[deviceId] = devInfo;
        emit deviceAdded(deviceId, info);
        emit deviceListChanged();
        emit deviceCountChanged();
    }
}

void DeviceController::onDeviceRemoved(const QString& deviceId) {
    if (m_devices.contains(deviceId)) {
        m_devices.remove(deviceId);
        emit deviceRemoved(deviceId);
        emit deviceListChanged();
        emit deviceCountChanged();
    }
}

void DeviceController::onDeviceStatusChanged(const QString& deviceId, int status) {
    if (m_devices.contains(deviceId)) {
        QString statusStr;
        switch (static_cast<DeviceStatus>(status)) {
            case DeviceStatus::OK: statusStr = "Online"; break;
            case DeviceStatus::ERROR: statusStr = "Error"; break;
            case DeviceStatus::WARNING: statusStr = "Warning"; break;
            case DeviceStatus::DISCONNECTED: statusStr = "Disconnected"; break;
            case DeviceStatus::INITIALIZING: statusStr = "Initializing"; break;
            case DeviceStatus::STOPPED: statusStr = "Stopped"; break;
            default: statusStr = "Unknown";
        }
        
        m_devices[deviceId].status = statusStr;
        m_devices[deviceId].lastSeen = QDateTime::currentDateTime();
        emit deviceStatusChanged(deviceId, statusStr);
    }
}

void DeviceController::onScanTimer() {
    scanForDevices();
}

// ============================================================================
// Private Methods
// ============================================================================

void DeviceController::updateDeviceList() {
    auto& dm = DeviceManager::getInstance();
    auto allDevices = dm.getAllDevices();
    
    // Check for new devices
    for (auto* device : allDevices) {
        QString deviceId = QString::fromStdString(device->getId());
        if (!m_devices.contains(deviceId)) {
            onDeviceAdded(deviceId, getDeviceInfo(deviceId));
        }
    }
    
    // Check for removed devices
    QStringList currentIds;
    for (auto* device : allDevices) {
        currentIds.append(QString::fromStdString(device->getId()));
    }
    
    for (auto it = m_devices.begin(); it != m_devices.end(); ) {
        if (!currentIds.contains(it.key())) {
            it = m_devices.erase(it);
            emit deviceRemoved(it.key());
        } else {
            ++it;
        }
    }
    
    emit deviceListChanged();
    emit deviceCountChanged();
}

void DeviceController::updateDeviceInfo(const QString& deviceId) {
    if (m_devices.contains(deviceId)) {
        // Update device info from DeviceManager
        onDeviceStatusChanged(deviceId, 0); // Refresh status
    }
}

void DeviceController::processDeviceData(const QString& deviceId, const QVariantMap& data) {
    // Process incoming device data
    // Update device info if needed
    if (m_devices.contains(deviceId)) {
        m_devices[deviceId].lastSeen = QDateTime::currentDateTime();
    }
}

std::string DeviceController::deviceStatusToString(DeviceStatus status) {
    switch (status) {
        case DeviceStatus::OK: return "Online";
        case DeviceStatus::ERROR: return "Error";
        case DeviceStatus::WARNING: return "Warning";
        case DeviceStatus::DISCONNECTED: return "Disconnected";
        case DeviceStatus::INITIALIZING: return "Initializing";
        case DeviceStatus::STOPPED: return "Stopped";
        default: return "Unknown";
    }
}
