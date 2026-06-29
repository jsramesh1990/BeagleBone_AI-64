#ifndef CHART_MODEL_H
#define CHART_MODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QPointF>
#include <QMutex>

/**
 * @brief Chart Model for displaying data in QML charts
 * 
 * Provides a data model for chart widgets with support for
 * auto-ranging, color coding, and data point management.
 */
class ChartModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString dataKey READ dataKey WRITE setDataKey NOTIFY dataKeyChanged)
    Q_PROPERTY(int maxPoints READ maxPoints WRITE setMaxPoints NOTIFY maxPointsChanged)
    Q_PROPERTY(bool autoRange READ autoRange WRITE setAutoRange NOTIFY autoRangeChanged)
    Q_PROPERTY(double minValue READ getMinValue NOTIFY rangeChanged)
    Q_PROPERTY(double maxValue READ getMaxValue NOTIFY rangeChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)

public:
    enum ChartRoles {
        XValueRole = Qt::UserRole + 1,
        YValueRole,
        LabelRole,
        ColorRole,
        IndexRole
    };

    explicit ChartModel(QObject* parent = nullptr);
    ~ChartModel();

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    QString deviceId() const { return m_deviceId; }
    void setDeviceId(const QString& deviceId);
    
    QString dataKey() const { return m_dataKey; }
    void setDataKey(const QString& dataKey);
    
    int maxPoints() const { return m_maxPoints; }
    void setMaxPoints(int maxPoints);
    
    bool autoRange() const { return m_autoRange; }
    void setAutoRange(bool autoRange);
    
    double getMinValue() const { QMutexLocker locker(&m_mutex); return m_minValue; }
    double getMaxValue() const { QMutexLocker locker(&m_mutex); return m_maxValue; }
    
    QString title() const { return m_title; }
    void setTitle(const QString& title) {
        if (m_title != title) {
            m_title = title;
            emit titleChanged();
        }
    }
    
    QString unit() const { return m_unit; }
    void setUnit(const QString& unit) {
        if (m_unit != unit) {
            m_unit = unit;
            emit unitChanged();
        }
    }

    // Public methods
    Q_INVOKABLE void addDataPoint(double x, double y, const QString& label = QString());
    Q_INVOKABLE void clear();
    Q_INVOKABLE void removeLast();
    Q_INVOKABLE void setData(const QVariantList& data);
    Q_INVOKABLE QVariantList getData() const;
    Q_INVOKABLE QPair<double, double> getRange() const;
    Q_INVOKABLE void setRange(double min, double max);
    Q_INVOKABLE void resetRange();
    Q_INVOKABLE int indexOf(double x) const;
    Q_INVOKABLE QVariantMap getPointAt(int index) const;

signals:
    void countChanged();
    void deviceIdChanged();
    void dataKeyChanged();
    void maxPointsChanged();
    void autoRangeChanged();
    void rangeChanged(double min, double max);
    void titleChanged();
    void unitChanged();
    void dataChanged();

public slots:
    void appendDataPoint(double x, double y, const QString& label = QString());
    void appendDataPoints(const QVariantList& points);

private:
    struct DataPoint {
        double x;
        double y;
        QString label;
        QString color;
        
        DataPoint() : x(0), y(0) {}
        DataPoint(double xVal, double yVal, const QString& lbl = QString())
            : x(xVal), y(yVal), label(lbl) {}
    };

    void updateRange();
    QString generateColor(double value) const;
    QString interpolateColor(double value, double minVal, double maxVal) const;

    QVector<DataPoint> m_data;
    QString m_deviceId;
    QString m_dataKey;
    QString m_title;
    QString m_unit;
    int m_maxPoints;
    bool m_autoRange;
    double m_minValue;
    double m_maxValue;
    mutable QMutex m_mutex;
};

#endif // CHART_MODEL_H
