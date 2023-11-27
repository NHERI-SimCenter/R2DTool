#ifndef SURROGATE_PY_FILTER_H
#define SURROGATE_PY_FILTER_H
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
#include "SC_FileEdit.h"
#include "SC_StringLineEdit.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QVector>
#include <QRadioButton>

//class InputWidgetParameters;

#include <string>
#include <sstream>
#include <iostream>
#include <QSpinBox>

using namespace std;

class SurrogateFile : public SimCenterWidget
{
    Q_OBJECT
public:
    explicit SurrogateFile(QWidget *parent = 0);
    ~SurrogateFile();

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    QRadioButton *button;  // used to mark if Event intended for deletion
    SC_StringLineEdit  *theFilePath;    // full path to file name
    SC_StringLineEdit  *theModelName;  // load factor

public slots:
    void chooseFileName(void);
    void onRemoveMod(bool);
    //void onAddMod(bool);

signals:
    void removeRecord();

};


class SurrogatePyFilter : public SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit SurrogatePyFilter(QWidget *parent = 0);
    ~SurrogatePyFilter();

    bool outputToJSON(QJsonObject &rvObject) override;
    bool inputFromJSON(QJsonObject &rvObject) override;
    bool outputAppDataToJSON(QJsonObject &rvObject) override;
    bool inputAppDataFromJSON(QJsonObject &rvObject) override;
    bool copyFiles(QString &dirName) override;

    QString getMainInput();
    QVBoxLayout *recordLayout;
    QRadioButton *button; // used to mark if Event intended for deletion
    QLineEdit *theName; // a QLineEdit with name of File (filename minus path and extension)
    QVector<SurrogateFile  *>theFiles;

    void clear(void);

public slots:

    void addMod(void);
    void removeMod(void);
    void removeAllMods(void);
    void loadModsFromDir(void);

private:
    SC_FileEdit *filterPath;

    QVBoxLayout *verticalLayout;
    QVBoxLayout *surLayout;

    QVector<SurrogateFile *>theModels;
};

#endif // SURROGATE_PY_FILTER_H
