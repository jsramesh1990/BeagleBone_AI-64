#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <memory>
#include "DataManager.h"
#include "DeviceController.h"
#include "SettingsManager.h"
#include "ThemeManager.h"

class Dashboard : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool fullScreen READ fullScreen WRITE setFullScreen NOTIFY fullScreenChanged)
    Q_PROPERTY(QString currentView READ currentView WRITE setCurrentView NOTIFY currentViewChanged)
    Q_PROPERTY(int refreshInterval READ refreshInterval WRITE setRefreshInterval NOTIFY refreshIntervalChanged)
    Q_PROPERTY(bool darkTheme READ darkTheme NOTIFY darkThemeChanged)
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    explicit Dashboard(QObject* parent = nullptr);
    ~Dashboard();

    // Properties
    QString title() const { return m_title; }
    void setTitle(const QString& title);

    bool fullScreen() const { return m_fullScreen; }
    void setFullScreen(bool fullScreen);

    QString currentView() const { return m_currentView; }
    void setCurrentView(const QString& view);

    int refreshInterval() const { return m_refreshInterval; }
    void setRefreshInterval(int interval);
    
    bool darkTheme() const { return m_themeManager ? m_themeManager->darkTheme() : false; }
    QString version() const { return "1.0.0"; }

    // Core components
    DataManager* dataManager() { return m_dataManager.get(); }
    DeviceController* deviceController() { return m_deviceController.get(); }
    SettingsManager* settingsManager() { return m_settingsManager.get(); }
    ThemeManager* themeManager() { return m_themeManager.get(); }

    Q_INVOKABLE void showView(const QString& view);
    Q_INVOKABLE void toggleFullScreen();
    Q_INVOKABLE void toggleTheme();
    Q_INVOKABLE void exitApplication();
    Q_INVOKABLE void refreshData();
    Q_INVOKABLE void saveState();
    Q_INVOKABLE void loadState();

signals:
    void titleChanged();
    void fullScreenChanged();
    void currentViewChanged();
    void refreshIntervalChanged();
    void darkThemeChanged();
    void viewChanged(const QString& view);
    void themeChanged(bool dark);
    void applicationExit();
    void stateSaved();
    void stateLoaded();
    void error(const QString& message);

private slots:
    void onDataUpdated();
    void onDeviceStatusChanged(const QString& deviceId, int status);
    void onError(const QString& error);
    void onSettingsChanged();
    void onThemeChanged(bool dark);

private:
    void setupConnections();
    void loadQml();
    void updateStatusBar();

    QString m_title;
    bool m_fullScreen;
    QString m_currentView;
    int m_refreshInterval;

    std::unique_ptr<QQmlApplicationEngine> m_engine;
    std::unique_ptr<DataManager> m_dataManager;
    std::unique_ptr<DeviceController> m_deviceController;
    std::unique_ptr<SettingsManager> m_settingsManager;
    std::unique_ptr<ThemeManager> m_themeManager;
};

#endif // DASHBOARD_H
