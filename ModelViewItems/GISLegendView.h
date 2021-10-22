#ifndef GISLegendView_H
#define GISLegendView_H

#include <QTreeView>
#include <QIdentityProxyModel>

#include <QIcon>


class GISLegendView : public QTreeView
{
public:
    GISLegendView(QWidget *parent = nullptr);

    QSize sizeHint() const;

    void clear();

    virtual QIdentityProxyModel *getProxyModel() const;

private:


};

#endif // GISLegendView_H
