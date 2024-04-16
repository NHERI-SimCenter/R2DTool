/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "GMSiteWidget.h"
#include "SiteConfigWidget.h"
#include "Vs30Widget.h"
#include "zDepthWidget.h"

#include <QVBoxLayout>
#include <QGroupBox>

GMSiteWidget::GMSiteWidget(QGISVisualizationWidget* visWidget, QWidget *parent) : QWidget(parent), theVisualizationWidget(visWidget)
{
    // Adding Site Config Widget
    m_siteConfig = new SiteConfig(this);
    m_siteConfigWidget = new SiteConfigWidget(*m_siteConfig, visWidget, false);

    // Adding vs30 widget
    this->m_vs30 = new Vs30(this);
    this->m_vs30Widget = new Vs30Widget(*this->m_vs30, *this->m_siteConfig);

    // Add the zDepth widgets
    QGroupBox* z1GroupBox = new QGroupBox(this);
    QString title = QString("Depth (m) to Vs=1.0 km/sec (Z1pt0)");
    z1GroupBox->setTitle(title);
    QHBoxLayout* z1Layout = new QHBoxLayout(z1GroupBox);
    m_z1DepthWidget = new zDepthWidget("Z1pt0",this);
    z1Layout->addWidget(m_z1DepthWidget);

    QGroupBox* z2GroupBox = new QGroupBox(this);
    QString title2 = QString("Depth (m) to Vs=2.5 km/sec (Z2pt5)");
    z2GroupBox->setTitle(title2);
    QHBoxLayout* z2Layout = new QHBoxLayout(z2GroupBox);
    m_z2DepthWidget = new zDepthWidget("Z2pt5",this);
    z2Layout->addWidget(m_z2DepthWidget);


    QVBoxLayout *siteLayout=new QVBoxLayout(this);
    siteLayout->addWidget(m_siteConfigWidget);
    siteLayout->addWidget(m_vs30Widget);
    siteLayout->addWidget(z1GroupBox);
    siteLayout->addWidget(z2GroupBox);
    siteLayout->addStretch();
}

SiteConfig *GMSiteWidget::siteConfig() const
{
    return m_siteConfig;
}

SiteConfigWidget *GMSiteWidget::siteConfigWidget() const
{
    return m_siteConfigWidget;
}


void GMSiteWidget::outputToJson(QJsonObject& obj)
{

    // Get the Vs30 Json object
    QJsonObject Vs30obj;
    m_vs30->outputToJSON(Vs30obj);
    obj.insert("Vs30", Vs30obj);
    // Get the z1pt0 Json object
    QJsonObject z1pt0obj;
    m_z1DepthWidget->outputToJSON(z1pt0obj);
    obj.insert("Z1pt0", z1pt0obj);
    // Get the z1pt0 Json object
    QJsonObject z2pt5obj;
    m_z2DepthWidget->outputToJSON(z2pt5obj);
    obj.insert("Z2pt5", z2pt5obj);

}

void GMSiteWidget::clear(){
    m_siteConfigWidget->clear();
}

