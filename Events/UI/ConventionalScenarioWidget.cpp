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

#include "ConventionalScenarioWidget.h"
#include "AssetInputDelegate.h"

#include <QJsonObject>
#include <QGridLayout>
#include <QLabel>

ConventionalScenarioWidget::ConventionalScenarioWidget(GMERFWidget* gmerf, QWidget *parent) : SimCenterAppWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);

    auto ruptureLabel = new QLabel("Enter the indexes of one or more rupture scenarios to analyze."
                                   "\nDefine a range of rupture scenarios with a dash and separate multiple ruputure scenarios with a comma.");
    ruptureLineEdit = new AssetInputDelegate();
    connect(ruptureLineEdit,&QLineEdit::editingFinished,this,&ConventionalScenarioWidget::selectComponents);
    ruptureLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    componentTableWidget = new ComponentTableView();
    componentTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    componentTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectionModel = componentTableWidget->selectionModel();

    mainLayout->addWidget(ruptureLabel,0,0);
    mainLayout->addWidget(ruptureLineEdit,0,1);
    mainLayout->addWidget(componentTableWidget, 1, 0, 1, 3);

    mainLayout->setRowStretch(2, 1);

    connect(gmerf, SIGNAL(ruptureFileReady(QString)), this, SLOT(LoadRupturesTable(QString)));

    // Update ruptureLineEdit based on selection on componentTableWidget
    QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged, [&]() {
        int indexColID = 0;
        if (!selectionModel->hasSelection()){
            return;
        }
        QModelIndexList indices = selectionModel->selectedRows();
        if (indices.size()<1){
            return;
        }
        QVector<int> selectedIndices;
        for(QModelIndex& id:indices){
            int row = id.row();
            selectedIndices.append((componentTableWidget->item(row, indexColID)).toInt());
        }
        std::sort(selectedIndices.begin(), selectedIndices.end());
        QString filter = convertToFirstLast(selectedIndices);
        ruptureLineEdit->setText(filter);
    });
}


bool ConventionalScenarioWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool ConventionalScenarioWidget::outputToJSON(QJsonObject& obj)
{

//    obj["RuptureFilter"] = ruptureLineEdit->getComponentAnalysisList();
    obj["RuptureFilter"] = ruptureLineEdit->text();
    obj.insert("method","MonteCarlo");

    return true;
}

void ConventionalScenarioWidget::selectComponents(void)
{
    try
    {
        ruptureLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}

bool ConventionalScenarioWidget::LoadRupturesTable(QString pathToRuptureFile){
    QFile jsonFile(pathToRuptureFile);
    if (jsonFile.exists() && jsonFile.open(QFile::ReadOnly)){
        QString text = jsonFile.readAll();
        QJsonDocument exDoc = QJsonDocument::fromJson(text.toUtf8());
        QJsonObject jsonObject = exDoc.object();
        QJsonArray features = jsonObject["features"].toArray();
        QVector<QStringList> data;
        if (features.size()<1){
            return false;
        }
        QJsonObject firstFeat = features[0].toObject();
        QJsonObject firstProp = firstFeat["properties"].toObject();
        QStringList keys = firstProp.keys();
        // Reorder the keys:
        int removed;
        removed = keys.removeAll("MeanAnnualRate");
        if (removed==1){
            keys.prepend("MeanAnnualRate");
        }
        removed = keys.removeAll("Magnitude");
        if (removed==1){
            keys.prepend("Magnitude");
        }
        removed = keys.removeAll("Name");
        if (removed==1){
            keys.prepend("Name");
        }
        removed = keys.removeAll("Rupture");
        if (removed==1){
            keys.prepend("Rupture");
        }
        removed = keys.removeAll("Source");
        if (removed==1){
            keys.prepend("Source");
        }
        int index = 1; // rupture index starts from 1 to be consistent with QGIS attribute table
        for (const QJsonValue& valueIt : features) {
            QJsonObject feat = valueIt.toObject();
            QJsonObject prop = feat["properties"].toObject();
            QStringList row;
            row.append(QString::number(index));
            index++;
            for (const QString& key:keys){
                QString value = prop[key].toVariant().toString();
                if (value == ""){
                    QJsonDocument jsonDoc(prop[key].toObject());
                    if (jsonDoc.toJson()!="{\n}\n"){
                        value = jsonDoc.toJson(QJsonDocument::Compact);
                    }
                }
                row.append(value);
            }
            data.append(row);
        }
        keys.prepend("index");
        componentTableWidget->getTableModel()->populateData(data, keys);
        componentTableWidget->show();
        componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    }
    return true;
}

