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

// Written: Frank McKenna

#include "CustomizedItemModel.h"
#include "SecondaryComponentSelection.h"

#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <QTreeView>
#include <QVBoxLayout>

SecondaryComponentSelection::SecondaryComponentSelection(QWidget *parent)
    :QWidget(parent), currentIndex(-1), numHidden(0)
{
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

    horizontalLayout->setMargin(0);
    theStackedWidget = new QStackedWidget();

    horizontalLayout->addWidget(theSelectionWidget);
    horizontalLayout->addWidget(theStackedWidget);

    this->setLayout(horizontalLayout);

    // nothing added, do not display
    theSelectionWidget->setHidden(true);
    this->setHidden(true);
}


SecondaryComponentSelection::~SecondaryComponentSelection()
{
    QLayout *layout = this->layout();
    layout->removeWidget(theStackedWidget);
    theStackedWidget->setParent(NULL);
}


bool SecondaryComponentSelection::addComponent(QString text, QWidget *theWidget)
{
    if (textIndices.indexOf(text) == -1) {
        QPushButton *theItem = new QPushButton();
        theItem->setText(text);
        theSelectionLayout->insertWidget(textIndices.length()+1, theItem); // 1 is for spacer, spacer is because top push not always working!
        theStackedWidget->addWidget(theWidget);
        textIndices.append(text);
        thePushButtons.append(theItem);

        // check if can display self and component selection
        int numButtons = thePushButtons.length();
        if (numButtons == 1)
            this->setHidden(false);
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


void SecondaryComponentSelection::selectionChangedSlot(const QString &selectedText)
{
    //
    // find text in list
    //

    int stackIndex = textIndices.lastIndexOf(selectedText);

    //
    // get stacked widget to display current if of course it exists
    //

    if (stackIndex != -1)
        theStackedWidget->setCurrentIndex(stackIndex);
}


QWidget * SecondaryComponentSelection::swapComponent(QString text, QWidget *theWidget)
{
    QWidget *theRes = NULL;

    //
    // find text iin list
    //

    int index = textIndices.indexOf(text);
    //
    // get stacked widget to display current if of course it exists
    //

    if (index != -1) {
        theRes=theStackedWidget->widget(index);
        if (theRes != NULL) {
            theStackedWidget->removeWidget(theRes);
        }
        theStackedWidget->insertWidget(index, theWidget);
    }
    return theRes;
}


bool SecondaryComponentSelection::displayComponent(QString text)
{
    //
    // find index of text in list and display corresponding widget if index found
    //

    int index = textIndices.indexOf(text);

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


void SecondaryComponentSelection::setWidth(const int width)
{
    theSelectionWidget->setMaximumWidth(width);
    theSelectionWidget->setMinimumWidth(width);
}


void SecondaryComponentSelection::setItemWidthHeight(const int width, const int height)
{
    /*
     *
    auto customModel = dynamic_cast<CustomizedItemModel*>(standardModel);

    if(customModel)
    {
        customModel->setItemWidthHeight(width,height);
    }
    */
}


void SecondaryComponentSelection::hideAll(){
    int length = thePushButtons.length();
    numHidden = length;
    for (int i=0; i<length; i++) {
        QPushButton *theButton = thePushButtons.at(i);
        theButton->hide();
    }
    theSelectionWidget->setHidden(true);
    this->setHidden(true);
}


bool SecondaryComponentSelection::hide(QString text){

    // find index
    int index = textIndices.indexOf(text);

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

            if (numHidden = thePushButtons.length()) {
                this->setHidden(true);
            } else {
                for (int i = 0; i<numButtons; i++) {
                    if (thePushButtons.at(i)->isHidden() == false) {
                        this->displayComponent(textIndices.at(i));
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


bool SecondaryComponentSelection::show(QString text)
{
    // find index
    int index = textIndices.indexOf(text);

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
            this->setHidden(false);
        if (numDisplayed > 1)
            theSelectionWidget->setHidden(false);

        this->displayComponent(text);
    }

    return true;
}


