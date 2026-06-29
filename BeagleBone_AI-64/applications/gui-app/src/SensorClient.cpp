#include "gui-app/SensorClient.h"
#include "logger/Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace common;

// ============================================================================
// SensorClient Implementation
// ============================================================================

SensorClient::SensorClient(QObject* parent) 
    : QObject(parent),
      m_connected(false),
      m_reconnectAttempts(0),
      m_maxReconnectAttempts(5) {
    
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &SensorClient::onReconnectTimeout);
    
    // Connect websocket signals
    connect(&m_webSocket, &QWebSocket::connected, this, &SensorClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &SensorClient::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, 
            this, &SensorClient::onTextMessageReceived);
    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &SensorClient::onError);
    
    LOG_DEBUG("SensorClient created");
}

SensorClient::~SensorClient() {
    disconnectFromServer();
    LOG_DEBUG("SensorClient destroyed");
}

// ============================================================================
// Properties
// ============================================================================

void SensorClient::setServerUrl(const QString& url) {
    if (m_serverUrl != url) {
        m_serverUrl = url;
        emit serverUrlChanged();
        
        if (m_connected) {
            // Reconnect if already connected
            disconnectFromServer();
            connectToServer();
        }
    }
}

// ============================================================================
// Public Methods
// ============================================================================

void SensorClient::connectToServer() {
    if (m_connected) {
        return;
    }
    
    if (m_serverUrl.isEmpty()) {
        LOG_ERROR("Server URL is empty");
        emit error("Server URL is empty");
        return;
    }
    
    LOG_INFO("Connecting to server: " + m_serverUrl.toStdString());
    m_webSocket.open(QUrl(m_serverUrl));
}

void SensorClient::disconnectFromServer() {
    if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
        m_webSocket.close();
    }
    m_connected = false;
    emit connectedChanged();
}

void SensorClient::sendCommand(const QString& deviceId, const QString& command, 
                               const QVariantMap& params) {
    if (!m_connected) {
        LOG_ERROR("Not connected to server");
        emit error("Not connected to server");
        return;
    }
    
    QJsonObject message;
    message["type"] = "command";
    message["deviceId"] = deviceId;
    message["command"] = command;
    
    QJsonObject paramsObj;
    for (auto it = params.begin(); it != params.end(); ++it) {
        paramsObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    message["params"] = paramsObj;
    
    QJsonDocument doc(message);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    
    m_webSocket.sendTextMessage(jsonString);
    LOG_DEBUG("Command sent: " + command.toStdString() + " to " + deviceId.toStdString());
}

void SensorClient::subscribeToDevice(const QString& deviceId) {
    if (!m_connected) {
        LOG_ERROR("Not connected to server");
        emit error("Not connected to server");
        return;
    }
    
    if (m_subscriptions.contains(deviceId)) {
        return;
    }
    
    QJsonObject message;
    message["type"] = "subscribe";
    message["deviceId"] = deviceId;
    
    QJsonDocument doc(message);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    
    m_webSocket.sendTextMessage(jsonString);
    m_subscriptions.insert(deviceId);
    
    LOG_DEBUG("Subscribed to device: " + deviceId.toStdString());
}

void SensorClient::unsubscribeFromDevice(const QString& deviceId) {
    if (!m_connected || !m_subscriptions.contains(deviceId)) {
        return;
    }
    
    QJsonObject message;
    message["type"] = "unsubscribe";
    message["deviceId"] = deviceId;
    
    QJsonDocument doc(message);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    
    m_webSocket.sendTextMessage(jsonString);
    m_subscriptions.remove(deviceId);
    
    LOG_DEBUG("Unsubscribed from device: " + deviceId.toStdString());
}

bool SensorClient::isSubscribed(const QString& deviceId) const {
    return m_subscriptions.contains(deviceId);
}

// ============================================================================
// Private Slots
// ============================================================================

void SensorClient::onConnected() {
    m_connected = true;
    m_reconnectAttempts = 0;
    emit connectedChanged();
    
    LOG_INFO("Connected to server");
}

void SensorClient::onDisconnected() {
    m_connected = false;
    emit connectedChanged();
    
    LOG_WARN("Disconnected from server");
    
    // Attempt to reconnect
    reconnect();
}

void SensorClient::onTextMessageReceived(const QString& message) {
    LOG_DEBUG("Message received: " + message.left(100).toStdString());
    
    try {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) {
            LOG_ERROR("Invalid message format");
            return;
        }
        
        processMessage(doc.object());
    } catch (const std::exception& e) {
        LOG_ERROR("Error processing message: " + std::string(e.what()));
        emit error("Error processing message");
    }
}

void SensorClient::onError(QAbstractSocket::SocketError error) {
    QString errorMsg = QString("WebSocket error: %1").arg(error);
    LOG_ERROR(errorMsg.toStdString());
    emit error(errorMsg);
    
    m_connected = false;
    emit connectedChanged();
}

void SensorClient::onReconnectTimeout() {
    if (!m_connected) {
        connectToServer();
    }
}

// ============================================================================
// Private Methods
// ============================================================================

void SensorClient::processMessage(const QJsonObject& json) {
    QString type = json.value("type").toString();
    
    if (type == "data") {
        QVariantMap data = json.value("data").toObject().toVariantMap();
        QString deviceId = json.value("deviceId").toString();
        
        if (!deviceId.isEmpty() && m_subscriptions.contains(deviceId)) {
            m_lastMessageTime[deviceId] = QDateTime::currentDateTime();
            emit dataReceived(deviceId, data);
        }
    } else if (type == "status") {
        QVariantMap data = json.value("data").toObject().toVariantMap();
        QString deviceId = json.value("deviceId").toString();
        int status = json.value("status").toInt(0);
        
        if (!deviceId.isEmpty()) {
            emit statusChanged(deviceId, status);
        }
    } else if (type == "discovery") {
        QVariantMap info = json.value("device").toObject().toVariantMap();
        QString deviceId = info.value("id").toString();
        
        if (!deviceId.isEmpty()) {
            emit deviceDiscovered(deviceId, info);
        }
    } else if (type == "error") {
        QString errorMsg = json.value("message").toString("Unknown error");
        emit error(errorMsg);
    } else {
        LOG_WARN("Unknown message type: " + type.toStdString());
    }
}

void SensorClient::reconnect() {
    if (m_reconnectAttempts >= m_maxReconnectAttempts) {
        LOG_ERROR("Max reconnect attempts reached");
        emit error("Max reconnect attempts reached");
        return;
    }
    
    m_reconnectAttempts++;
    emit reconnectAttemptsChanged();
    
    int delay = qMin(5000, 1000 * m_reconnectAttempts);
    LOG_INFO("Reconnecting in " + QString::number(delay) + "ms (attempt " + 
             QString::number(m_reconnectAttempts) + "/" + 
             QString::number(m_maxReconnectAttempts) + ")");
    
    m_reconnectTimer.start(delay);
}
