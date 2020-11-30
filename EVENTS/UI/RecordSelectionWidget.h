#ifndef RECORDSELECTIONWIDGET_H
#define RECORDSELECTIONWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include "RecordSelectionConfig.h"

class RecordSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecordSelectionWidget(RecordSelectionConfig& selectionConfig, QWidget *parent = nullptr);

signals:

public slots:
private:
    RecordSelectionConfig& m_selectionConfig;
    QComboBox* m_dbBox;
//    QComboBox* m_errorBox;
};

#endif // RECORDSELECTIONWIDGET_H
