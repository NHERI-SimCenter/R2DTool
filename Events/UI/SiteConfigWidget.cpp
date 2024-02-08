#include "SiteConfigWidget.h"
#include "SiteWidget.h"
#include "SiteGridWidget.h"
#include "SiteScatterWidget.h"
#include "QGISVisualizationWidget.h"
#include "VerticalScrollingWidget.h"

#include "QGISSiteInputWidget.h"

#include <QtWidgets>
#include <QValidator>
#include <QTableWidget>
#include <QScrollArea>

SiteConfigWidget::SiteConfigWidget(SiteConfig &siteconfig, QGISVisualizationWidget* visWidget, bool soilResponse, QWidget *parent)
    : QWidget(parent), m_siteConfig(siteconfig), visualizationWidget(visWidget)
{
    QGroupBox* siteGroupBox = new QGroupBox(tr("Site"));
    siteGroupBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QVBoxLayout* groupLayout = new QVBoxLayout(siteGroupBox);
    groupLayout->setContentsMargins(0,0,0,0);
    groupLayout->setSpacing(0);

    //First we need to add type radio buttons
    m_typeButtonsGroup = new QButtonGroup();

    siteRadioButton = new QRadioButton(tr("Single Location"));
    gridRadioButton = new QRadioButton(tr("Grid of Locations"));
    scatRadioButton = new QRadioButton(tr("Scattering Locations"));
    //QRadioButton* csvfRadioButton = new QRadioButton(tr("CSV Inventory"));
    m_typeButtonsGroup->addButton(siteRadioButton, 0);
    m_typeButtonsGroup->addButton(gridRadioButton, 1);
    m_typeButtonsGroup->addButton(scatRadioButton, 2);
    //m_typeButtonsGroup->addButton(csvfRadioButton, 3);

    QWidget* typeGroupBox = new QWidget();
    typeGroupBox->setContentsMargins(0,0,0,0);
    typeGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");
    QHBoxLayout* typeLayout = new QHBoxLayout(typeGroupBox);
    typeLayout->addWidget(siteRadioButton);
    typeLayout->addWidget(gridRadioButton);
    typeLayout->addWidget(scatRadioButton);
    //typeLayout->addWidget(csvfRadioButton);

    typeLayout->addStretch(1);

    groupLayout->addWidget(typeGroupBox);

    //We will add stacked widget to switch between grid and single location
    m_stackedWidgets = new QStackedWidget();
    m_stackedWidgets->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    //We will add the site to the stacked widget
    siteWidget = new SiteWidget(siteconfig.site());
    m_stackedWidgets->addWidget(siteWidget);

    //we will also add a grid widget
    siteGridWidget = new SiteGridWidget(siteconfig.siteGrid());
    m_stackedWidgets->addWidget(siteGridWidget);

    //we will also add a scatter widget
    siteScatWidget = new SiteScatterWidget(siteconfig.siteScatter());
    //m_stackedWidgets->addWidget(siteScatWidget);

    //add a new one for site inventory
    if (soilResponse)
        csvSiteInventory = new QGISSiteInputWidget(this, visualizationWidget, "SiteSoilColumn","RegionalSiteResponse");
    else
        csvSiteInventory = new QGISSiteInputWidget(this, visualizationWidget, "Sites","regionalGroundMotion");

    csvSiteInventory->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    auto sa = new VerticalScrollingWidget(csvSiteInventory);

    m_stackedWidgets->addWidget(sa);

    groupLayout->addWidget(m_stackedWidgets);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(siteGroupBox);

    if(m_siteConfig.getType() == SiteConfig::SiteType::Single)
    {
        m_typeButtonsGroup->button(0)->setChecked(true);
        m_stackedWidgets->setCurrentIndex(0);
    }
    else if (m_siteConfig.getType() == SiteConfig::SiteType::Grid)
    {
        m_typeButtonsGroup->button(1)->setChecked(true);
        m_stackedWidgets->setCurrentIndex(1);
    }
    //else if (m_siteConfig.getType() == SiteConfig::SiteType::Scatter)
    //{
    //    m_typeButtonsGroup->button(2)->setChecked(true);
    //    m_stackedWidgets->setCurrentIndex(2);
    //}
    else if (m_siteConfig.getType() == SiteConfig::SiteType::UserCSV)
    {
        m_typeButtonsGroup->button(2)->setChecked(true);
        m_stackedWidgets->setCurrentIndex(2);
    }

    setupConnections();
}

