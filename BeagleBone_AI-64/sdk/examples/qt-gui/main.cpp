// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    
    // Set application info
    app.setOrganizationName("BBB-AI-64");
    app.setApplicationName("Dashboard");
    app.setApplicationVersion("1.0.0");
    
    // Set style
    QQuickStyle::setStyle("Material");
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Load main QML
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}
