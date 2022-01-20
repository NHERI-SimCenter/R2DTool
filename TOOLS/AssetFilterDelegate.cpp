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

#include "AssetFilterDelegate.h"

#include <QRegExpValidator>

#include <qgsvectorlayer.h>

AssetFilterDelegate::AssetFilterDelegate(QgsVectorLayer *layer) : mainLayer(layer)
{
    qb = std::make_unique<QgsQueryBuilder>(layer);
}


int AssetFilterDelegate::openQueryBuilderDialog(QVector<int>& filterIds)
{
    // launch the query builder
    QString subsetBefore = qb->sql();

    if (qb->exec() && (subsetBefore != qb->sql()))
    {
        auto newFilter = qb->sql();

        mainLayer->setSubsetString(QString());

        return setFilterString(newFilter,filterIds);
    }

    return 1;
}


int AssetFilterDelegate::setFilterString(const QString& filter, QVector<int>& filterIds)
{
    // Quick return if the filter string is empty
    if(filter.isEmpty())
        return 1;

    auto fieldIndex = mainLayer->dataProvider()->fieldNameIndex("ID");

    if(fieldIndex == -1)
        return -1;

    QgsFeatureIterator filteredFeats = mainLayer->getFeatures(filter);

    QgsFeature feature;
    while (filteredFeats.nextFeature(feature))
    {
        bool ok = false;
        auto featId = feature.attribute(fieldIndex).toInt(&ok);

        if(ok)
            filterIds.push_back(featId);
        else
            return -1;
    }

    return 0;
}


QString AssetFilterDelegate::getFilterString(void)
{
    return mainLayer->subsetString();
}


void AssetFilterDelegate::clear()
{

}

