#ifndef GMPEWIDGET_H
#define GMPEWIDGET_H
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
#include <QLabel>

#include "GMPE.h"

class SC_ComboBox;
class QGroupBox;

class GMPEWidget : public SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit GMPEWidget(QStringList* selectedIMTypes, QWidget *parent = nullptr);

    bool outputToJSON(QJsonObject& obj);
    bool inputFromJSON(QJsonObject& obj);

signals:

public slots:
    void handleAvailableGMPE(const QString sourceType);
    void toggleIMselection(QStringList* selectedIMTypes);
private:
    GMPE* m_gmpe_intensity = new GMPE("intensity");
    GMPE* m_gmpe_duration = new GMPE("duration");

    SC_ComboBox* PGAtypeBox = nullptr;
    SC_ComboBox* PGVtypeBox = nullptr;
    SC_ComboBox* SAtypeBox = nullptr;
    SC_ComboBox* DS575HtypeBox = nullptr;
    SC_ComboBox* DS595HtypeBox = nullptr;

    QLabel* PGAtypeLabel = new QLabel(tr("PGA:"));
    QLabel* SAtypeLabel = new QLabel(tr("SA:"));
    QLabel* PGVtypeLabel = new QLabel(tr("PGV:"));
    QLabel* DS575HtypeLabel = new QLabel(tr("DS575H:"));
    QLabel* DS595HtypeLabel = new QLabel(tr("DS595H:"));

    QStringList* _selectedIMTypes;


    QGroupBox* gmpeGroupBox = nullptr;

    void setupConnections();
};

#endif // GMPEWIDGET_H
