#ifndef RECORDSELECTION_H
#define RECORDSELECTION_H

#include <QObject>


class RecordSelection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int recordId READ recordId WRITE setRecordId NOTIFY recordChanged)
    Q_PROPERTY(double scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleChanged)

public:
    explicit RecordSelection(QObject *parent = nullptr);
    int recordId() const;
    double scaleFactor() const;

signals:
    void recordChanged(int newRecordId);
    void scaleChanged(double newScaleFactor);

public slots:
    void setRecordId(int recordId);
    void setScaleFactor(double scaleFactor);

private:
    int m_recordId;
    double m_scaleFactor;
};

#endif // RECORDSELECTION_H
