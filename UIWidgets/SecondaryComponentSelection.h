#ifndef SECONDARY_COMPONENT_SELECTION_H
#define SECONDARY_COMPONENT_SELECTION_H

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

/**
 *  @author  fmckenna
 *  @date    09/2018
 *  @version 1.0
 *
 *  @section DESCRIPTION
 *
 * The purpose of this class is to define interface for SimCenterComponentSelection. These are
 * selection widgets that provide the user a choice of options and display a window based on that * choice. They are used in workflow applications. Functionality used in many SimCenter clases.
 */

#include <QWidget>
#include <QModelIndex>
#include <QList>
#include <QString>

class QStackedWidget;
class QVBoxLayout;
class QFrame;
class QPushButton;



class SecondaryComponentSelection : public QWidget
{
    Q_OBJECT
public:
    explicit SecondaryComponentSelection(QWidget *parent = 0);
    virtual ~SecondaryComponentSelection();

    /** 
     *   @brief addComponent method to add component to selection options.
     *   @param QString text to appear in selection
     *   @param SecondaryAppWidget * the SecondaryAppWidget to appear when selection made
     *   @return bool true or false indicating success or failure to add
     */  

    virtual bool addComponent(QString text, QWidget *theWidget);

    /** 
     *   @brief swapComponent method to add component to selection options.
     *   @param QString text associated with widget to be swapped
     *   @param SecondaryAppWidget * the SecondaryAppWidget to appear when selection made
     *   @return QWidget * - old widget or NULL if not found
     */  

    virtual QWidget *swapComponent(QString text, QWidget *theWidget);

    /** 
     *   @brief swapComponent method to add component to selection options.
     *   @param QString text associated with widget to be swapped
     *   @param SecondaryAppWidget * the SecondaryAppWidget to appear when selection made
     *   @return QWidget * - old widget or NULL if not found
     */  

    virtual bool displayComponent(QString text);

    /** 
     *   @brief displayComponent method to display a component
     *   @param QString text associated with widget to be displayed
     *   @return bool true or false indicating success or failure to add
     */  

    virtual void hideAll();
    virtual bool hide(QString text);
    virtual bool show(QString text);

    /**
     *   @brief Function to set the width on the selection tree widget
     *   @param int width of the widget
     */
    void setWidth(const int width);

    /**
     *   @brief Function to set the width and height of an item on the selection tree widget
     *   @param int width of a treeview item
     *   @param int height of a treeview item
     */
    void setItemWidthHeight(const int width, const int height);

signals:

public slots:

    /**
     *   @brief selectionChangedSlot slot invoked when slection is changed.
     *   @param QItemSelection new item selected
     */
    void selectionChangedSlot(const QString &);
private:
    int currentIndex;
    int numHidden;
    QVBoxLayout *theSelectionLayout;
    QFrame *theSelectionWidget;
    QStackedWidget *theStackedWidget;
    QList<QString> textIndices;
    QList<QPushButton *>thePushButtons;
};

#endif // SECONDARY_COMPONENT_SELECTION_H
