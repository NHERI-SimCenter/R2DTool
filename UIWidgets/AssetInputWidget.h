#ifndef AssetInputWidget_H
#define AssetInputWidget_H
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

// Written by: Dr. Stevan Gavrilovic

// Use this class to import assets in a csv format; functionality implemented in the 'loadAssetData' function

#include "SimCenterAppWidget.h"
#include "GISSelectable.h"
#include "ComponentDatabase.h"

#include <set>

#include <QString>
#include <QObject>

class AssetInputDelegate;
class AssetFilterDelegate;
class ComponentTableView;
class VisualizationWidget;

class QgsFeature;
class QGISVisualizationWidget;
class QgsVectorLayer;
class QgsGeometry;

class QGroupBox;
class QLineEdit;
class QTableWidget;
class QLabel;
class QVBoxLayout;
class QPushButton;
class QHBoxLayout;
class QGridLayout;

class AssetInputWidget : public  SimCenterAppWidget, public GISSelectable
{
    Q_OBJECT

public:
    explicit AssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType = QString());
    virtual ~AssetInputWidget();

    virtual int loadAssetVisualization() = 0;

    void insertSelectedAssets(QgsFeatureIds& featureIds);
    void clearSelectedAssets(void);

    ComponentTableView *getTableWidget() const;

    int getNumberOfAseets(void);

    // Set the filter string and select the components
    void setFilterString(const QString& filter);
    QString getFilterString(void);

    int numberComponentsSelected(void);

    // Set custom labels in widget
    void setComponentType(const QString &value);
    void setLabel1(const QString &value);
    void setLabel2(const QString &value);
    void setLabel3(const QString &value);
    void setGroupBoxText(const QString &value);

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    bool copyFiles(QString &destName);

    QString getPathToComponentFile(void) const;
    void setPathToComponentFile(const QString& path);

    virtual void clear(void);

    QStringList getTableHorizontalHeadings();

    // Selects all of the components for analysis
    void selectAllComponents(void);

    int applyFilterString(const QString& filter);

    bool isEmpty();

    QgsVectorLayer *getMainLayer() const;
    QgsVectorLayer *getSelectedLayer() const;

    void setFilterVisibility(const bool value);

    QString getAssetType() const;

    QHBoxLayout *getAssetFilePathLayout() const;
    QLabel *getLabel1() const;

    void hideAssetFilePath() const;
    void hideCRS_Selection() const;

    void setPathToComponentInputFile(const QString &newPathToComponentInputFile);

    void insertWidgetIntoLayout(QWidget* widget, int numRowsFromBottom, int numColumnsSpan = 0);

signals:
    void headingValuesChanged(QStringList);
    void doneLoadingComponents(void);

public slots:
    void handleComponentSelection(void);
    void handleCellChanged(const int row, const int col);

protected slots:
    void selectComponents(void);
    virtual bool loadAssetData(bool message = true);
    void chooseComponentInfoFileDialog(void);
    void clearComponentSelection(void);
    void handleComponentFilter(void);

protected:

    QGISVisualizationWidget* theVisualizationWidget = nullptr;

    ComponentTableView* componentTableWidget = nullptr;
    ComponentDatabase*  theComponentDb = nullptr;

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

    AssetInputDelegate* selectComponentsLineEdit = nullptr;
    AssetFilterDelegate* filterDelegateWidget = nullptr;
    QPushButton* browseFileButton = nullptr;

    QWidget* filterWidget = nullptr;

    int offset;

    QString pathToComponentInputFile;
    QLineEdit* componentFileLineEdit = nullptr;
    QHBoxLayout* pathLayout = nullptr;
    QGroupBox* componentGroupBox = nullptr;

    QGridLayout* mainWidgetLayout = nullptr;

    QLabel* label1 = nullptr;
    QLabel* label2 = nullptr;
    QLabel* label3 = nullptr;

    QString appType;
    QString assetType;

    QStringList tableHorizontalHeadings;

    virtual void createComponentsBox(void);

    QgsVectorLayer* mainLayer = nullptr;
    QgsVectorLayer* selectedFeaturesLayer = nullptr;

    void clearTableData(void);


};

#endif // AssetInputWidget_H
