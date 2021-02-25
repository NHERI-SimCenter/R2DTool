#ifndef PYTHONPROGRESSDIALOG_H
#define PYTHONPROGRESSDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class QProgressBar;

class PythonProgressDialog : public QDialog
{
    Q_OBJECT

public:
    PythonProgressDialog(QWidget* parent);

    void appendText(const QString text);

    void appendErrorMessage(const QString text);

    void clear(void);

    void showDialog(bool visible);

    void setProgressBarValue(const int val);
    void setProgressBarRange(const int start,const int end);

    void hideAfterElapsedTime(int sec);

public slots:
    void showProgressBar(void);
    void hideProgressBar(void);
    void handleCloseButtonPress();
    void handleClearButtonPress();

signals:
    void processResults(QString);

private:

    QPlainTextEdit* progressTextEdit;

    QString cleanUpText(const QString text);

    QProgressBar* progressBar;
};

#endif // PYTHONPROGRESSDIALOG_H
