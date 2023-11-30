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

#include "GroundMotionModelsWidget.h"
#include "MeanUCERFPoissonWidget.h"
#include "MeanUCERFFM3Widget.h"
#include "SpatialCorrelationWidget.h"
#include "GMPEWidget.h"
#include "IntensityMeasureWidget.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>

GroundMotionModelsWidget::GroundMotionModelsWidget(QWidget *parent) : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    spatialCorrWidget = new SpatialCorrelationWidget();

    m_gmpe = new GMPE(this);
    m_gmpeWidget = new GMPEWidget(*this->m_gmpe);

    m_intensityMeasure = new IntensityMeasure(this);
    m_intensityMeasureWidget = new IntensityMeasureWidget(*this->m_intensityMeasure);

    // GMPE options (link between source type and GMPE options)
//    connect(erfWidget->ruptureWidget(), SIGNAL(widgetTypeChanged(QString)),
//            m_gmpeWidget, SLOT(handleAvailableGMPE(QString)));

//    // correlation model options (link between source type and correlation model options)
//    connect(erfWidget->ruptureWidget(), SIGNAL(widgetTypeChanged(QString)),
//            spatialCorrWidget, SLOT(handleAvailableModel(QString)));

//    // Intensity Measure Levels options (link between source type and intensity measure levels options)
//    connect(erfWidget->ruptureWidget(), SIGNAL(widgetTypeChanged(QString)),
//            m_intensityMeasureWidget, SLOT(handleIntensityMeasureLevels(QString)));

    mainLayout->addWidget(m_intensityMeasureWidget);
    mainLayout->addWidget(m_gmpeWidget);
    mainLayout->addWidget(spatialCorrWidget);

}


void GroundMotionModelsWidget::reset(void)
{

}


IntensityMeasure *GroundMotionModelsWidget::intensityMeasure() const
{
    return m_intensityMeasure;
}


bool GroundMotionModelsWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool GroundMotionModelsWidget::outputToJSON(QJsonObject& obj)
{

    // Get the correlation model Json object
    auto corrModObj = spatialCorrWidget->getJsonCorr();

    // Get the intensity measure Json object
    auto IMObj = m_intensityMeasure->getJson();

    obj.insert("CorrelationModel", corrModObj);
    obj.insert("IntensityMeasure", IMObj);

    return true;
}

