#include "SiteConfigWidget.h"
#include "SiteWidget.h"
#include "SiteGridWidget.h"

#include <QtWidgets>
#include <QValidator>

SiteConfigWidget::SiteConfigWidget(SiteConfig &siteconfig, QWidget *parent) : QWidget(parent), m_siteConfig(siteconfig)
{
    QGroupBox* siteGroupBox = new QGroupBox(tr("Site"));

    QVBoxLayout* groupLayout = new QVBoxLayout(siteGroupBox);
    groupLayout->setContentsMargins(5,0,5,5);

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
    m_typeButtonsGroup->addButton(siteRadioButton, 0);
    m_typeButtonsGroup->addButton(gridRadioButton, 1);

    QWidget* typeGroupBox = new QWidget(this);
    typeGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");
    QHBoxLayout* typeLayout = new QHBoxLayout(typeGroupBox);
    typeGroupBox->setLayout(typeLayout);
    typeLayout->addWidget(siteRadioButton);
    typeLayout->addWidget(gridRadioButton);
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

    groupLayout->addWidget(m_stackedWidgets);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(siteGroupBox);

    if(m_siteConfig.getType() == SiteConfig::SiteType::Single)
    {
        m_typeButtonsGroup->button(0)->setChecked(true);
        m_stackedWidgets->setCurrentIndex(0);
    }
    else
    {
        m_typeButtonsGroup->button(1)->setChecked(true);
        m_stackedWidgets->setCurrentIndex(1);
    }

    setupConnections();
}

SiteGridWidget *SiteConfigWidget::getSiteGridWidget() const
{
    return siteGridWidget;
}

void SiteConfigWidget::setupConnections()
{
    connect(m_typeButtonsGroup, QOverload<int>::of(&QButtonGroup::idReleased), [this](int id)
    {
        if(id == 0)
            m_siteConfig.setType(SiteConfig::SiteType::Single);
        else
            m_siteConfig.setType(SiteConfig::SiteType::Grid);
    });

    connect(&m_siteConfig, &SiteConfig::typeChanged, [this](SiteConfig::SiteType type)
    {
        if(type == SiteConfig::SiteType::Single)
        {
            m_typeButtonsGroup->button(0)->setChecked(true);
            m_stackedWidgets->setCurrentIndex(0);
        }
        else
        {
            m_typeButtonsGroup->button(1)->setChecked(true);
            m_stackedWidgets->setCurrentIndex(1);
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

