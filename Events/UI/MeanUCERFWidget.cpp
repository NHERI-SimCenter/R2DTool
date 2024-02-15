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

#include "MeanUCERFWidget.h"
#include "MeanUCERFPoissonWidget.h"
#include "MeanUCERFFM3Widget.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>

MeanUCERFWidget::MeanUCERFWidget(QString jsonKey, QWidget *parent) : SimCenterAppSelection("Mean UCERF3 Presets",jsonKey,parent), jsonKey(jsonKey)
{
    this->setContentsMargins(0,0,0,0);

    poissonWidget = new MeanUCERFPoissonWidget();
    FM3P1Widget = new MeanUCERFFM3Widget();
    FM3P2Widget = new MeanUCERFFM3Widget();

    this->addComponent("(POISSON ONLY) Both FM Branch Averaged","Mean UCERF3",poissonWidget);
    this->addComponent("FM3.1 Branch Averaged","Mean UCERF3",FM3P1Widget);
    this->addComponent("FM3.2 Branch Averaged","Mean UCERF3",FM3P2Widget);

}


bool MeanUCERFWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool MeanUCERFWidget::outputToJSON(QJsonObject& obj)
{
    obj["Model"] = this->getCurrentSelectionName();
    SimCenterAppSelection::outputToJSON(obj);
    QJsonObject ModelParams = obj["ModelParameters"].toObject();
    ModelParams["preset"] = this->getCurrentComboName();
    obj["ModelParameters"] = ModelParams;
    return true;
}

