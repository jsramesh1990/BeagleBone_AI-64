#ifndef SENSOR_CLIENT_H
#define SENSOR_CLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QMap>
#include <QTimer>
#include <QSet>
#include <QDateTime>

class SensorClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    Q_PROPERTY(int reconnectAttempts READ reconnectAttempts NOTIFY reconnectAttemptsChanged)
    Q_PROPERTY(int maxReconnectAttempts READ maxReconnectAttempts WRITE setMaxReconnectAttempts NOTIFY maxReconnectAttemptsChanged)
    Q_PROPERTY(QStringList subscribedDevices READ subscribedDevices NOTIFY subscribedDevicesChanged)

public:
    explicit SensorClient(QObject* parent = nullptr);
    ~SensorClient();

    // Properties
    bool connected() const { return m_connected; }
    QString serverUrl() const { return m_serverUrl; }
    void setServerUrl(const QString& url);
    int reconnectAttempts() const { return m_reconnectAttempts; }
    int maxReconnectAttempts() const { return m_maxReconnectAttempts; }
    void setMaxReconnectAttempts(int attempts) { 
        if (m_maxReconnectAttempts != attempts) {
            m_maxReconnectAttempts = attempts;
            emit maxReconnectAttemptsChanged();
        }
    }
    QStringList subscribedDevices() const { return m_subscriptions.values(); }

    Q_INVOKABLE void connectToServer();
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void sendCommand(const QString& deviceId, const QString& command, 
                                 const QVariantMap& params = QVariantMap());
    Q_INVOKABLE void subscribeToDevice(const QString& deviceId);
    Q_INVOKABLE void unsubscribeFromDevice(const QString& deviceId);
    Q_INVOKABLE bool isSubscribed(const QString& deviceId) const;
    Q_INVOKABLE void setReconnectDelay(int delayMs) { m_reconnectDelay = delayMs; }
    Q_INVOKABLE int getReconnectDelay() const { return m_reconnectDelay; }

signals:
    void connectedChanged();
    void serverUrlChanged();
    void reconnectAttemptsChanged();
    void maxReconnectAttemptsChanged();
    void subscribedDevicesChanged();
    void dataReceived(const QString& deviceId, const QVariantMap& data);
    void statusChanged(const QString& deviceId, int status);
    void error(const QString& message);
    void deviceDiscovered(const QString& deviceId, const QVariantMap& info);
    void connectionError(const QString& error);
    void reconnecting(int attempt, int maxAttempts);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimeout();

private:
    void processMessage(const QJsonObject& json);
    void handleDeviceData(const QVariantMap& data);
    void handleStatusUpdate(const QVariantMap& data);
    void handleDeviceDiscovery(const QVariantMap& data);
    void reconnect();

    QWebSocket m_webSocket;
    QTimer m_reconnectTimer;
    QString m_serverUrl;
    bool m_connected;
    int m_reconnectAttempts;
    int m_maxReconnectAttempts;
    int m_reconnectDelay;
    QSet<QString> m_subscriptions;
    QMap<QString, QDateTime> m_lastMessageTime;
};

#endif // SENSOR_CLIENT_H
