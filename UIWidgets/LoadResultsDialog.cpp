#include "LoadResultsDialog.h"
#include "Utils/PythonProgressDialog.h"
#include "WorkflowAppWidget.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QCoreApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGuiApplication>
#include <QScreen>

LoadResultsDialog::LoadResultsDialog(WorkflowAppWidget* parent) : QDialog(parent), workflowWidget(parent)
{

    QGridLayout* mainLayout = new QGridLayout(this);

    auto pathInputFileLabel = new QLabel("Input file: ", this);
    auto pathResultsFolder = new QLabel("Results folder: ", this);

    inputFileLineEdit = new QLineEdit(this);
    resultsFolderLineEdit = new QLineEdit(this);

    inputFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    resultsFolderLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    auto browseInputButton = new QPushButton("Browse",this);
    auto browseResultsFolderButton = new QPushButton("Browse",this);

    connect(browseInputButton,&QPushButton::clicked, this, &LoadResultsDialog::handleGetPathToInputFile);
    connect(browseResultsFolderButton,&QPushButton::clicked, this, &LoadResultsDialog::handleGetPathToResultsFolder);

    mainLayout->addWidget(pathInputFileLabel,0,0);
    mainLayout->addWidget(inputFileLineEdit,0,1);
    mainLayout->addWidget(browseInputButton,0,2);

    mainLayout->addWidget(pathResultsFolder,1,0);
    mainLayout->addWidget(resultsFolderLineEdit,1,1);
    mainLayout->addWidget(browseResultsFolderButton,1,2);

    auto loadResultsButton = new QPushButton("Load Results",this);

    connect(loadResultsButton,&QPushButton::clicked, this, &LoadResultsDialog::handleLoadResults);

    mainLayout->addWidget(loadResultsButton,2,0,1,3);


    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.20*rec.height())?int(0.20*rec.height()):this->height();
    int width  = this->width()<int(0.25*rec.width())?int(0.25*rec.width()):this->width();

    this->resize(width, height);

    // Test to remove
//     inputFileLineEdit->setText("/Users/steve/Documents/R2D/RemoteWorkDir/inputRWHALE.json");
//     resultsFolderLineEdit->setText("/Users/steve/Documents/R2D/RemoteWorkDir");

//     this->handleLoadResults();
}


void LoadResultsDialog::handleGetPathToInputFile(void)
{

    QString existingDir = QCoreApplication::applicationDirPath();

    QString selectedFile = QFileDialog::getOpenFileName(this,
                                                        tr("Select Input File"),
                                                        existingDir,
                                                        "All files (*.*)");
    if(!selectedFile.isEmpty()) {

        QFile inputFile(selectedFile);

        if(inputFile.exists())
            inputFileLineEdit->setText(selectedFile);
    }

}


void LoadResultsDialog::handleGetPathToResultsFolder(void)
{

    QString existingDir = inputFileLineEdit->text();

    if(existingDir.isEmpty())
        existingDir = QCoreApplication::applicationDirPath();

    QString existingResultsDir = QFileDialog::getExistingDirectory(this,
                                                                   tr("Select Results Folder"),
                                                                   existingDir);
    if(!existingResultsDir.isEmpty()) {

        QDir resultsDir(existingResultsDir);

        if(resultsDir.exists())
            resultsFolderLineEdit->setText(existingResultsDir);
    }

}


void LoadResultsDialog::handleLoadResults(void)
{
    this->hide();

    auto statusDialog = PythonProgressDialog::getInstance();

    auto filePath = inputFileLineEdit->text();
    auto resultsPath = resultsFolderLineEdit->text();

    if(filePath.isEmpty())
    {
        statusDialog->appendText("Please select an input file before loading the results");
        return;
    }

    if(resultsPath.isEmpty())
    {
        statusDialog->appendText("Please select the results folder before loading the results");
        return;
    }

    // First clear the workflow widget in case old results are there
    workflowWidget->clear();

    // First load the input file
    auto res = workflowWidget->loadFile(filePath);
    if(res != 0)
        return;

#ifdef OpenSRA
    workflowWidget->postprocessResults(resultsPath,QString(),QString());
#else
    workflowWidget->processResults(resultsPath);
#endif

    statusDialog->appendText("Done loading the results");
}

