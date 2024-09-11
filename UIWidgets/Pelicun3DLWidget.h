#ifndef Pelicun3DLWidget_H
#define Pelicun3DLWidget_H

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

#include "SimCenterAppWidget.h"
#include "Pelicun3PostProcessor.h"
#include "VisualizationWidget.h"
#include <QMainWindow>

class QComboBox;
class QCheckBox;
class QLineEdit;
class QHBoxLayout;
class QWidget;

class Pelicun3DLWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit Pelicun3DLWidget(QWidget *parent = nullptr);

    bool outputAppDataToJSON(QJsonObject &jsonObject);

    bool inputAppDataFromJSON(QJsonObject &jsonObject);

    void clear(void);

    bool copyFiles(QString &destName);

    bool recursiveCopy(const QString &sourcePath, const QString &destPath);

    bool outputCitation(QJsonObject &jsonObject);

    SC_ResultsWidget* getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType);

    QMainWindow* getPostProcessor(void);
public slots:

    void handleComboBoxChanged(const QString &text);
    void handleBrowseButton1Pressed(void);
    void handleBrowseButton2Pressed(void);

private:
    QWidget* autoPopulateScriptWidget;
    QWidget* customModelDirWidget;
    Pelicun3PostProcessor* resultWidget = nullptr;

    QComboBox* DLTypeComboBox;
    QLineEdit* realizationsLineEdit;
    QComboBox* eventTimeComboBox;
    QCheckBox* detailedResultsCheckBox;
    QCheckBox* logFileCheckBox;
    QCheckBox* coupledEDPCheckBox;
    QCheckBox* groundFailureCheckBox;
    QLineEdit* autoPopulationScriptLineEdit;
    QLineEdit* customModelDirLineEdit;
    void clearParams(void);
};

#endif // Pelicun3DLWidget_H
