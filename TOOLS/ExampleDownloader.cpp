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

#include "ExampleDownloader.h"
#include "SimCenterTreeView.h"
#include "NetworkDownloadManager.h"
#include "CheckableTreeModel.h"
#include "LayerTreeItem.h"
#include "Utils/PythonProgressDialog.h"
#include "MainWindowWorkflowApp.h"

#include <QDebug>
#include <QUuid>
#include <QPushButton>
#include <QApplication>
#include <QVBoxLayout>

ExampleDownloader::ExampleDownloader(MainWindowWorkflowApp *parent) : QDialog(nullptr)
{
    statusDialog = PythonProgressDialog::getInstance();
    workflowApp = parent;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    exampleTreeView = new SimCenterTreeView(this, "Examples");

    exampleTreeView->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    exampleTreeView->setContextMenuPolicy(Qt::NoContextMenu);

    mainLayout->addWidget(exampleTreeView);

    QPushButton* closeButton = new QPushButton("Close",this);

    QPushButton* updateButton = new QPushButton("Update",this);
    updateButton->setToolTip("Download or remove checked items");

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(updateButton);

    mainLayout->addLayout(buttonLayout);

    // Make the connections
    connect(closeButton,&QPushButton::pressed,this,&QDialog::close);
    connect(updateButton,&QPushButton::pressed,this,&ExampleDownloader::updateExamples);

    this->setWindowModality(Qt::ApplicationModal);

    downloadManager = std::make_unique<NetworkDownloadManager>(parent);

    connect(downloadManager.get(),&NetworkDownloadManager::downloadSuccess, this, &ExampleDownloader::handleDownloadFinished);
}


ExampleDownloader::~ExampleDownloader()
{

}


void ExampleDownloader::addExampleToDownload(const QString url, const QString name, const QString description, const QString inputFile)
{
    R2DExample exmpl;

    exmpl.name = name;
    exmpl.url = url;
    exmpl.description = description;
    exmpl.inputFile = inputFile;

    auto uid = QUuid::createUuid().toString();

    exmpl.uid = uid;

    exampleTreeView->addItemToTree(name,uid);

    exampleContainer.emplace(std::make_pair(name,exmpl));

}


void ExampleDownloader::updateExamples(void)
{

    QString msg = "Updating examples...  please wait.";
    statusDialog->appendText(msg);
    QApplication::processEvents();

    QStringList downloadUrlList;
    QStringList fileNameList;

    auto allTreeItems = exampleTreeView->getAllTreeItems();

    for(auto&& item : allTreeItems)
    {
        auto name = item->getName();

        if(name.compare("Examples") == 0)
            continue;

        auto state = item->getState();

        // State of 0 is unchecked
        if(state == 0)
        {
            this->removeExample(name);
        }
        else // Otherwise checked
        {
            auto found = exampleContainer.find (name);

            if (found != exampleContainer.end() )
            {

                if(checkIfExampleExists(name) == true)
                    continue;

                auto example = found->second;

                auto url = example.url;

                downloadUrlList.append(url);
                fileNameList.append(name);
            }
            else
            {
                QString err = "Could not find example "+name;
                statusDialog->appendErrorMessage(err);
            }
        }
    }

    if(!downloadUrlList.isEmpty())
        downloadManager->downloadExamples(downloadUrlList, fileNameList);
    else
        this->handleDownloadFinished(true);

    this->hide();
}



void ExampleDownloader::removeExample(const QString& name)
{
    if(checkIfExampleExists(name) == true)
    {
        auto res = this->deleteExampleFolder(name);
        if(res == true)
            statusDialog->appendText("Successfully removed example " + name);
        else
            statusDialog->appendText("Failed to remove example " + name);

    }

}


bool ExampleDownloader::checkIfExampleExists(const QString& name)
{
    auto pathAllExamplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples";

    auto pathToExampleInputFile = pathAllExamplesFolder;

    auto found = exampleContainer.find (name);
    if (found != exampleContainer.end() )
    {
        auto example = found->second;

        auto inputFile = example.inputFile;

        pathToExampleInputFile += QDir::separator() + inputFile;

    }
    else
        return false;

    QFile inputFile(pathToExampleInputFile);

    if(!inputFile.exists())
        return false;

    return true;
}


bool ExampleDownloader::deleteExampleFolder(const QString& name)
{
    auto pathAllExamplesFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "Examples";

    auto pathToExampleFile = pathAllExamplesFolder;

    auto found = exampleContainer.find (name);
    if (found != exampleContainer.end() )
    {
        auto example = found->second;

        auto inputFile = example.inputFile;

        pathToExampleFile += QDir::separator() + inputFile;

    }
    else
        return false;


    QFileInfo fileInfo(pathToExampleFile);

    if(!fileInfo.exists())
        return false;

    QDir exampleFolder(fileInfo.absolutePath());

    statusDialog->appendText("Removing Example "+name);

    return exampleFolder.removeRecursively();
}


void ExampleDownloader::updateTree(void)
{
    auto allTreeItems = exampleTreeView->getAllTreeItems();

    for(auto&& item : allTreeItems)
    {
        auto name = item->getName();

        if(this->checkIfExampleExists(name) == true)
            item->setState(2);
    }
}


void ExampleDownloader::handleDownloadFinished(bool val)
{
    if(val)
        workflowApp->updateExamplesMenu();
}

