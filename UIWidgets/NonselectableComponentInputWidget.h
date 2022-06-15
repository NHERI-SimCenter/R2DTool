#ifndef NonselectableComponentInputWidget_H
#define NonselectableComponentInputWidget_H
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

#include <set>

#include <QString>
#include <QObject>

class ComponentTableView;

class QgsFeature;
class QGISVisualizationWidget;
class QgsVectorLayer;
class QgsGeometry;

class QGroupBox;
class QLineEdit;
class QTableWidget;
class QLabel;


class NonselectableComponentInputWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit NonselectableComponentInputWidget(QWidget *parent, ComponentDatabase* compDB, QGISVisualizationWidget* visWidget, QString componentType);
    virtual ~NonselectableComponentInputWidget();

    ComponentTableView *getTableWidget() const;

    // Set custom labels in widget
    void setComponentType(const QString &value);
    void setLabel1(const QString &value);
    void setLabel2(const QString &value);
    void setGroupBoxText(const QString &value);

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destName);

    QString getPathToComponentFile(void) const;

    virtual void clear(void);

    QStringList getTableHorizontalHeadings();

    bool isEmpty();

    void setPathToComponentInputFile(const QString &newPathToComponentInputFile);

signals:
    void headingValuesChanged(QStringList);
    void doneLoadingComponents(void);


public slots:
    void handleCellChanged(const int row, const int col);
    virtual bool loadComponentData(void);

protected slots:
    void chooseComponentInfoFileDialog(void);

protected:

    QGISVisualizationWidget* theVisualizationWidget;

    ComponentTableView* componentTableWidget;

    ComponentDatabase*  theComponentDb;

    // Returns a vector of sorted items that are unique
    template <typename T>
    void uniqueVec(std::vector<T>& vec)
    {
        std::sort(vec.begin(), vec.end());

        // Using std::unique to get the unique items in the vector
        auto ip = std::unique(vec.begin(), vec.end());

        // Resizing the vector so as to remove the terms that became undefined after the unique operation
        vec.resize(std::distance(vec.begin(), ip));
    }

    int offset;

    QString pathToComponentInputFile;
    QLineEdit* componentFileLineEdit;

    QGroupBox* componentGroupBox;

    QLabel* label1 = nullptr;
    QLabel* label2 = nullptr;

    QString appType;
    QString componentType;

    QStringList tableHorizontalHeadings;

    void createComponentsBox(void);
};

#endif // NonselectableComponentInputWidget_H
