#include "gui-app/ThemeManager.h"
#include "logger/Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickStyle>

using namespace common;

// ============================================================================
// ThemeManager Implementation
// ============================================================================

ThemeManager::ThemeManager(QObject* parent) 
    : QObject(parent),
      m_darkTheme(false),
      m_primaryColor("#00b4d8"),
      m_accentColor("#ff6b6b"),
      m_successColor("#4ecdc4"),
      m_warningColor("#ffd93d"),
      m_backgroundColor("#f5f5f5"),
      m_surfaceColor("#ffffff"),
      m_textColor("#333333"),
      m_secondaryTextColor("#666666") {
    
    // Set default theme files
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    m_themeFile = configDir + "/gui-app/theme.json";
    
    LOG_DEBUG("ThemeManager created");
}

ThemeManager::~ThemeManager() {
    saveTheme();
    LOG_DEBUG("ThemeManager destroyed");
}

// ============================================================================
// Properties
// ============================================================================

void ThemeManager::setDarkTheme(bool dark) {
    if (m_darkTheme != dark) {
        m_darkTheme = dark;
        applyTheme();
        emit darkThemeChanged();
        emit themeChanged(dark);
        
        LOG_DEBUG("Theme changed to " + QString(dark ? "dark" : "light").toStdString());
    }
}

// ============================================================================
// Public Methods
// ============================================================================

void ThemeManager::initialize() {
    loadTheme();
    applyTheme();
    LOG_INFO("ThemeManager initialized");
}

void ThemeManager::toggleTheme() {
    setDarkTheme(!m_darkTheme);
}

void ThemeManager::applyTheme() {
    // Apply theme colors
    if (m_darkTheme) {
        m_backgroundColor = "#1a1a2e";
        m_surfaceColor = "#16213e";
        m_textColor = "#ffffff";
        m_secondaryTextColor = "#aaaaaa";
    } else {
        m_backgroundColor = "#f5f5f5";
        m_surfaceColor = "#ffffff";
        m_textColor = "#333333";
        m_secondaryTextColor = "#666666";
    }
    
    // Emit color changes
    emit backgroundColorChanged();
    emit surfaceColorChanged();
    emit textColorChanged();
    emit secondaryTextColorChanged();
    emit themeApplied();
    
    // Apply to QML styles
    applyQMLTheme();
}

void ThemeManager::applyQMLTheme() {
    // Apply material style colors
    if (m_darkTheme) {
        QQuickStyle::setStyle("Material");
        // Material dark theme
        QVariantMap material;
        material["theme"] = "Dark";
        // Set in QML context
    } else {
        QQuickStyle::setStyle("Material");
        // Material light theme
        QVariantMap material;
        material["theme"] = "Light";
        // Set in QML context
    }
}

bool ThemeManager::loadTheme() {
    QFile file(m_themeFile);
    if (!file.exists()) {
        LOG_INFO("Theme file does not exist, using defaults");
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Failed to open theme file: " + m_themeFile.toStdString());
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        LOG_ERROR("Invalid theme file format");
        return false;
    }
    
    QJsonObject json = doc.object();
    
    // Load theme settings
    m_darkTheme = json.value("darkTheme").toBool(false);
    m_primaryColor = json.value("primaryColor").toString("#00b4d8");
    m_accentColor = json.value("accentColor").toString("#ff6b6b");
    m_successColor = json.value("successColor").toString("#4ecdc4");
    m_warningColor = json.value("warningColor").toString("#ffd93d");
    
    // Load custom colors
    if (json.contains("customColors")) {
        QJsonObject colors = json.value("customColors").toObject();
        for (auto it = colors.begin(); it != colors.end(); ++it) {
            m_customColors[it.key()] = it.value().toString();
        }
    }
    
    LOG_INFO("Theme loaded from: " + m_themeFile.toStdString());
    return true;
}

bool ThemeManager::saveTheme() {
    QJsonObject json;
    json["darkTheme"] = m_darkTheme;
    json["primaryColor"] = m_primaryColor;
    json["accentColor"] = m_accentColor;
    json["successColor"] = m_successColor;
    json["warningColor"] = m_warningColor;
    
    // Save custom colors
    QJsonObject colors;
    for (auto it = m_customColors.begin(); it != m_customColors.end(); ++it) {
        colors[it.key()] = it.value();
    }
    json["customColors"] = colors;
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    // Create directory if it doesn't exist
    QFileInfo fileInfo(m_themeFile);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(m_themeFile);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("Failed to open theme file for writing: " + m_themeFile.toStdString());
        return false;
    }
    
    file.write(data);
    file.close();
    
    LOG_INFO("Theme saved to: " + m_themeFile.toStdString());
    return true;
}

void ThemeManager::resetToDefaults() {
    m_darkTheme = false;
    m_primaryColor = "#00b4d8";
    m_accentColor = "#ff6b6b";
    m_successColor = "#4ecdc4";
    m_warningColor = "#ffd93d";
    m_customColors.clear();
    
    applyTheme();
    saveTheme();
    
    LOG_INFO("Theme reset to defaults");
}

QString ThemeManager::getCustomColor(const QString& name, const QString& defaultValue) const {
    return m_customColors.value(name, defaultValue);
}

void ThemeManager::setCustomColor(const QString& name, const QString& color) {
    if (m_customColors.value(name) != color) {
        m_customColors[name] = color;
        emit customColorChanged(name, color);
        saveTheme();
    }
}
