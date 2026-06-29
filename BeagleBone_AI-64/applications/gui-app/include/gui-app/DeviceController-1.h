#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QDateTime>
#include <memory>

class DeviceController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int deviceCount READ deviceCount NOTIFY deviceCountChanged)
    Q_PROPERTY(QStringList deviceIds READ deviceIds NOTIFY deviceListChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY scanningChanged)

public:
    struct DeviceInfo {
        QString id;
        QString name;
        QString type;
        QString status;
        bool enabled;
        QVariantMap properties;
        QDateTime lastSeen;
        QString manufacturer;
        QString model;
        QString version;
        QString serialNumber;
        
        DeviceInfo() : enabled(true) {}
    };

    explicit DeviceController(QObject* parent = nullptr);
    ~DeviceController();

    // Properties
    int deviceCount() const { return m_devices.size(); }
    QStringList deviceIds() const { return m_devices.keys(); }
    bool isScanning() const { return m_isScanning; }

    Q_INVOKABLE bool enableDevice(const QString& deviceId);
    Q_INVOKABLE bool disableDevice(const QString& deviceId);
    Q_INVOKABLE bool calibrateDevice(const QString& deviceId, 
                                     const QVariantMap& params = QVariantMap());
    Q_INVOKABLE bool resetDevice(const QString& deviceId);
    Q_INVOKABLE bool selfTestDevice(const QString& deviceId);
    Q_INVOKABLE QVariantMap getDeviceInfo(const QString& deviceId) const;
    Q_INVOKABLE QVariantMap getDeviceStatus(const QString& deviceId) const;
    Q_INVOKABLE QVariantList getDeviceList() const;
    Q_INVOKABLE void scanForDevices();
    Q_INVOKABLE void setScanInterval(int seconds) { m_scanTimer.setInterval(seconds * 1000); }
    Q_INVOKABLE int getScanInterval() const { return m_scanTimer.interval() / 1000; }

signals:
    void deviceCountChanged();
    void deviceListChanged();
    void scanningChanged();
    void deviceAdded(const QString& deviceId, const QVariantMap& info);
    void deviceRemoved(const QString& deviceId);
    void deviceStatusChanged(const QString& deviceId, const QString& status);
    void deviceCalibrationComplete(const QString& deviceId, bool success);
    void deviceSelfTestComplete(const QString& deviceId, const QVariantMap& result);
    void scanStarted();
    void scanComplete(int deviceCount);
    void error(const QString& message);

private slots:
    void onDeviceAdded(const QString& deviceId, const QVariantMap& info);
    void onDeviceRemoved(const QString& deviceId);
    void onDeviceStatusChanged(const QString& deviceId, int status);
    void onScanTimer();

private:
    void updateDeviceList();
    void updateDeviceInfo(const QString& deviceId);
    void processDeviceData(const QString& deviceId, const QVariantMap& data);
    std::string deviceStatusToString(int status);

    QMap<QString, DeviceInfo> m_devices;
    QTimer m_scanTimer;
    bool m_isScanning;
};

#endif // DEVICE_CONTROLLER_H
