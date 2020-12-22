#ifndef NONEWIDGET_H
#define NONEWIDGET_H

#include "SimCenterAppWidget.h"

#include <QJsonObject>

class NoneWidget : public  SimCenterAppWidget
{
public:
    NoneWidget();

    bool outputAppDataToJSON(QJsonObject &jsonObject);
};

#endif // NONEWIDGET_H
