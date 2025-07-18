#ifndef SpatialCorrelationWidget_H
#define SpatialCorrelationWidget_H
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "SimCenterAppWidget.h"
#include <QLabel>

class SC_ComboBox;
class QLineEdit;

class SpatialCorrelationWidget : public SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit SpatialCorrelationWidget(QStringList* selectedIMTypes, QWidget *parent = nullptr);

    bool outputToJSON(QJsonObject& obj);
    bool inputFromJSON(QJsonObject& obj);

signals:

public slots:
//    void handleAvailableModel(const QString sourceType);
    void toggleIMselection(QStringList* selectedIMTypes);

private:
    SC_ComboBox* PGAcorrelationBoxInter = nullptr;
    SC_ComboBox* PGAcorrelationBoxIntra = nullptr;
    SC_ComboBox* SAcorrelationBoxInter = nullptr;
    SC_ComboBox* SAcorrelationBoxIntra = nullptr;
    SC_ComboBox* PGVcorrelationBoxInter = nullptr;
    SC_ComboBox* PGVcorrelationBoxIntra = nullptr;
    SC_ComboBox* DS575HcorrelationBoxInter = nullptr;
    SC_ComboBox* DS575HcorrelationBoxIntra = nullptr;
    SC_ComboBox* DS595HcorrelationBoxInter = nullptr;
    SC_ComboBox* DS595HcorrelationBoxIntra = nullptr;

    QLabel* spatialCorrelationInterLabel;
    QLabel* spatialCorrelationIntraLabel;

    QStringList* _selectedIMTypes;
    QLabel* PGVtypeLabelInter = new QLabel(tr("PGV:"));
    QLabel* PGAtypeLabelInter = new QLabel(tr("PGA:"));
    QLabel* SAtypeLabelInter = new QLabel(tr("SA:"));
    QLabel* DS575HtypeLabelInter = new QLabel(tr("DS575H:"));
    QLabel* DS595HtypeLabelInter = new QLabel(tr("DS595H:"));

    QLabel* PGVtypeLabelIntra = new QLabel(tr("PGV:"));
    QLabel* PGAtypeLabelIntra = new QLabel(tr("PGA:"));
    QLabel* SAtypeLabelIntra = new QLabel(tr("SA:"));
    QLabel* DS575HtypeLabelIntra = new QLabel(tr("DS575H:"));
    QLabel* DS595HtypeLabelIntra = new QLabel(tr("DS595H:"));

};

#endif // SpatialCorrelationWidget_H
