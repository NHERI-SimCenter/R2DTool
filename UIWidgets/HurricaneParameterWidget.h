#ifndef HURRICANEPARAMETERWIDGET_H
#define HURRICANEPARAMETERWIDGET_H
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

#include <QWidget>

class QLineEdit;
class QComboBox;

class HurricaneParameterWidget : public QWidget
{
    Q_OBJECT

public:
    HurricaneParameterWidget(QWidget* parent = nullptr);

    void setLandfallLat(const double val);
    void setLandfallLon(const double val);
    void setLandfallPress(const double val);
    void setLandingAngle(const double val);
    void setLandfallSpeed(const double val);
    void setLandfallRadius(const double val);

    void clear();

    QJsonObject getEventJson(void);
    QJsonObject getLandfallParamsJson(void);

private:

    QLineEdit* latLandfallLineEdit;
    QLineEdit* lonLandfallLineEdit;
    QLineEdit* angleLandfallLineEdit;
    QLineEdit* pressLandfallLineEdit;
    QLineEdit* speedLandfallLineEdit;
    QLineEdit* radiusLandfallLineEdit;

    QLineEdit* latLandfallPerturbLineEdit;
    QLineEdit* lonLandfallPerturbLineEdit;
    QLineEdit* angleLandfallPerturbLineEdit;
    QLineEdit* pressLandfallPerturbLineEdit;
    QLineEdit* speedLandfallPerturbLineEdit;
    QLineEdit* radiusLandfallPerturbLineEdit;
    QLineEdit* refHeightLineEdit;
    QLineEdit* gustLineEdit;
    QComboBox* exposureComboBox;
};

#endif // HURRICANEPARAMETERWIDGET_H
