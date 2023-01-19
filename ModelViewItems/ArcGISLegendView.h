#ifndef ArcGISLegendView_H
#define ArcGISLegendView_H

#include "GISLegendView.h"

#include <QTreeView>
#include <QIdentityProxyModel>
#include <QIcon>

#include "LegendInfoListModel.h"

class RoleProxyModel: public QIdentityProxyModel
{
public:

    RoleProxyModel(QWidget* parent) : QIdentityProxyModel(parent)
    {

    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (sourceModel())
        {
            return "Legend";
        }

        return QIdentityProxyModel::headerData(section, orientation, role);
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (role == Qt::DecorationRole)
        {
            const QUrl iconRole = index.data(Esri::ArcGISRuntime::LegendInfoListModel::LegendInfoSymbolUrlRole).toUrl();
            return QIcon(iconRole.toLocalFile());
        }
        else
        {
            return QIdentityProxyModel::data(index, role);
        }
    }

private:

};


class ArcGISLegendView : public GISLegendView
{
public:
    ArcGISLegendView(QWidget *parent = nullptr);

    QIdentityProxyModel *getProxyModel() const;

private:

    RoleProxyModel* proxyModel;

};

#endif // ArcGISLegendView_H
