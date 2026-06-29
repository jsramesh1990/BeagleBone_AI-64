#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QDateTime>
#include <QVariantMap>
#include <QMutex>
#include <memory>
#include <vector>

namespace common { namespace utils { class ThreadPool; } }

class DataManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap currentData READ currentData NOTIFY dataUpdated)
    Q_PROPERTY(QVariantMap deviceStatus READ deviceStatus NOTIFY statusUpdated)
    Q_PROPERTY(int dataPoints READ dataPoints NOTIFY dataPointsChanged)

public:
    struct SensorData {
        QString deviceId;
        QString type;
        QDateTime timestamp;
        QVariantMap values;
        bool isValid;
    };

    explicit DataManager(QObject* parent = nullptr);
    ~DataManager();

    // Properties
    QVariantMap currentData() const { return m_currentData; }
    QVariantMap deviceStatus() const { return m_deviceStatus; }
    int dataPoints() const { return m_dataPoints; }

    Q_INVOKABLE void startUpdates();
    Q_INVOKABLE void stopUpdates();
    Q_INVOKABLE void refreshNow();
    Q_INVOKABLE QVariantList getHistory(const QString& deviceId, int count = 100);
    Q_INVOKABLE QVariantMap getDeviceInfo(const QString& deviceId);
    Q_INVOKABLE QStringList getDeviceIds() const;
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE void saveData(const QString& filename);
    Q_INVOKABLE void loadData(const QString& filename);

signals:
    void dataUpdated(const QVariantMap& data);
    void statusUpdated(const QVariantMap& status);
    void dataPointsChanged();
    void error(const QString& message);
    void deviceAdded(const QString& deviceId);
    void deviceRemoved(const QString& deviceId);
    void sensorDataReceived(const QString& deviceId, const QVariantMap& data);

public slots:
    void updateData();
    void handleDeviceData(const QString& deviceId, const QVariantMap& data);
    void handleDeviceStatus(const QString& deviceId, int status);

private:
    void processSensorData(const SensorData& data);
    void updateCurrentData();
    void checkTimeouts();
    void saveToCache(const SensorData& data);

    QVariantMap m_currentData;
    QVariantMap m_deviceStatus;
    QMap<QString, std::vector<SensorData>> m_history;
    QMap<QString, QDateTime> m_lastUpdate;
    QTimer m_updateTimer;
    int m_dataPoints;
    mutable QMutex m_mutex;
    std::unique_ptr<common::utils::ThreadPool> m_threadPool;

    static const int MAX_HISTORY_SIZE = 10000;
    static const int TIMEOUT_MS = 30000;
};

#endif // DATA_MANAGER_H
