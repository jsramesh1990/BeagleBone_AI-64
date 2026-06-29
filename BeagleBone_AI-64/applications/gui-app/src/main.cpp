#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QDebug>

#include "gui-app/Dashboard.h"
#include "gui-app/DataManager.h"
#include "gui-app/SensorClient.h"
#include "gui-app/DeviceController.h"
#include "gui-app/ChartModel.h"
#include "gui-app/SettingsManager.h"
#include "gui-app/ThemeManager.h"

#include "logger/Logger.h"
#include "config/ConfigManager.h"
#include "ipc/IpcManager.h"

using namespace common;

int main(int argc, char *argv[]) {
    // Initialize logger
    auto& logger = Logger::getInstance();
    logger.initialize();
    logger.info("GUI Application starting");

    // Initialize configuration
    auto& config = ConfigManager::getInstance();
    config.loadConfig("/etc/gui-app/config.json");

    // Initialize IPC
    auto& ipc = IpcManager::getInstance();
    ipc.initialize(IpcManager::TransportType::UNIX_SOCKET, "/tmp/gui-app.sock");
    ipc.start();

    // Set up QML
    QGuiApplication app(argc, argv);
    app.setOrganizationName("BBB-AI64");
    app.setApplicationName("Dashboard");
    app.setApplicationVersion("1.0.0");

    // Load fonts
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Bold.ttf");

    // Set style
    QQuickStyle::setStyle("Material");

    // Register QML types
    qmlRegisterType<Dashboard>("Dashboard", 1, 0, "Dashboard");
    qmlRegisterType<DataManager>("Dashboard", 1, 0, "DataManager");
    qmlRegisterType<SensorClient>("Dashboard", 1, 0, "SensorClient");
    qmlRegisterType<DeviceController>("Dashboard", 1, 0, "DeviceController");
    qmlRegisterType<ChartModel>("Dashboard", 1, 0, "ChartModel");
    qmlRegisterType<SettingsManager>("Dashboard", 1, 0, "SettingsManager");
    qmlRegisterType<ThemeManager>("Dashboard", 1, 0, "ThemeManager");

    // Create main components
    Dashboard dashboard;

    // Set up QML engine
    QQmlApplicationEngine engine;
    
    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("dashboard", &dashboard);
    engine.rootContext()->setContextProperty("dataManager", dashboard.dataManager());
    engine.rootContext()->setContextProperty("deviceController", dashboard.deviceController());
    engine.rootContext()->setContextProperty("settingsManager", dashboard.settingsManager());
    engine.rootContext()->setContextProperty("themeManager", dashboard.themeManager());

    // Load main QML
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            logger.fatal("Failed to load QML: " + url.toString().toStdString());
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);

    // Start data updates
    dashboard.dataManager()->startUpdates();

    logger.info("GUI Application started successfully");

    int result = app.exec();

    // Cleanup
    dashboard.dataManager()->stopUpdates();
    ipc.stop();
    logger.info("GUI Application exiting");

    return result;
}
