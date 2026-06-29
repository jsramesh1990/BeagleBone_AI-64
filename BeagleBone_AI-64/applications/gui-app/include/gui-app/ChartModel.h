#ifndef CHART_MODEL_H
#define CHART_MODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QVector>
#include <QPointF>
#include <QMutex>

class ChartModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString dataKey READ dataKey WRITE setDataKey NOTIFY dataKeyChanged)

public:
    enum ChartRoles {
        XValueRole = Qt::UserRole + 1,
        YValueRole,
        LabelRole,
        ColorRole
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

    Q_INVOKABLE void addDataPoint(double x, double y, const QString& label = QString());
    Q_INVOKABLE void clear();
    Q_INVOKABLE void removeLast();
    Q_INVOKABLE void setData(const QVariantList& data);
    Q_INVOKABLE QVariantList getData() const;
    Q_INVOKABLE void setMaxPoints(int maxPoints);
    Q_INVOKABLE void setAutoRange(bool autoRange);
    Q_INVOKABLE QPair<double, double> getRange() const;
    Q_INVOKABLE double getMinValue() const;
    Q_INVOKABLE double getMaxValue() const;

signals:
    void countChanged();
    void deviceIdChanged();
    void dataKeyChanged();
    void dataChanged();
    void rangeChanged(double min, double max);

public slots:
    void appendDataPoint(double x, double y, const QString& label = QString());
    void appendDataPoints(const QVariantList& points);

private:
    struct DataPoint {
        double x;
        double y;
        QString label;
        QString color;
    };

    void updateRange();

    QVector<DataPoint> m_data;
    QString m_deviceId;
    QString m_dataKey;
    int m_maxPoints;
    bool m_autoRange;
    double m_minValue;
    double m_maxValue;
    mutable QMutex m_mutex;
};

#endif // CHART_MODEL_H
