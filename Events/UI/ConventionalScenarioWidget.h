#ifndef ConventionalScenarioWidget_H
#define ConventionalScenarioWidget_H
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
#include "qjsonobject.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "GMERFWidget.h"
#include <QFile>
#include <QList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHeaderView>

class AssetInputDelegate;

class ConventionalScenarioWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit ConventionalScenarioWidget(GMERFWidget* gmerf, QWidget *parent = nullptr);

    bool outputToJSON(QJsonObject& obj);
    bool inputFromJSON(QJsonObject& obj);


public slots:
    bool LoadRupturesTable(QString pathToRuptureFile);

protected slots:
    void selectComponents(void);
private:

    AssetInputDelegate* ruptureLineEdit = nullptr;
    ComponentTableView* componentTableWidget = nullptr;
    QItemSelectionModel *selectionModel;
    // A function to convert the selected indices to filter string
    QString convertToFirstLast(const QVector<int>& sequence) {
        if (sequence.isEmpty()) {
            return "Sequence is empty.";
        }
        QString result;
        int start = sequence.first();
        int end = sequence.first();
        for (int i = 1; i < sequence.size(); ++i) {
            if (sequence.at(i) == end + 1) {
                // Current number is consecutive to the previous one
                end = sequence.at(i);
            } else {
                // Current number is not consecutive
                if (!result.isEmpty()) {
                    result += ", ";
                }
                if (start == end) {
                    // Only one number in the range
                    result += QString::number(start);
                } else {
                    // Range of consecutive numbers
                    result += QString::number(start) + "-" + QString::number(end);
                }
                // Start a new range
                start = end = sequence.at(i);
            }
        }
        // Append the last range
        if (!result.isEmpty()) {
            result += ", ";
        }
        if (start == end) {
            result += QString::number(start);
        } else {
            result += QString::number(start) + "-" + QString::number(end);
        }
        return result;
    }

};

#endif // ConventionalScenarioWidget_H
