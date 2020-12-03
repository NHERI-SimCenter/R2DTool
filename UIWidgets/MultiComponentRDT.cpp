/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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
// Latest revision: 10.08.2020

#include "MultiComponentRDT.h"
#include "ComponentInputWidget.h"
#include "VisualizationWidget.h"
#include "sectiontitle.h"
#include "SecondaryComponentSelection.h"

// Qt headers
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
#include <QColorTransform>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QCheckBox>
#include "SecondaryComponentSelection.h"

MultiComponentRDT::MultiComponentRDT(QWidget *parent)
    : SimCenterAppWidget(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->setMargin(0);
  
  theSelection = new SecondaryComponentSelection(this);
  mainLayout->addWidget(theSelection);

  theSelection->setWidth(120);
  this->setLayout(mainLayout);
}


MultiComponentRDT::~MultiComponentRDT()
{

}


bool MultiComponentRDT::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool MultiComponentRDT::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool MultiComponentRDT::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool MultiComponentRDT::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool MultiComponentRDT::copyFiles(QString &destDir)
{
    return false;
}


bool
MultiComponentRDT::addComponent(QString text, SimCenterAppWidget *theComponent)
{
    if (theNames.indexOf(text) == -1) {
      theNames.append(text);
      theComponents.append(theComponent);
      theSelection->addComponent(text, theComponent);
    }
}

SimCenterAppWidget *
MultiComponentRDT::getComponent(QString text)
{
    if (theNames.indexOf(text) != -1) 
      return theComponents.at(theNames.indexOf(text));
    else
      return NULL;
}

void MultiComponentRDT::hideAll()
{
  return theSelection->hideAll();
}

bool
MultiComponentRDT::hide(QString text) {
  qDebug() << "MULTI::hide " << text;
  return theSelection->hide(text);
}

bool
MultiComponentRDT::show(QString text) {
     qDebug() << "MULTI::show " << text;
  return theSelection->show(text);
}

