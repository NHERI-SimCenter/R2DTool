#ifndef OpenQuakeClassicalWidget_H
#define OpenQuakeClassicalWidget_H

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

// Written by: Kuanshi Zhong

#include "SimCenterWidget.h"

class OpenQuakeClassical;

class QComboBox;
class QLineEdit;
class QPushButton;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;

class OpenQuakeClassicalWidget : public SimCenterWidget
{
    Q_OBJECT

public:
    explicit OpenQuakeClassicalWidget(QWidget *parent = nullptr);

    OpenQuakeClassical* getRuptureSource() const;

signals:

public slots:
    void loadSourceLogicTree();
    void loadGMPELogicTree();
    void loadSourceDir();
    void handleOQVersionChanged(const QString& selection);

private:
    OpenQuakeClassical* m_eqRupture;
    QComboBox* ModelTypeCombo;
    QLineEdit* sourceFilenameLineEdit;
    QLineEdit* gmpeFilenameLineEdit;
    QLineEdit* sourceDirLineEdit;
    QString sourceFilePath;
    QString gmpeFilePath;
    QString sourceDir;
    QDoubleSpinBox* m_rupMeshBox;
    QDoubleSpinBox* m_areaMeshBox;
    QDoubleSpinBox* m_maxDistanceBox;
    QPushButton* browseSourceFileButton;
    QPushButton* browseGMPEFileButton;
    QPushButton* browseSourceDirButton;

    QLineEdit* rpLineEdit;
    QLineEdit* randSeedLineEdit;
    QLineEdit* quantLineEdit;
    QLineEdit* ivgtTimeLineEdit;

    QCheckBox* indivHCCheckBox;
    QCheckBox* meanHCCheckBox;
    QCheckBox* hazMapCheckBox;
    QCheckBox* UHSCheckBox;

    QComboBox* oqVersionCombo;
    QString oqVersion = "3.17.1";

    void setupConnections();
    void setSourceFile(QString dirPath);
    void setGMPEFile(QString dirPath);
    void setSourceDir(QString dirPath);

};

#endif // OpenQuakeClassicalWidget_H
