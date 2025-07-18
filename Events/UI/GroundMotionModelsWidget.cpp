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
#include <QSet>
#include <QMessageBox>

GroundMotionModelsWidget::GroundMotionModelsWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    QVBoxLayout * mainLayout = new QVBoxLayout(this);

    theIntensityMeasure = new IntensityMeasure(this);
    theIntensityMeasureWidget = new IntensityMeasureWidget(theIntensityMeasure);

    theSpatialCorrWidget = new SpatialCorrelationWidget(theIntensityMeasureWidget->getSelectedIMTypes());

    theGMPE = new GMPEWidget(theIntensityMeasureWidget->getSelectedIMTypes());

    mainLayout->addWidget(theIntensityMeasureWidget);
    mainLayout->addWidget(theGMPE);
    mainLayout->addWidget(theSpatialCorrWidget);
    mainLayout->addStretch(0);

    connect(theIntensityMeasureWidget, &IntensityMeasureWidget::IMSelectionChanged,
            theGMPE, &GMPEWidget::toggleIMselection);
    connect(theIntensityMeasureWidget, &IntensityMeasureWidget::IMSelectionChanged,
            theSpatialCorrWidget, &SpatialCorrelationWidget::toggleIMselection);

}


void GroundMotionModelsWidget::reset(void)
{

}


IntensityMeasure *GroundMotionModelsWidget::intensityMeasure() const
{
    return theIntensityMeasure;
}


bool GroundMotionModelsWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool GroundMotionModelsWidget::outputToJSON(QJsonObject& obj)
{
    QStringList* selectedIMtypes = theIntensityMeasureWidget->getSelectedIMTypes();
    if (selectedIMtypes->size()==0){
        errorMessage("Ground Motion Models: At least one intensity measure needs to be selected." );
        return false;
    }
    bool vectorIM = selectedIMtypes->size()>1;

    // Get the correlation model Json object
    QJsonObject spatCorrObj;
    if (!theSpatialCorrWidget->outputToJSON(spatCorrObj))
        return false;
    // Get the intensity measure Json object
    QJsonObject IMObj;
    if (!theIntensityMeasureWidget->outputToJSON(IMObj))
        return false;


    QJsonObject GMPEObj;
    if (!theGMPE->outputToJSON(GMPEObj))
        return false;


    if (vectorIM){
        QSet<QString> intraEventCorrModels;
        for(int i = 0; i < selectedIMtypes->size(); i++){
            QString imType_i = selectedIMtypes->at(i);
            QJsonObject gmpe_i = GMPEObj[imType_i].toObject();
            GMPEObj.remove(imType_i);
            QJsonObject spatCorr_i = spatCorrObj[imType_i].toObject();
            spatCorrObj.remove(imType_i);
            QJsonObject IMObj_i = IMObj[imType_i].toObject();
            IMObj_i["GMPE"] = gmpe_i["Type"].toString();
            IMObj_i["InterEventCorr"] = spatCorr_i["InterEventCorr"].toString();
            IMObj_i["IntraEventCorr"] = spatCorr_i["IntraEventCorr"].toString();
            IMObj[imType_i] = IMObj_i;
            intraEventCorrModels.insert(spatCorr_i["IntraEventCorr"].toString());
        }
        if (intraEventCorrModels.size()>1){
            QMessageBox msgBox;

            msgBox.setText("Warning:\n"
                           "Different Intra-event Spatial Correlation models are selected, and the cross correlation of the intra-event residuals between Intensity Measures can not be captured.\n"
                           "Continue running?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

            auto res = msgBox.exec();

            if(res != QMessageBox::Yes)
                return false;
//            errorMessage("Warning: The cross correlation of the intra-event residuals between Intensity Measures can not be captured if different intra-event spatial correlation models are chosen.");
        }
        GMPEObj.insert("Type", "Vector");
        IMObj.insert("Type", "Vector");
        spatCorrObj.insert("Type", "Vector");
    }
    obj.insert("CorrelationModel", spatCorrObj);
    obj.insert("IntensityMeasure", IMObj);
    obj.insert("GMPE", GMPEObj);

    return true;
}

//void GroundMotionModelsWidget::addGMMforSA(bool SAenabled){

//}

