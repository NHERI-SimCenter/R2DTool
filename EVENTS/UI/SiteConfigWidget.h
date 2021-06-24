#ifndef SITECONFIGWIDGET_H
#define SITECONFIGWIDGET_H

#include <QWidget>
#include "SiteConfig.h"

class SiteGridWidget;
class SiteScatterWidget;

class QButtonGroup;
class QStackedWidget;
class QLineEdit;

class SiteConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SiteConfigWidget(SiteConfig& siteconfig, QWidget *parent = nullptr);

    SiteGridWidget *getSiteGridWidget() const;
    SiteScatterWidget *getSiteScatterWidget() const;

    int getNumberOfGMPerSite(void);

signals:

public slots:

private:
    SiteConfig& m_siteConfig;
    QButtonGroup* m_typeButtonsGroup;
    QStackedWidget* m_stackedWidgets;
    SiteGridWidget* siteGridWidget;
    SiteScatterWidget* siteScatWidget;
    QLineEdit* numGMLineEdit;

    void setupConnections();
};

#endif // SITECONFIGWIDGET_H
