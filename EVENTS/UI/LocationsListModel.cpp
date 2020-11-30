#include "LocationsListModel.h"

LocationsListModel::LocationsListModel(QObject *parent) : QAbstractListModel(parent)
{

}


int LocationsListModel::rowCount(const QModelIndex &parent) const
{
    return m_locations.size();
}

QVariant LocationsListModel::data(const QModelIndex &index, int role) const
{
    return QVariant::fromValue<Location*>(m_locations[index.row()]);
}

void LocationsListModel::setLocations(QVector<Location*> locations)
{
    if(m_locations.size() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, m_locations.size()-1);
        //Disposing old locations
        Location* pLocation;
        foreach (pLocation, m_locations)
            pLocation->deleteLater();
        m_locations.clear();
        endRemoveRows();
    }

    //Adding new locations
    beginInsertRows(QModelIndex(), 0, locations.size()-1);
    m_locations = locations;
    endInsertRows();
}
