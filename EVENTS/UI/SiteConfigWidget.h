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
class QRadioButton;

class SiteConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SiteConfigWidget(SiteConfig& siteconfig, VisualizationWidget* visWidget, bool soilResponse = false, QWidget *parent = nullptr);

    SiteGridWidget *getSiteGridWidget() const;
    SiteScatterWidget *getSiteScatterWidget() const;
    QGISSiteInputWidget *getCsvSiteWidget();

    int getNumberOfGMPerSite(void);
    QString getFilter(void);
    void setSiteType(SiteConfig::SiteType siteType);

signals:
    void soilDataCompleteSignal(bool flag);
    void setSiteFilterSignal(QString filter);
    void siteTypeChangedSignal(SiteConfig::SiteType siteType);

public slots:
    void soilDataCompleteSlot(bool flag);
    void setSiteFilterSlot(QString filter);

private:
    SiteConfig& m_siteConfig;
    QButtonGroup* m_typeButtonsGroup;
    QStackedWidget* m_stackedWidgets;
    SiteGridWidget* siteGridWidget;
    SiteScatterWidget* siteScatWidget;
    QGISSiteInputWidget* csvSiteInventory;
    QLineEdit* numGMLineEdit;
    VisualizationWidget* visualizationWidget;

    QRadioButton* siteRadioButton;
    QRadioButton* gridRadioButton;
    QRadioButton* scatRadioButton;

    void setupConnections();
};

#endif // SITECONFIGWIDGET_H
