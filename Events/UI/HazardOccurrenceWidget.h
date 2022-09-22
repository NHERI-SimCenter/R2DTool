#ifndef HazardOccurrenceWidget_H
#define HazardOccurrenceWidget_H
/* *****************************************************************************
Copyright (c) 2016-2022, The Regents of the University of California (Regents).
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

// Written by: Kuanshi Zhong

#include <QWidget>

class HazardOccurrence;

class QComboBox;
class QLineEdit;
class QDoubleSpinBox;
class QPushButton;
class QRegExpValidator;
class QLabel;

class HazardOccurrenceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HazardOccurrenceWidget(QWidget *parent = nullptr);

    HazardOccurrence* getRuptureSource() const;

signals:

public slots:

    void setHCType(const QString value);
    void setIMType(const QString value);
    void setHCFile(const QString value);
    QString checkReturnPeriodsValid(const QString& input) const;
    void commitReturnPeriods();
    void handleTypeChanged(const QString &val);
    void loadHazardCurveFile();

private:
    HazardOccurrence* m_eqRupture;
    QComboBox* ModelTypeCombo;
    QLineEdit* candidEQLineEdit;
    QDoubleSpinBox* m_magnitudeMinBox;
    QDoubleSpinBox* m_magnitudeMaxBox;
    QDoubleSpinBox* m_maxDistanceBox;

    // number of scenarios
    QLineEdit* NumScenarioLineEdit;
    // number of ground motion maps
    QLineEdit* NumGMMapLineEdit;
    // Hazard occurrence model
    QComboBox* HOModelTypeCombo;
    // Hazard occurrence model
    QComboBox* HCTypeCombo;
    QLineEdit* FilenameLineEdit;
    QString HazardCurveFile;
    QPushButton* browseFileButton;
    // Hazard curve edition
    QComboBox* NSHM_Edition_Combo;
    // intensity measure type
    QComboBox* IMT_Combo;
    // period lineedit
    QLabel* IMT_period;
    QLineEdit* PeriodEdit;
    // return periods
    QLineEdit* return_periods_lineEdit;
    QRegExpValidator* LEValidator;

    void setupConnections();

};

#endif // HazardOccurrenceWidget_H
