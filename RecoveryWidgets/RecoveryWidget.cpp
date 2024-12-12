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

// Written by: Sina Naeimi

#include "RecoveryWidget.h"
#include "NoneWidget.h"
#include "Pyrecodes.h"
#include "PyrecodesUI.h"

#include "SecondaryComponentSelection.h"
#include "VisualizationWidget.h"
#include "SectionTitle.h"

// Qt headers
#include <QDebug>

RecoveryWidget::RecoveryWidget(QWidget *parent)
  : SimCenterAppSelection(QString("Recovery"), QString("Recovery"), parent)
{
  // Create the widgets    
  SimCenterAppWidget *noneWidget3 = new NoneWidget(this);
  SimCenterAppWidget *pyrecodesRecovery = new Pyrecodes(this);
  SimCenterAppWidget *pyrecodesUI_Recovery = new PyrecodesUI(this);    
  this->addComponent(QString("None"), QString("None"), noneWidget3);
  addComponent(QString("Pyrecodes"), QString("Pyrecodes"), pyrecodesRecovery);
  addComponent(QString("PyrecodesUI"), QString("PyrecodesUI"), pyrecodesUI_Recovery);      

  //addComponent("Recovery", RecWidget);
  // We shall not hide it since we want to have recovery at any time
  //this->hideAll();
}

RecoveryWidget::~RecoveryWidget()
{

}




