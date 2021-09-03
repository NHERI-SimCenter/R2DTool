#include "ArcGISLegendView.h"

#include "GISLegendView.h"

#include <QHeaderView>


ArcGISLegendView::ArcGISLegendView(QWidget *parent) : GISLegendView(parent)
{
    proxyModel = new RoleProxyModel(this);
    this->setModel(proxyModel);
}




QIdentityProxyModel *ArcGISLegendView::getProxyModel() const
{
    return proxyModel;
}
