#include "gui-app/ChartModel.h"
#include "logger/Logger.h"
#include <QDateTime>
#include <cmath>

using namespace common;

// ============================================================================
// ChartModel Implementation
// ============================================================================

ChartModel::ChartModel(QObject* parent) 
    : QAbstractListModel(parent),
      m_maxPoints(100),
      m_autoRange(true),
      m_minValue(0),
      m_maxValue(100) {
    
    LOG_DEBUG("ChartModel created");
}

ChartModel::~ChartModel() {
    LOG_DEBUG("ChartModel destroyed");
}

// ============================================================================
// QAbstractListModel Implementation
// ============================================================================

int ChartModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    QMutexLocker locker(&m_mutex);
    return m_data.size();
}

QVariant ChartModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_data.size()) {
        return QVariant();
    }

    QMutexLocker locker(&m_mutex);
    const DataPoint& point = m_data[index.row()];

    switch (role) {
        case XValueRole:
            return point.x;
        case YValueRole:
            return point.y;
        case LabelRole:
            return point.label;
        case ColorRole:
            return point.color;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> ChartModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[XValueRole] = "xValue";
    roles[YValueRole] = "yValue";
    roles[LabelRole] = "label";
    roles[ColorRole] = "color";
    return roles;
}

// ============================================================================
// Properties
// ============================================================================

void ChartModel::setDeviceId(const QString& deviceId) {
    if (m_deviceId != deviceId) {
        m_deviceId = deviceId;
        emit deviceIdChanged();
        clear();
    }
}

void ChartModel::setDataKey(const QString& dataKey) {
    if (m_dataKey != dataKey) {
        m_dataKey = dataKey;
        emit dataKeyChanged();
        clear();
    }
}

// ============================================================================
// Public Methods
// ============================================================================

void ChartModel::addDataPoint(double x, double y, const QString& label) {
    appendDataPoint(x, y, label);
}

void ChartModel::appendDataPoint(double x, double y, const QString& label) {
    QMutexLocker locker(&m_mutex);
    
    // Generate color based on value
    QString color = generateColor(y);
    
    DataPoint point;
    point.x = x;
    point.y = y;
    point.label = label.isEmpty() ? QString::number(x) : label;
    point.color = color;
    
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(point);
    endInsertRows();
    
    // Limit points
    while (m_data.size() > m_maxPoints) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_data.removeFirst();
        endRemoveRows();
    }
    
    updateRange();
    emit countChanged();
    emit dataChanged();
}

void ChartModel::appendDataPoints(const QVariantList& points) {
    QMutexLocker locker(&m_mutex);
    
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size() + points.size() - 1);
    
    for (const auto& pointVariant : points) {
        QVariantMap pointMap = pointVariant.toMap();
        DataPoint point;
        point.x = pointMap.value("x", 0.0).toDouble();
        point.y = pointMap.value("y", 0.0).toDouble();
        point.label = pointMap.value("label", "").toString();
        point.color = generateColor(point.y);
        m_data.append(point);
    }
    
    endInsertRows();
    
    // Limit points
    while (m_data.size() > m_maxPoints) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_data.removeFirst();
        endRemoveRows();
    }
    
    updateRange();
    emit countChanged();
    emit dataChanged();
}

void ChartModel::clear() {
    QMutexLocker locker(&m_mutex);
    beginResetModel();
    m_data.clear();
    endResetModel();
    m_minValue = 0;
    m_maxValue = 100;
    emit countChanged();
    emit dataChanged();
    emit rangeChanged(m_minValue, m_maxValue);
}

void ChartModel::removeLast() {
    QMutexLocker locker(&m_mutex);
    if (!m_data.isEmpty()) {
        beginRemoveRows(QModelIndex(), m_data.size() - 1, m_data.size() - 1);
        m_data.removeLast();
        endRemoveRows();
        updateRange();
        emit countChanged();
        emit dataChanged();
    }
}

void ChartModel::setData(const QVariantList& data) {
    clear();
    
    QMutexLocker locker(&m_mutex);
    beginInsertRows(QModelIndex(), 0, data.size() - 1);
    
    for (const auto& item : data) {
        QVariantMap map = item.toMap();
        DataPoint point;
        point.x = map.value("x", 0.0).toDouble();
        point.y = map.value("y", 0.0).toDouble();
        point.label = map.value("label", "").toString();
        point.color = generateColor(point.y);
        m_data.append(point);
    }
    
    endInsertRows();
    
    // Limit points
    while (m_data.size() > m_maxPoints) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_data.removeFirst();
        endRemoveRows();
    }
    
    updateRange();
    emit countChanged();
    emit dataChanged();
}

QVariantList ChartModel::getData() const {
    QMutexLocker locker(&m_mutex);
    QVariantList result;
    for (const auto& point : m_data) {
        QVariantMap map;
        map["x"] = point.x;
        map["y"] = point.y;
        map["label"] = point.label;
        map["color"] = point.color;
        result.append(map);
    }
    return result;
}

void ChartModel::setMaxPoints(int maxPoints) {
    if (m_maxPoints != maxPoints && maxPoints > 0) {
        m_maxPoints = maxPoints;
        // Trim data if needed
        QMutexLocker locker(&m_mutex);
        while (m_data.size() > m_maxPoints) {
            beginRemoveRows(QModelIndex(), 0, 0);
            m_data.removeFirst();
            endRemoveRows();
        }
        emit countChanged();
    }
}

void ChartModel::setAutoRange(bool autoRange) {
    if (m_autoRange != autoRange) {
        m_autoRange = autoRange;
        if (autoRange) {
            updateRange();
        }
    }
}

QPair<double, double> ChartModel::getRange() const {
    QMutexLocker locker(&m_mutex);
    return qMakePair(m_minValue, m_maxValue);
}

double ChartModel::getMinValue() const {
    QMutexLocker locker(&m_mutex);
    return m_minValue;
}

double ChartModel::getMaxValue() const {
    QMutexLocker locker(&m_mutex);
    return m_maxValue;
}

// ============================================================================
// Private Methods
// ============================================================================

void ChartModel::updateRange() {
    if (!m_autoRange || m_data.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    double minVal = m_data[0].y;
    double maxVal = m_data[0].y;
    
    for (const auto& point : m_data) {
        if (point.y < minVal) minVal = point.y;
        if (point.y > maxVal) maxVal = point.y;
    }
    
    // Add some padding (10%)
    double padding = (maxVal - minVal) * 0.1;
    if (padding < 0.1) padding = 0.1;
    
    double oldMin = m_minValue;
    double oldMax = m_maxValue;
    
    m_minValue = minVal - padding;
    m_maxValue = maxVal + padding;
    
    if (oldMin != m_minValue || oldMax != m_maxValue) {
        emit rangeChanged(m_minValue, m_maxValue);
    }
}

QString ChartModel::generateColor(double value) const {
    // Generate color based on value (green to red gradient)
    // Normalize value to 0-1 range
    double normalized = 0.5;
    
    if (!m_data.isEmpty()) {
        double minVal = m_data[0].y;
        double maxVal = m_data[0].y;
        
        for (const auto& point : m_data) {
            if (point.y < minVal) minVal = point.y;
            if (point.y > maxVal) maxVal = point.y;
        }
        
        if (maxVal > minVal) {
            normalized = (value - minVal) / (maxVal - minVal);
        }
    }
    
    // Clamp to 0-1
    normalized = std::max(0.0, std::min(1.0, normalized));
    
    // HSL: 120 (green) to 0 (red)
    int hue = 120 - static_cast<int>(normalized * 120);
    
    return QString("hsl(%1, 80%, 50%)").arg(hue);
}
