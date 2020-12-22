#include "NoneWidget.h"

NoneWidget::NoneWidget(QWidget *parent) : SimCenterAppWidget(parent)
{

}


bool NoneWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);
    return false;
}
