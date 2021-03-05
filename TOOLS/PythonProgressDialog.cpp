#include "PythonProgressDialog.h"

#include <QDebug>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>

PythonProgressDialog::PythonProgressDialog(QWidget* parent) : QDialog(parent)
{

    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowTitle("Program Output");
    this->setAutoFillBackground(true);

    auto progressLayout = new QVBoxLayout(this);

    progressTextEdit = new QPlainTextEdit(this);
    progressTextEdit->setWordWrapMode(QTextOption::WrapMode::WordWrap);
    progressTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    progressTextEdit->setReadOnly(true);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0,0);
    progressBar->hide();

    // give it some dimension
    int nWidth = 800;
    int nHeight = 500;
    if (parent != NULL)
        setGeometry(parent->x() + parent->width()/2,
                    parent->y() + parent->height()/2 - nHeight/2,
                    nWidth, nHeight);
    else
        resize(nWidth, nHeight);


    progressLayout->addWidget(progressTextEdit);
    progressLayout->addWidget(progressBar);

    auto buttonsLayout = new QHBoxLayout();

    QPushButton* closeButton = new QPushButton("Close",this);
    buttonsLayout->addWidget(closeButton);

    QPushButton* clearButton = new QPushButton("Clear",this);
    buttonsLayout->addWidget(clearButton);

    progressLayout->addLayout(buttonsLayout);

    connect(closeButton,&QPushButton::pressed, this, &PythonProgressDialog::handleCloseButtonPress);
    connect(clearButton,&QPushButton::pressed, this, &PythonProgressDialog::handleClearButtonPress);
}


void PythonProgressDialog::clear(void)
{
    progressTextEdit->clear();
    progressBar->setRange(0,0);
    this->hideProgressBar();
}


void PythonProgressDialog::showDialog(bool visible)
{
    if(visible)
    {
        this->show();
        this->raise();
        this->activateWindow();
    }
    else
    {
        this->hide();
    }
}


void PythonProgressDialog::appendText(const QString text)
{
    if(!this->isVisible())
        this->showDialog(true);

    auto cleanText = cleanUpText(text);

    progressTextEdit->appendPlainText(cleanText+ "\n");

    qDebug()<<cleanText;
}


void PythonProgressDialog::appendErrorMessage(const QString text)
{
    if(!this->isVisible())
        this->showDialog(true);

    auto msgStr = QString("<font color=%1>").arg("red") + text + QString("</font>") + QString("<font color=%1>").arg("black") + QString("&nbsp;") + QString("</font>");

    // Output to console and to text edit
    progressTextEdit->appendHtml(msgStr);

    progressTextEdit->appendPlainText("\n");

    qDebug()<<text;
}


void PythonProgressDialog::appendInfoMessage(const QString text)
{
    if(!this->isVisible())
        this->showDialog(true);

    auto msgStr = QString("<font color=%1>").arg("blue") + text + QString("</font>") + QString("<font color=%1>").arg("black") + QString("&nbsp;") + QString("</font>");

    // Output to console and to text edit
    progressTextEdit->appendHtml(msgStr);

    progressTextEdit->appendPlainText("\n");

    qDebug()<<text;
}



void PythonProgressDialog::handleCloseButtonPress()
{
    this->showDialog(false);
}


void PythonProgressDialog::handleClearButtonPress()
{
    this->clear();
}


QString PythonProgressDialog::cleanUpText(const QString text)
{
    // Split the text up if there are any newline
    auto cleanText = text.trimmed();

    cleanText.replace("\\n", " \n ");

    return cleanText;
}


void PythonProgressDialog::showProgressBar(void)
{
    progressBar->show();
}


void PythonProgressDialog::hideProgressBar(void)
{
    progressBar->hide();
}

void PythonProgressDialog::setProgressBarValue(const int val)
{
    progressBar->setValue(val);
}


void PythonProgressDialog::setProgressBarRange(const int start,const int end)
{
    progressBar->setRange(start,end);
}


void PythonProgressDialog::hideAfterElapsedTime(int sec)
{
    progressTextEdit->appendPlainText("This window will automatically close in "+QString::number(sec) + " seconds \n");

    QTimer::singleShot(sec*1000, [=]() {

        if(this->isVisible())
            this->showDialog(false);

        progressTextEdit->undo();
    });
}
