#include "SiteConfigWidget.h"
#include "SiteWidget.h"
#include "SiteGridWidget.h"
#include "SiteScatterWidget.h"

#include <QtWidgets>
#include <QValidator>

SiteConfigWidget::SiteConfigWidget(SiteConfig &siteconfig, QWidget *parent) : QWidget(parent), m_siteConfig(siteconfig)
{
    QGroupBox* siteGroupBox = new QGroupBox(tr("Site"));

    QVBoxLayout* groupLayout = new QVBoxLayout(siteGroupBox);
    groupLayout->setContentsMargins(3,0,3,0);
    groupLayout->setSpacing(0);

    //First we need to add type radio buttons
    m_typeButtonsGroup = new QButtonGroup(this);

    // Number of ground motions per site
    QLabel* numGMLabel= new QLabel("Number of ground motions per site:",this);
    numGMLineEdit = new QLineEdit(this);
    numGMLineEdit->setText("1");
    auto validator = new QIntValidator(1, 50, this);
    numGMLineEdit->setValidator(validator);

    QRadioButton* siteRadioButton = new QRadioButton(tr("Single Location"));
    QRadioButton* gridRadioButton = new QRadioButton(tr("Grid of Locations"));
    QRadioButton* scatRadioButton = new QRadioButton(tr("Scattering Locations"));
    m_typeButtonsGroup->addButton(siteRadioButton, 0);
    m_typeButtonsGroup->addButton(gridRadioButton, 1);
    m_typeButtonsGroup->addButton(scatRadioButton, 2);

    QWidget* typeGroupBox = new QWidget(this);
    typeGroupBox->setContentsMargins(0,0,0,0);
    typeGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");
    QHBoxLayout* typeLayout = new QHBoxLayout(typeGroupBox);
    typeGroupBox->setLayout(typeLayout);
    typeLayout->addWidget(siteRadioButton);
    typeLayout->addWidget(gridRadioButton);
    typeLayout->addWidget(scatRadioButton);
    typeLayout->addWidget(numGMLabel);
    typeLayout->addWidget(numGMLineEdit);
    typeLayout->addStretch(1);

    groupLayout->addWidget(typeGroupBox);

    //We will add stacked widget to switch between grid and single location
    m_stackedWidgets = new QStackedWidget(this);

    //We will add the site to the stacked widget
    SiteWidget* siteWidget = new SiteWidget(siteconfig.site());
    m_stackedWidgets->addWidget(siteWidget);

    //we will also add a grid widget
    siteGridWidget = new SiteGridWidget(siteconfig.siteGrid());
    m_stackedWidgets->addWidget(siteGridWidget);

    //we will also add a scatter widget
    siteScatWidget = new SiteScatterWidget(siteconfig.siteScatter());
    m_stackedWidgets->addWidget(siteScatWidget);

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
    else if (m_siteConfig.getType() == SiteConfig::SiteType::Scatter)
    {
        m_typeButtonsGroup->button(2)->setChecked(true);
        m_stackedWidgets->setCurrentIndex(2);
    }

    setupConnections();
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

void SiteConfigWidget::setupConnections()
{
    connect(m_typeButtonsGroup, QOverload<int>::of(&QButtonGroup::buttonReleased), [this](int id)
    {
        if(id == 0)
            m_siteConfig.setType(SiteConfig::SiteType::Single);
        else if (id == 1)
            m_siteConfig.setType(SiteConfig::SiteType::Grid);
        else if (id == 2)
            m_siteConfig.setType(SiteConfig::SiteType::Scatter);
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
        else if (type == SiteConfig::SiteType::Scatter)
        {
            m_typeButtonsGroup->button(2)->setChecked(true);
            m_stackedWidgets->setCurrentIndex(2);
        }
    });
}


int SiteConfigWidget::getNumberOfGMPerSite(void)
{
    auto res = false;
    int numGM = numGMLineEdit->text().toInt(&res);

    if(res == true)
        return numGM;

    return -1;
}

