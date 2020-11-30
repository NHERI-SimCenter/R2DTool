#ifndef GMPEWIDGET_H
#define GMPEWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include "GMPE.h"

class GMPEWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GMPEWidget(GMPE& gmpe, QWidget *parent = nullptr);

signals:

public slots:

private:
    GMPE& m_gmpe;
    QComboBox* m_typeBox;

    void setupConnections();
};

#endif // GMPEWIDGET_H
