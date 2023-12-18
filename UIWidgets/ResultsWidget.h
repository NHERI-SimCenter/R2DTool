#ifndef ResultsWidget_H
#define ResultsWidget_H
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

#include "SimCenterAppWidget.h"
#include "QGISVisualizationWidget.h"
#include <QJsonArray>
#include "qgsproject.h"
class AssetInputDelegate;
class PelicunPostProcessor;
class VisualizationWidget;
class CBCitiesPostProcessor;


class QTabWidget;
class QStackedWidget;
class QVBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

class ResultsWidget : public SimCenterAppWidget
{

    Q_OBJECT
public:

    explicit ResultsWidget(QWidget *parent, VisualizationWidget* visWidget);
    virtual ~ResultsWidget();

    virtual bool outputToJSON(QJsonObject &rvObject);
    virtual bool inputFromJSON(QJsonObject &rvObject);

    virtual int processResults(QString resultsDir);

    void clear(void);

    void resultsShow(bool value);

private slots:

    int printToPDF(void);
    void selectComponents(void);
    void handleComponentSelection(void);
    void chooseResultsDirDialog(void);

private:

    QStackedWidget* mainStackedWidget = nullptr;
    QTabWidget* resTabWidget = nullptr;
    QLabel* resultsMainLabel = nullptr;

    QLabel* selectComponentsText = nullptr;
    QPushButton *selectComponentsButton = nullptr;
    QPushButton *exportPDFFileButton = nullptr;
    QPushButton *exportBrowseFileButton = nullptr;
    QLabel* exportLabel = nullptr;

    QString DVApp;
    QLineEdit* exportPathLineEdit = nullptr;
    QVBoxLayout* mainLayout = nullptr;
    QWidget* resultsPageWidget = nullptr;

    AssetInputDelegate* selectComponentsLineEdit = nullptr;
    QGISVisualizationWidget* theVisualizationWidget = nullptr;

    QWidget *theParent = nullptr;

    std::unique_ptr<PelicunPostProcessor> thePelicunPostProcessor;
    std::unique_ptr<CBCitiesPostProcessor> theCBCitiesPostProcessor;

};

#endif // ResultsWidget
