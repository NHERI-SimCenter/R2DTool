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

// Written by: fmk
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
#include <QStackedLayout>
#include <QStackedWidget>


// A class acting for secondary level RDT menu items
// whose display is dependent on the selection in GI
// of assets selected. What is actually displayed? that
// functionality is performed by the SecondaryComponentSelection object

// NOTE: The GI interacts through the connection
// of signals from checkboxes with slot in WorkflowAppRDT. 
// That method calls each MultiComponentRDT with a show or a hide


MultiComponentRDT::MultiComponentRDT(QWidget *parent)
    :SimCenterAppWidget(parent), currentIndex(-1), numHidden(0)
{
    /*
  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->setMargin(0);
  
  theSelection = new SecondaryComponentSelection(this);
  mainLayout->addWidget(theSelection);

  theSelection->setWidth(120);
  this->setLayout(mainLayout);
  */

    // HBox Layout for widget

    //
    // leftside selection, VBoxLayout with PushButtons
    //

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    theSelectionWidget = new QFrame();
    theSelectionLayout = new QVBoxLayout();

    theSelectionWidget->setObjectName("SCS");
    const QString STYLE_SHEET = "QFrame#SCS {\n"
                                "    background-color: whitesmoke;\n"
                                "    border: 1px solid gray;\n"
                                "    border-left: 0px;\n"
                                "    border-right: 1px solid black;\n"
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

    // set this widgets layout
    this->setLayout(horizontalLayout);

    // nothing yet added, do not display anything!
    theSelectionWidget->setHidden(true);
    theStackedWidget->setHidden(true);
}


MultiComponentRDT::~MultiComponentRDT()
{

}


bool MultiComponentRDT::outputToJSON(QJsonObject &jsonObject)
{ 
    bool res = true;
    int length = theNames.length();
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->outputToJSON(jsonObject);
            if (res1 != true) {
                res = false;
            }
        }
    }
    return res;
}


bool MultiComponentRDT::inputFromJSON(QJsonObject &jsonObject)
{

    bool res = true;
    int length = theNames.length();
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->inputFromJSON(jsonObject);
            if (res1 != true) {
                res = false;
            }
        }
    }
    return res;

}


bool MultiComponentRDT::outputAppDataToJSON(QJsonObject &jsonObject)
{
    bool res = true;
    int length = theNames.length();
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->outputAppDataToJSON(jsonObject);
            if (res1 != true) {
                res = false;
            }
        }
    }
    return res;
}


bool MultiComponentRDT::inputAppDataFromJSON(QJsonObject &jsonObject)

{
    bool res = true;
    int length = theNames.length();
    for (int i =0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        if (theButton->isHidden() == false) {
            SimCenterAppWidget *theWidget = theComponents.at(i);
            bool res1 = theWidget->inputAppDataFromJSON(jsonObject);
            if (res1 != true) {
                res = false;
            }
        }
    }
    return res;
}



bool MultiComponentRDT::copyFiles(QString &destDir)
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


bool
MultiComponentRDT::addComponent(QString text, SimCenterAppWidget *theComponent)
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

        return true;
    } else
        qDebug() << "ComponentSelection: text: " << text << " option already exists";

    return false;
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
    int length = thePushButtons.length();
    numHidden = length;
    for (int i=0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        theButton->hide();
    }
    theSelectionWidget->setHidden(true);
    theStackedWidget->setHidden(true);
}

bool
MultiComponentRDT::hide(QString text) {
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
        if (currentIndex == index) {
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

bool
MultiComponentRDT::show(QString text) {
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
    }

    return true;
}

bool
MultiComponentRDT::displayComponent(QString text)
{
    //
    // find index of text in list and display corresponding widget if index found
    //

    int index = theNames.indexOf(text);

    if (index != -1 && index != currentIndex) {
        theStackedWidget->setCurrentIndex(index);

        QPushButton *theItem = thePushButtons.at(index);
        theItem->setStyleSheet("background-color: rgb(63, 147, 168);");
        if (currentIndex != -1) {

            QPushButton *theOldItem = thePushButtons.at(currentIndex);
            theOldItem->setStyleSheet("background-color: #64B5F6;");
        }
        currentIndex = index;
        return true;
    }

    return false;
}

void
MultiComponentRDT::selectionChangedSlot(const QString &selectedText)
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

