#include "gui-app/Dashboard.h"
#include "logger/Logger.h"

using namespace common;

Dashboard::Dashboard(QObject* parent) 
    : QObject(parent),
      m_title("Dashboard"),
      m_fullScreen(false),
      m_currentView("dashboard"),
      m_refreshInterval(1000) {
    
    // Initialize components
    m_dataManager = std::make_unique<DataManager>(this);
    m_deviceController = std::make_unique<DeviceController>(this);
    m_settingsManager = std::make_unique<SettingsManager>(this);
    m_themeManager = std::make_unique<ThemeManager>(this);

    // Load settings
    m_settingsManager->load();

    // Setup connections
    setupConnections();

    // Create QML engine
    m_engine = std::make_unique<QQmlApplicationEngine>();
    
    // Register components with QML
    qmlRegisterType<DataManager>("Dashboard", 1, 0, "DataManager");
    qmlRegisterType<DeviceController>("Dashboard", 1, 0, "DeviceController");
    
    // Load QML
    loadQml();
}

Dashboard::~Dashboard() {
    if (m_engine) {
        m_engine->deleteLater();
    }
}

void Dashboard::setupConnections() {
    // Connect data manager signals
    connect(m_dataManager.get(), &DataManager::dataUpdated,
            this, &Dashboard::onDataUpdated);
    
    connect(m_dataManager.get(), &DataManager::error,
            this, &Dashboard::onError);

    // Connect device controller signals
    connect(m_deviceController.get(), &DeviceController::deviceStatusChanged,
            this, &Dashboard::onDeviceStatusChanged);

    // Connect settings manager
    connect(m_settingsManager.get(), &SettingsManager::themeChanged,
            this, &Dashboard::themeChanged);
}

void Dashboard::loadQml() {
    // In production, the engine is created in main.cpp
    // This method is for dynamic loading if needed
}

void Dashboard::setTitle(const QString& title) {
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void Dashboard::setFullScreen(bool fullScreen) {
    if (m_fullScreen != fullScreen) {
        m_fullScreen = fullScreen;
        emit fullScreenChanged();
    }
}

void Dashboard::setCurrentView(const QString& view) {
    if (m_currentView != view) {
        m_currentView = view;
        emit currentViewChanged();
        emit viewChanged(view);
    }
}

void Dashboard::setRefreshInterval(int interval) {
    if (m_refreshInterval != interval) {
        m_refreshInterval = interval;
        emit refreshIntervalChanged();
    }
}

void Dashboard::showView(const QString& view) {
    setCurrentView(view);
}

void Dashboard::toggleFullScreen() {
    setFullScreen(!m_fullScreen);
}

void Dashboard::toggleTheme() {
    m_themeManager->toggleTheme();
    emit themeChanged(m_themeManager->isDarkTheme());
}

void Dashboard::exitApplication() {
    emit applicationExit();
    QCoreApplication::exit(0);
}

void Dashboard::onDataUpdated() {
    // Handle data updates
}

void Dashboard::onDeviceStatusChanged(const QString& deviceId, int status) {
    // Handle device status changes
}

void Dashboard::onError(const QString& error) {
    LOG_ERROR("Dashboard error: " + error.toStdString());
}
