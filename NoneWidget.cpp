#include "NoneWidget.h"

NoneWidget::NoneWidget() : SimCenterAppWidget()
{

}


bool NoneWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);
    return false;
}
