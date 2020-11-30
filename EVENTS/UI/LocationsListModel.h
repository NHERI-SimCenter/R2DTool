#ifndef LOCATIONSLISTMODEL_H
#define LOCATIONSLISTMODEL_H

#include "Location.h"

#include <QAbstractListModel>

class LocationsListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit LocationsListModel(QObject *parent = nullptr);

signals:

public slots:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void setLocations(QVector<Location *> locations);

private:
    QVector<Location*> m_locations;
};

#endif // LOCATIONSLISTMODEL_H
