#ifndef GISLEGENDVIEW_H
#define GISLEGENDVIEW_H

#include <QListView>
#include <QIdentityProxyModel>
#include <QIcon>

#include "LegendInfoListModel.h"

class RoleProxyModel: public QIdentityProxyModel
{
public:

    RoleProxyModel(QWidget* parent) : QIdentityProxyModel(parent)
    {

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
};


class GISLegendView : public QListView
{
public:
    GISLegendView(QWidget *parent = nullptr);

//    QSize sizeHint() const;
//    QSize minimumSizeHint() const;

    void setModel(QAbstractItemModel *model);
};

#endif // GISLEGENDVIEW_H
