#ifndef ComponentInputWidget_H
#define ComponentInputWidget_H
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
#include "ComponentDatabase.h"
#include "VisualizationWidget.h"

#include <set>

#include <QString>
#include <QObject>

class AssetInputDelegate;

class QGroupBox;
class QLineEdit;
class QTableWidget;
class QLabel;

class ComponentInputWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit ComponentInputWidget(QWidget *parent, QString componentType, QString appType = QString());
    virtual ~ComponentInputWidget();

    QGroupBox* getComponentsWidget(void);

    QTableWidget *getTableWidget() const;

    void insertSelectedComponent(const int ComponentID);

    int numberComponentsSelected(void);

    ComponentDatabase* getComponentDatabase();

    // Set custom labels in widget
    void setComponentType(const QString &value);
    void setLabel1(const QString &value);
    void setLabel2(const QString &value);
    void setLabel3(const QString &value);  
    void setGroupBoxText(const QString &value);

    void loadFileFromPath(QString& path);

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destName);

    QString getPathToComponentFile(void) const;

    void clear(void);

    void setTheVisualizationWidget(VisualizationWidget *value);

signals:
    void componentDataLoaded();

public slots:
    void handleComponentSelection(void);
    void handleCellChanged(int row, int column);

private slots:
    void selectComponents(void);
    void loadComponentData(void);
    void chooseComponentInfoFileDialog(void);
    void clearComponentSelection(void);

private:
    QString pathToComponentInfoFile;
    QLineEdit* componentFileLineEdit;
    AssetInputDelegate* selectComponentsLineEdit;
    QTableWidget* componentTableWidget;
    QLabel* componentInfoText;
    QGroupBox* componentGroupBox;

    QString appType;
    QString componentType;
    QString label1;
    QString label2;
    QString label3;

    void createComponentsBox(void);

    ComponentDatabase theComponentDb;
    VisualizationWidget* theVisualizationWidget;

};

#endif // ComponentInputWidget_H
