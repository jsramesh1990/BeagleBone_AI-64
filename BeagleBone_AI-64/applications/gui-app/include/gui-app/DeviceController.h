#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <memory>

class DeviceController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int deviceCount READ deviceCount NOTIFY deviceCountChanged)
    Q_PROPERTY(QStringList deviceIds READ deviceIds NOTIFY deviceListChanged)

public:
    struct DeviceInfo {
        QString id;
        QString name;
        QString type;
        QString status;
        bool enabled;
        QVariantMap properties;
        QDateTime lastSeen;
    };

    explicit DeviceController(QObject* parent = nullptr);
    ~DeviceController();

    // Properties
    int deviceCount() const { return m_devices.size(); }
    QStringList deviceIds() const { return m_devices.keys(); }

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

signals:
    void deviceCountChanged();
    void deviceListChanged();
    void deviceAdded(const QString& deviceId, const QVariantMap& info);
    void deviceRemoved(const QString& deviceId);
    void deviceStatusChanged(const QString& deviceId, const QString& status);
    void deviceCalibrationComplete(const QString& deviceId, bool success);
    void deviceSelfTestComplete(const QString& deviceId, const QVariantMap& result);
    void scanStarted();
    void scanComplete(int deviceCount);

private slots:
    void onDeviceAdded(const QString& deviceId, const QVariantMap& info);
    void onDeviceRemoved(const QString& deviceId);
    void onDeviceStatusChanged(const QString& deviceId, int status);
    void onScanTimer();

private:
    void updateDeviceList();
    void processDeviceData(const QString& deviceId, const QVariantMap& data);

    QMap<QString, DeviceInfo> m_devices;
    QTimer m_scanTimer;
    bool m_isScanning;
};

#endif // DEVICE_CONTROLLER_H
