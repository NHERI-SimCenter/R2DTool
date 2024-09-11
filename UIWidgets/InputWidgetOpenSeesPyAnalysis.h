#ifndef INPUT_WIDGET_OPENSEESPY_ANALYSIS_H
#define INPUT_WIDGET_OPENSEESPY_ANALYSIS_H
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

// Written by: Frank McKenna, Stevan Gavrilovic

#include "SimCenterAppWidget.h"

class QLineEdit;
class QComboBox;
class QStackedWidget;

class InputWidgetOpenSeesPyAnalysis : public  SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit InputWidgetOpenSeesPyAnalysis(QWidget *parent = 0);
    ~InputWidgetOpenSeesPyAnalysis();

    bool outputToJSON(QJsonObject &jsonObject) override;
    bool inputFromJSON(QJsonObject &jsonObject) override;
    bool outputAppDataToJSON(QJsonObject &jsonObject) override;
    bool inputAppDataFromJSON(QJsonObject &jsonObject) override;
    bool copyFiles(QString &dirName) override;
    bool outputCitation(QJsonObject &jsonObject) override;

signals:

public slots:
   void clear(void) override;
   void chooseFileName(void);
   void dampingEditingFinished();
   void toleranceEditingFinished();

   void changedDampingMethod(QString);

private:
    //QLineEdit   *theTolerance;
    QLineEdit   *theAnalysis;
    QLineEdit   *theConvergenceTest;
    //QLineEdit   *theAlgorithm;
    QComboBox   *theAlgorithm;
    QLineEdit   *theIntegration;
    QLineEdit   *theSolver;
    QLineEdit *file;

    QString lastDampingRatio;
    QString lastTolerance;

    QStackedWidget *theStackedWidget;
    QComboBox *theSelectionBox;

    QLineEdit *dampingRatio;
    QLineEdit *firstMode;
    QLineEdit *secondMode;
    QComboBox *theRayleighStiffness;

    QLineEdit *numModesModal;
    QLineEdit *dampingRatioModal;
    QLineEdit *dampingRatioModalTangent;
};

#endif // INPUTWIDGET_OPENSEESPY_ANALYSIS_H
