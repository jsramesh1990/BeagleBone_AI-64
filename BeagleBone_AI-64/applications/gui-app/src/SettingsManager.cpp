#include "gui-app/SettingsManager.h"
#include "logger/Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

using namespace common;

// ============================================================================
// SettingsManager Implementation
// ============================================================================

SettingsManager::SettingsManager(QObject* parent) 
    : QObject(parent),
      m_refreshInterval(1000),
      m_darkTheme(false),
      m_autoStart(true),
      m_showNotifications(true),
      m_soundEnabled(true) {
    
    // Set default settings file path
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    m_settingsFile = configDir + "/gui-app/settings.json";
    
    LOG_DEBUG("SettingsManager created, config file: " + m_settingsFile.toStdString());
}

SettingsManager::~SettingsManager() {
    save();
    LOG_DEBUG("SettingsManager destroyed");
}

// ============================================================================
// Properties
// ============================================================================

void SettingsManager::setRefreshInterval(int interval) {
    if (m_refreshInterval != interval && interval >= 100) {
        m_refreshInterval = interval;
        emit refreshIntervalChanged();
        setDirty(true);
    }
}

void SettingsManager::setDarkTheme(bool dark) {
    if (m_darkTheme != dark) {
        m_darkTheme = dark;
        emit darkThemeChanged();
        emit themeChanged(dark);
        setDirty(true);
    }
}

void SettingsManager::setAutoStart(bool autoStart) {
    if (m_autoStart != autoStart) {
        m_autoStart = autoStart;
        emit autoStartChanged();
        setDirty(true);
    }
}

void SettingsManager::setShowNotifications(bool show) {
    if (m_showNotifications != show) {
        m_showNotifications = show;
        emit showNotificationsChanged();
        setDirty(true);
    }
}

void SettingsManager::setSoundEnabled(bool enabled) {
    if (m_soundEnabled != enabled) {
        m_soundEnabled = enabled;
        emit soundEnabledChanged();
        setDirty(true);
    }
}

// ============================================================================
// Public Methods
// ============================================================================

bool SettingsManager::load() {
    QFile file(m_settingsFile);
    if (!file.exists()) {
        LOG_INFO("Settings file does not exist, using defaults");
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Failed to open settings file: " + m_settingsFile.toStdString());
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        LOG_ERROR("Invalid settings file format");
        return false;
    }
    
    QJsonObject json = doc.object();
    
    // Load settings
    m_refreshInterval = json.value("refreshInterval").toInt(1000);
    m_darkTheme = json.value("darkTheme").toBool(false);
    m_autoStart = json.value("autoStart").toBool(true);
    m_showNotifications = json.value("showNotifications").toBool(true);
    m_soundEnabled = json.value("soundEnabled").toBool(true);
    
    // Load custom settings
    if (json.contains("custom")) {
        QJsonObject custom = json.value("custom").toObject();
        for (auto it = custom.begin(); it != custom.end(); ++it) {
            m_customSettings[it.key()] = it.value().toVariant();
        }
    }
    
    // Load server URL
    m_serverUrl = json.value("serverUrl").toString("ws://localhost:8080/ws");
    
    LOG_INFO("Settings loaded from: " + m_settingsFile.toStdString());
    m_dirty = false;
    
    // Emit signals for changed properties
    emit refreshIntervalChanged();
    emit darkThemeChanged();
    emit autoStartChanged();
    emit showNotificationsChanged();
    emit soundEnabledChanged();
    emit serverUrlChanged();
    
    return true;
}

bool SettingsManager::save() {
    if (!m_dirty) {
        return true;
    }
    
    // Create directory if it doesn't exist
    QFileInfo fileInfo(m_settingsFile);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QJsonObject json;
    json["refreshInterval"] = m_refreshInterval;
    json["darkTheme"] = m_darkTheme;
    json["autoStart"] = m_autoStart;
    json["showNotifications"] = m_showNotifications;
    json["soundEnabled"] = m_soundEnabled;
    json["serverUrl"] = m_serverUrl;
    
    // Save custom settings
    QJsonObject custom;
    for (auto it = m_customSettings.begin(); it != m_customSettings.end(); ++it) {
        custom[it.key()] = QJsonValue::fromVariant(it.value());
    }
    json["custom"] = custom;
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    QFile file(m_settingsFile);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("Failed to open settings file for writing: " + m_settingsFile.toStdString());
        return false;
    }
    
    file.write(data);
    file.close();
    
    m_dirty = false;
    LOG_INFO("Settings saved to: " + m_settingsFile.toStdString());
    return true;
}

void SettingsManager::resetToDefaults() {
    m_refreshInterval = 1000;
    m_darkTheme = false;
    m_autoStart = true;
    m_showNotifications = true;
    m_soundEnabled = true;
    m_serverUrl = "ws://localhost:8080/ws";
    m_customSettings.clear();
    
    emit refreshIntervalChanged();
    emit darkThemeChanged();
    emit autoStartChanged();
    emit showNotificationsChanged();
    emit soundEnabledChanged();
    emit serverUrlChanged();
    emit themeChanged(m_darkTheme);
    
    setDirty(true);
    save();
    
    LOG_INFO("Settings reset to defaults");
}

QVariant SettingsManager::getCustomSetting(const QString& key, const QVariant& defaultValue) const {
    return m_customSettings.value(key, defaultValue);
}

void SettingsManager::setCustomSetting(const QString& key, const QVariant& value) {
    if (m_customSettings.value(key) != value) {
        m_customSettings[key] = value;
        emit customSettingChanged(key, value);
        setDirty(true);
    }
}

// ============================================================================
// Private Methods
// ============================================================================

void SettingsManager::setDirty(bool dirty) {
    if (m_dirty != dirty) {
        m_dirty = dirty;
        if (dirty) {
            // Auto-save after a delay
            QTimer::singleShot(5000, this, &SettingsManager::save);
        }
    }
}
