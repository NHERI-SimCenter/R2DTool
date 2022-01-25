#ifndef SITECONFIGWIDGET_H
#define SITECONFIGWIDGET_H

#include <QWidget>
#include "SiteConfig.h"

class SiteGridWidget;
class SiteScatterWidget;
class VisualizationWidget;

class QButtonGroup;
class QStackedWidget;
class QLineEdit;
class QGISSiteInputWidget;

class SiteConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SiteConfigWidget(SiteConfig& siteconfig, VisualizationWidget* visWidget, bool soilResponse = false, QWidget *parent = nullptr);

    SiteGridWidget *getSiteGridWidget() const;
    SiteScatterWidget *getSiteScatterWidget() const;
    QGISSiteInputWidget *getCsvSiteWidget();

    int getNumberOfGMPerSite(void);

signals:
    void soilDataCompleteSignal(bool flag);

public slots:
    void soilDataCompleteSlot(bool flag);

private:
    SiteConfig& m_siteConfig;
    QButtonGroup* m_typeButtonsGroup;
    QStackedWidget* m_stackedWidgets;
    SiteGridWidget* siteGridWidget;
    SiteScatterWidget* siteScatWidget;
    QGISSiteInputWidget* csvSiteInventory;
    QLineEdit* numGMLineEdit;
    VisualizationWidget* visualizationWidget;

    void setupConnections();
};

#endif // SITECONFIGWIDGET_H
