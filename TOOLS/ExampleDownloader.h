#ifndef ExampleDownloader_H
#define ExampleDownloader_H
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

#include <QDialog>
#include <unordered_map>

class TreeItem;
class SimCenterTreeView;
class NetworkDownloadManager;
class PythonProgressDialog;
class MainWindowWorkflowApp;

struct R2DExample{

public:
    QString url;
    QString name;
    QString description;
    QString inputFile;
    QString uid;
};

class ExampleDownloader : public QDialog
{
    Q_OBJECT

public:
    explicit ExampleDownloader(MainWindowWorkflowApp *parent);
    ~ExampleDownloader();

    void addExampleToDownload(const QString url, const QString name, const QString description, const QString inputFile);

    void updateTree(void);

private slots:

    void updateExamples(void);
    void removeExample(const QString& name);

    void handleDownloadFinished(bool val);

private:

    SimCenterTreeView* exampleTreeView;

    std::unique_ptr<NetworkDownloadManager> downloadManager;

    std::unordered_map<QString, R2DExample> exampleContainer;

    bool checkIfExampleExists(const QString& name);

    bool deleteExampleFolder(const QString& name);

    PythonProgressDialog* statusDialog;
    MainWindowWorkflowApp* workflowApp;
};

#endif // ExampleDownloader_H
