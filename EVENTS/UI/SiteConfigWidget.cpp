#include "SiteConfigWidget.h"
#include "SiteWidget.h"
#include "SiteGridWidget.h"
#include "SiteScatterWidget.h"
#include "ComponentInputWidget.h"
#include "VisualizationWidget.h"

#ifdef Q_GIS
#include "QGISSiteInputWidget.h"
#endif

#include <QtWidgets>
#include <QValidator>
#include <QTableWidget>
#include <QScrollArea>

SiteConfigWidget::SiteConfigWidget(SiteConfig &siteconfig, VisualizationWidget* visWidget, bool soilResponse, QWidget *parent)
    : QWidget(parent), m_siteConfig(siteconfig), visualizationWidget(visWidget)
{
    QGroupBox* siteGroupBox = new QGroupBox(tr("Site"));

    QVBoxLayout* groupLayout = new QVBoxLayout(siteGroupBox);
    groupLayout->setContentsMargins(0,0,0,0);
    groupLayout->setSpacing(0);

    //First we need to add type radio buttons
    m_typeButtonsGroup = new QButtonGroup(this);

    // Number of ground motions per site
    QLabel* numGMLabel;
    if (!soilResponse)
    {
        numGMLabel= new QLabel("Number of ground motions per site:",this);
        numGMLineEdit = new QLineEdit(this);
        numGMLineEdit->setText("1");
        auto validator = new QIntValidator(1, 99999999, this);
        numGMLineEdit->setValidator(validator);
    }

    siteRadioButton = new QRadioButton(tr("Single Location"));
    gridRadioButton = new QRadioButton(tr("Grid of Locations"));
    scatRadioButton = new QRadioButton(tr("Scattering Locations"));
    //QRadioButton* csvfRadioButton = new QRadioButton(tr("CSV Inventory"));
    m_typeButtonsGroup->addButton(siteRadioButton, 0);
    m_typeButtonsGroup->addButton(gridRadioButton, 1);
    m_typeButtonsGroup->addButton(scatRadioButton, 2);
    //m_typeButtonsGroup->addButton(csvfRadioButton, 3);

    QWidget* typeGroupBox = new QWidget(this);
    typeGroupBox->setContentsMargins(0,0,0,0);
    typeGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");
    QHBoxLayout* typeLayout = new QHBoxLayout(typeGroupBox);
    typeGroupBox->setLayout(typeLayout);
    typeLayout->addWidget(siteRadioButton);
    typeLayout->addWidget(gridRadioButton);
    typeLayout->addWidget(scatRadioButton);
    //typeLayout->addWidget(csvfRadioButton);
    if (!soilResponse)
    {
        typeLayout->addWidget(numGMLabel);
        typeLayout->addWidget(numGMLineEdit);
    }
    typeLayout->addStretch(1);

    groupLayout->addWidget(typeGroupBox);

    //We will add stacked widget to switch between grid and single location
    m_stackedWidgets = new QStackedWidget(this);

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
        csvSiteInventory = new QGISSiteInputWidget(this, visualizationWidget, "Soil","RegionalSiteResponse");
    else
        csvSiteInventory = new QGISSiteInputWidget(this, visualizationWidget, "Site","regionalGroundMotion");
    csvSiteInventory->setMaximumWidth(800);
    csvSiteInventory->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    QScrollArea *sa = new QScrollArea;
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setWidgetResizable(true);
    sa->setLineWidth(0);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setWidget(csvSiteInventory);
    sa->setMinimumHeight(300);
    m_stackedWidgets->addWidget(sa);

    groupLayout->addWidget(m_stackedWidgets);

    QVBoxLayout* layout = new QVBoxLayout(this);
    siteGroupBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
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


int SiteConfigWidget::getNumberOfGMPerSite(void)
{
    auto res = false;
    int numGM = numGMLineEdit->text().toInt(&res);

    if(res == true)
        return numGM;

    return -1;
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