void SiteConfigWidget::clear()
{
    csvSiteInventory->clear();
}

SiteWidget *SiteConfigWidget::getSiteWidget()
{
    return siteWidget;
}

SiteGridWidget *SiteConfigWidget::getSiteGridWidget() const
{
    return siteGridWidget;
}

// Getting the site scatter widget
SiteScatterWidget *SiteConfigWidget::getSiteScatterWidget() const
{
    return siteScatWidget;
}

// Getting the site user-csv widget
QGISSiteInputWidget *SiteConfigWidget::getCsvSiteWidget()
{
    return csvSiteInventory;
}

void SiteConfigWidget::soilDataCompleteSlot(bool flag)
{
    emit soilDataCompleteSignal(flag);
}

void SiteConfigWidget::setupConnections()
{
    connect(m_typeButtonsGroup, QOverload<int>::of(&QButtonGroup::buttonReleased), [this](int id)
    {
        if(id == 0)
            m_siteConfig.setType(SiteConfig::SiteType::Single);
        else if (id == 1)
            m_siteConfig.setType(SiteConfig::SiteType::Grid);
        //else if (id == 2)
        //    m_siteConfig.setType(SiteConfig::SiteType::Scatter);
        else if (id == 2)
            m_siteConfig.setType(SiteConfig::SiteType::UserCSV);
    });

    connect(&m_siteConfig, &SiteConfig::typeChanged, [this](SiteConfig::SiteType type)
    {
        if(type == SiteConfig::SiteType::Single)
        {
            m_typeButtonsGroup->button(0)->setChecked(true);
            m_stackedWidgets->setCurrentIndex(0);
        }
        else if (type == SiteConfig::SiteType::Grid)
        {
            m_typeButtonsGroup->button(1)->setChecked(true);
            m_stackedWidgets->setCurrentIndex(1);
        }
        //else if (type == SiteConfig::SiteType::Scatter)
        //{
        //    m_typeButtonsGroup->button(2)->setChecked(true);
        //    m_stackedWidgets->setCurrentIndex(2);
        //}
        else if (type == SiteConfig::SiteType::UserCSV)
        {
            m_typeButtonsGroup->button(2)->setChecked(true);
            m_stackedWidgets->setCurrentIndex(2);
        }
    });

    connect(&m_siteConfig, &SiteConfig::typeChanged, [this](SiteConfig::SiteType siteType)
    {
        emit siteTypeChangedSignal(siteType);
    });

    connect(csvSiteInventory, SIGNAL(soilDataCompleteSignal(bool)), this, SLOT(soilDataCompleteSlot(bool)));
    connect(this, SIGNAL(setSiteFilterSignal(QString)), csvSiteInventory, SLOT(setSiteFilter(QString)));
    connect(csvSiteInventory, SIGNAL(activateSoilModelWidget(bool)), this, SLOT(activateSoilModelWidgetSlot(bool)));
}



QString SiteConfigWidget::getFilter(void)
{
    QString filter = "";
    if (m_siteConfig.getType() == SiteConfig::SiteType::UserCSV)
        filter = csvSiteInventory->getFilterString();
    else if (m_siteConfig.getType() == SiteConfig::SiteType::Grid)
    {
        int siteNum = siteGridWidget->getNumSites();
        filter = "0-"+QString::number(siteNum-1);
    }
    return filter;
}

void SiteConfigWidget::setSiteFilterSlot(QString filter)
{
    if (m_siteConfig.getType() == SiteConfig::SiteType::UserCSV)
        emit setSiteFilterSignal(filter);
}


void SiteConfigWidget::setSiteType(SiteConfig::SiteType siteType)
{
    if (siteType==SiteConfig::SiteType::Single)
        siteRadioButton->click();
    else if (siteType==SiteConfig::SiteType::Grid)
        gridRadioButton->click();
    else if (siteType==SiteConfig::SiteType::UserCSV)
        scatRadioButton->click();
}

void SiteConfigWidget::activateSoilModelWidgetSlot(bool flag)
{
    emit activateSoilModelWidgetSignal(flag);
}

