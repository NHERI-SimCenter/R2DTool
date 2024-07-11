#ifndef LandslideBrayMacedo2019_H
#define LandslideBrayMacedo2019_H

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

// Written by: Jinyan Zhao

#include "SimCenterAppWidget.h"
#include "SC_DoubleLineEdit.h"

class OpenQuakeScenario;

class QComboBox;
class QLineEdit;
class QPushButton;
class CRSSelectionWidget;
class QSignalMapper;
class SC_ComboBox;
class QGroupBox;
class QCheckBox;

class LandslideBrayMacedo2019 : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit LandslideBrayMacedo2019(QWidget *parent = nullptr);
    bool outputToJSON(QJsonObject &jsonObject);

signals:

public slots:
    void loadFile(QString fieldKey);
    void setDefaultFilePath();
    void handleInputTypeChanged();
private:

    QLineEdit* DistWaterFilenameLineEdit = nullptr;
    QLineEdit* DistCoastFilenameLineEdit = nullptr;
    QLineEdit* DistRiverFilenameLineEdit = nullptr;
    QLineEdit* GwDepthFilenameLineEdit = nullptr;
    QLineEdit* PrecipitationFilenameLineEdit = nullptr;
    CRSSelectionWidget* crsSelectorWidget = nullptr;


    QPushButton* DistWaterBrowseFileButton;
    QPushButton* DistCoastBrowseFileButton;
    QPushButton* DistRiverBrowseFileButton;
    QPushButton* GwDepthBrowseFileButton;
    QPushButton* PrecipitationBrowseFileButton;
    QPushButton* resetToDefaultButton;

    QString DistWaterFilePath;
    QString DistCoastFilePath;
    QString DistRiverFilePath;
    QString GwDepthFilePath;
    QString PrecipitationFilePath;

    SC_ComboBox* DistWaterComboBox = nullptr;
    SC_ComboBox* DistCoastComboBox = nullptr;
    SC_ComboBox* DistRiverComboBox = nullptr;
    SC_ComboBox* GwDepthComboBox = nullptr;
    SC_ComboBox* PrecipitationComboBox = nullptr;

    QGroupBox* outputSaveGroupBox = nullptr;

    QMap<QString, QCheckBox*> outputSaveCheckBoxes;

    void setupConnections();
//    void setRupFile(QString dirPath);

};

#endif // LandslideBrayMacedo2019_H
