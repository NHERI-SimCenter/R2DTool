﻿#ifndef MultiComponentR2D_H
#define MultiComponentR2D_H
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

#include "SimCenterAppWidget.h"

class SecondaryComponentSelection;
class QFrame;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class MultiComponentR2D : public  SimCenterAppWidget
{
    Q_OBJECT

public:
  explicit MultiComponentR2D(QString jsonKeyword, QWidget *parent);
    ~MultiComponentR2D();

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destName);

    void clear(void);
    virtual void hideAll();
    virtual bool hide(QString text);
    virtual bool show(QString text);
    bool addComponent(QString text, SimCenterAppWidget *);
    SimCenterAppWidget *getComponent(QString text);
    void removeAllComponents(void);
    bool outputCitation(QJsonObject &jsonObject);
  
    // SG add
    SimCenterAppWidget* getCurrentComponent(void);
    void hideSelectionWidget(void);
    int getCurrentIndex(void) const;
    int getIndexOfComponent(QString text) const;

    // Returns a list of all the components that are active or 'on', i.e., theButton->isHidden() == false
    QList<QString> getActiveComponents(void);

public slots:
    void selectionChangedSlot(const QString &);

signals:
    void selectionChangedSignal(const QString &);

protected:
    virtual bool displayComponent(QString text);
    QVBoxLayout *theMainLayout = nullptr;
    QString jsonKeyword;

private:
    int numHidden;

    QFrame *theSelectionWidget = nullptr;
    QVBoxLayout *theSelectionLayout = nullptr;
    QStackedWidget *theStackedWidget = nullptr;

    QList<QString> theNames;
    QList<QPushButton *>thePushButtons;
    QList<SimCenterAppWidget *> theComponents;

    //  SecondaryComponentSelection *theSelection;
};

#endif // MultiComponentR2D_H
