#ifndef TREEITEM_H
#define TREEITEM_H
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

#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QVector>

class QDialog;

class TreeItem : public QObject
{
    Q_OBJECT

public:
    explicit TreeItem(const QVector<QVariant> &data, const QString& ID = QString(), TreeItem *parentItem = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);
    void removeChild(TreeItem *child);
    void removeChild(int row);

    TreeItem* child(int row);

    QStringList getActionList();

    int childCount() const;
    int columnCount() const;

    QVariant data(int column) const;
    int row() const;

    TreeItem* getParentItem();

    // 0 = unchecked
    // 1 = partially checked
    // 2 = checked
    int getState() const;

    void setState(int state);

    QVector<TreeItem*> getChildItems() const;

    void moveChild(int sourceRow, int destinationRow);

    TreeItem *findChild(QString name);

    QString getName() const;

    QString getItemID() const;

public slots:

    void changeOpacity();

    void handleChangeOpacity(int value);

signals:

void opacityChanged(const QString& layerID, const double opacity);

private:
    QVector<TreeItem*> vecChildItems;
    QVector<QVariant> itemData;
    TreeItem* parentItem;
    int currentState;

    QString itemName;
    QString itemID;

    QDialog* opacityDialog;
};


#endif // TREEITEM_H
