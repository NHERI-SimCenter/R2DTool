#ifndef GMAPPCONFIGWIDGET_H
#define GMAPPCONFIGWIDGET_H

#include <QWidget>

class GmAppConfig;
class QLineEdit;
class QPushButton;
class GMWidget;

class GmAppConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GmAppConfigWidget(GmAppConfig* appConfig, GMWidget *parent = nullptr);

signals:

public slots:

    void saveConfig(void);

private:
    GmAppConfig* appConfig;
    GMWidget* parentWidget;

    QLineEdit* workDirectoryBox;
    QPushButton* workDirectoryButton;
    QLineEdit* inputDirectoryBox;
    QPushButton* inputDirectoryButton;
    QLineEdit* outputDirectoryBox;
    QPushButton* outputDirectoryButton;

    QPushButton* closeButton;

    QLineEdit* usernameBox;
    QLineEdit* passwordBox;

    void setupConnections();
};

#endif // GMAPPCONFIGWIDGET_H
