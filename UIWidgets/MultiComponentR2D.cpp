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

// Written by: Frank McKenna

#include "MultiComponentR2D.h"
#include "SecondaryComponentSelection.h"
#include "sectiontitle.h"
#include "VisualizationWidget.h"

// Qt headers
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QPushButton>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QJsonDocument>

// A class acting for secondary level R2D menu items
// whose display is dependent on the selection in GI
// of assets selected. What is actually displayed? that
// functionality is performed by the SecondaryComponentSelection object

// NOTE: The GI interacts through the connection
// of signals from checkboxes with slot in WorkflowAppR2D.
// That method calls each MultiComponentR2D with a show or a hide


MultiComponentR2D::MultiComponentR2D(QString key, QWidget *parent)
:SimCenterAppWidget(parent), numHidden(0)
{

  jsonKeyword = key;
  
    // HBox Layout for widget

    //
    // leftside selection, VBoxLayout with PushButtons
    //
    theMainLayout = new QVBoxLayout(this);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    theSelectionWidget = new QFrame();
    theSelectionLayout = new QVBoxLayout();

    theSelectionWidget->setObjectName("SCS");
    const QString STYLE_SHEET = "QFrame#SCS {\n"
                                "    border: 0px solid gray;\n"
                                "    border-left: 1px;\n"
                                "    border-right: 0px solid black;\n"
                                "    border-top: 0px solid black;\n"
                                "    border-bottom: 0px solid blacj;\n"
                                "    border-radius: 0px;\n"
                                "}\n";
    theSelectionWidget->setStyleSheet(STYLE_SHEET);

    horizontalLayout->setContentsMargins(0,5,5,5);
    horizontalLayout->setSpacing(0);

    theSelectionWidget->setLayout(theSelectionLayout);
    QSpacerItem *spacer = new QSpacerItem(10,10);
    theSelectionLayout->addSpacerItem(spacer);
    theSelectionLayout->addStretch();

    horizontalLayout->addWidget(theSelectionWidget);

    //
    // right side of widget is a stacked widget
    //

    theStackedWidget = new QStackedWidget();
    horizontalLayout->addWidget(theStackedWidget);

    // SG add
    connect(theStackedWidget,&QStackedWidget::currentChanged,this,[=](int idx){

        if (idx == -1)
            return;

        if(idx<=theNames.size()-1)
            emit selectionChangedSignal(theNames.at(idx));
    });

    theMainLayout->addLayout(horizontalLayout);

    // nothing yet added, do not display anything!
    theSelectionWidget->setHidden(true);
    theStackedWidget->setHidden(true);
}


MultiComponentR2D::~MultiComponentR2D()
{

}


bool MultiComponentR2D::outputToJSON(QJsonObject &jsonObject)
{
    bool res = true;
    int length = theNames.length();
    QJsonObject dataObj;
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->outputToJSON(dataObj);
            // qDebug() << __PRETTY_FUNCTION__ << jsonObject;
            if (res1 != true) {
                res = false;
            } 
        }
    }
    jsonObject[jsonKeyword] = dataObj;
    
    return res;
}


bool MultiComponentR2D::inputFromJSON(QJsonObject &jsonObject)
{
  if (!jsonObject.contains(jsonKeyword)) {
    QString errorMsg(QString("MultiComponentR2D keyWord: ") + jsonKeyword +
		     QString(" not in json"));
    errorMessage(errorMsg);
    return false;
  }
  
  QJsonObject dataObj = jsonObject[jsonKeyword].toObject();  
  bool res = true;
  int length = theNames.length();
  for (int i =0; i<length; i++) {
    QPushButton *theButton = thePushButtons.at(i);
    if (theButton->isHidden() == false) {
      SimCenterAppWidget *theWidget = theComponents.at(i);
      bool res1 = theWidget->inputFromJSON(dataObj);
      if (res1 != true) {
	res = false;
      }
    }
  }
  return res;
}


QList<QString> MultiComponentR2D::getActiveComponents(void)
{
    QList<QString> activeComponents;
    int length = theNames.length();
    for (int i =0; i<length; ++i) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            activeComponents.append(theNames.at(i));
        }
    }

    return activeComponents;
}


bool MultiComponentR2D::outputAppDataToJSON(QJsonObject &jsonObject)
{
  //    errorMessage(jsonKeyword);

    bool res = true;
    int length = theNames.length();
    QJsonObject dataObj;
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->outputAppDataToJSON(dataObj);
            // qDebug() << __PRETTY_FUNCTION__ << jsonObject;
            if (res1 != true) {
                res = false;
            } 
        }
    }

    jsonObject[jsonKeyword] = dataObj;

    return res;
}


bool MultiComponentR2D::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    QJsonObject dataObj;
    if (jsonObject.contains(jsonKeyword)) {
        dataObj = jsonObject[jsonKeyword].toObject();
    }
    else if (jsonObject.contains("Application")) {
        auto appName = jsonObject["Application"].toString();
        if(appName != jsonKeyword)
        {
            auto  errMsg = QString(QString("The application name ") + appName + QString(" does not match the json keyword ") + jsonKeyword);
            errorMessage(errMsg);
            return false;
        }
        dataObj = jsonObject["ApplicationData"].toObject();

    }
    else {
            QString errorMsg(QString("MultiComponentR2D::inputAppData keyWord: ")
                             + jsonKeyword + QString(" not in json, nor does it contain an application field"));
            errorMessage(errorMsg);
            return false;
    }

    
    bool res = true;
    int length = theNames.length();
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->inputAppDataFromJSON(dataObj);
            if (res1 != true) {
                res = false;
            }
        }
    }
    return res;
}


bool MultiComponentR2D::copyFiles(QString &destDir)
{
    bool res = true;
    int length = theNames.length();
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->copyFiles(destDir);
            if (res1 != true) {
                res = false;
            }
        }
    }
    return res;
}


bool MultiComponentR2D::addComponent(QString text, SimCenterAppWidget *theComponent)
{
    if (theNames.indexOf(text) == -1) {


        QPushButton *theItem = new QPushButton();
        theItem->setText(text);
        theSelectionLayout->insertWidget(theNames.length()+1, theItem); // 1 is for spacer, spacer is because top push not always working!
        thePushButtons.append(theItem);
        theNames.append(text);
        theComponents.append(theComponent);

        theStackedWidget->addWidget(theComponent);

        // check if can display self and component selection
        int numButtons = thePushButtons.length();
        if (numButtons == 1)
            theStackedWidget->setHidden(false);
        else if (numButtons > 1)
            theSelectionWidget->setHidden(false);

        connect(theItem, &QPushButton::clicked, this, [=](){
            this->displayComponent(text);
        });

        theComponent->setProperty("ComponentText",text);

        return true;
    } else
        qDebug() << "ComponentSelection: text: " << text << " option already exists";

    return false;
}


SimCenterAppWidget * MultiComponentR2D::getComponent(QString text)
{
    if (theNames.indexOf(text) != -1)
        return theComponents.at(theNames.indexOf(text));
    else
        return NULL;
}


SimCenterAppWidget* MultiComponentR2D::getCurrentComponent(void)
{
    auto currentIndex = theStackedWidget->currentIndex();
    if(currentIndex >= 0 && currentIndex <= theComponents.size()-1)
        return theComponents.at(currentIndex);

    return nullptr;
}


void MultiComponentR2D::hideAll()
{
    this->hideSelectionWidget();
    theStackedWidget->setHidden(true);
}


void MultiComponentR2D::hideSelectionWidget(void)
{
    int length = thePushButtons.length();
    numHidden = length;
    for (int i=0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        theButton->hide();
    }
    theSelectionWidget->setHidden(true);
}


bool MultiComponentR2D::hide(QString text) {
    // find index
    int index = theNames.indexOf(text);

    // if not 0, hide it
    if (index != -1) {

        QPushButton *theButton = thePushButtons.at(index);
        if (theButton->isHidden() == false) {
            theButton->setHidden(true);
            numHidden++;
        }

        int numButtons = thePushButtons.length();

        // if currently displayed, show something else or nothing if all hidden!
        if (theStackedWidget->currentIndex() == index) {
            if (numHidden == thePushButtons.length()) {
                theSelectionWidget->setHidden(true);
                theStackedWidget->setHidden(true);
            } else {
                for (int i = 0; i<numButtons; i++) {
                    if (thePushButtons.at(i)->isHidden() == false) {
                        this->displayComponent(theNames.at(i));
                        i = numButtons;
                    }
                }
            }
        }

        if ((numButtons - numHidden) == 1) {
            theSelectionWidget->hide();
        }
    }
    return true;
}


bool MultiComponentR2D::show(QString text) {
    // find index
    int index = theNames.indexOf(text);

    // if not 0, hide it
    if (index != -1) {
        QPushButton *theButton = thePushButtons.at(index);
        if (theButton->isHidden() == true) {
            theButton->setHidden(false);
            numHidden--;
        }

        int numButtons = thePushButtons.length();
        int numDisplayed = numButtons - numHidden;
        if (numDisplayed > 0)
            theStackedWidget->setHidden(false);
        if (numDisplayed > 1)
            theSelectionWidget->setHidden(false);

        this->displayComponent(text);

        return true;
    }

    return false;
}


bool MultiComponentR2D::displayComponent(QString text)
{
    //
    // find index of text in list and display corresponding widget if index found
    //

    int index = theNames.indexOf(text);

    if (index != -1) {

        auto currIndex = theStackedWidget->currentIndex();

        if (index == currIndex)
            return true;

        theStackedWidget->setCurrentIndex(index);

        QPushButton *theItem = thePushButtons.at(index);
        theItem->setStyleSheet("background-color: rgb(63, 147, 168);");

        if (theStackedWidget->currentIndex() != -1) {

            QPushButton *theOldItem = thePushButtons.at(currIndex);
            theOldItem->setStyleSheet("background-color: rgb(79, 83, 89);");
        }

        return true;
    }

    return false;
}


int MultiComponentR2D::getCurrentIndex(void) const
{
    return theStackedWidget->currentIndex();
}


int MultiComponentR2D::getIndexOfComponent(QString text) const
{
    int index = theNames.indexOf(text);

    return index;
}


void MultiComponentR2D::selectionChangedSlot(const QString &selectedText)
{
    //
    // find text in list
    //

    int stackIndex = theNames.lastIndexOf(selectedText);

    //
    // get stacked widget to display current if of course it exists
    //

    if (stackIndex != -1)
        theStackedWidget->setCurrentIndex(stackIndex);
}


void MultiComponentR2D::clear(void)
{
    foreach (auto&& comp, theComponents)
        comp->clear();
}


void MultiComponentR2D::removeAllComponents(void)
{

    while (theStackedWidget->count() > 0) {
        QWidget *widget = theStackedWidget->widget(0);
        theStackedWidget->removeWidget(widget);
        delete widget;
    }

    theNames.clear();
    theComponents.clear();
    qDeleteAll(thePushButtons);
    thePushButtons.clear();
}
