#include "ExampleDownloader.h"
#include "SimCenterTreeView.h"
#include "NetworkDownloadManager.h"

#include <QDebug>
#include <QUuid>
#include <QPushButton>
#include <QVBoxLayout>

ExampleDownloader::ExampleDownloader(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    exampleTreeView = new SimCenterTreeView(this, "Examples");

    exampleTreeView->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    mainLayout->addWidget(exampleTreeView);

    QPushButton* closeButton = new QPushButton("Close",this);

    QPushButton* downloadButton = new QPushButton("Download",this);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(downloadButton);

    mainLayout->addLayout(buttonLayout);

    connect(closeButton,&QPushButton::pressed,this,&QDialog::close);

    this->setWindowModality(Qt::ApplicationModal);

    downloadManager = std::make_unique<NetworkDownloadManager>();

//    QStringList downloadList = {"https://berkeley.box.com/public/static/p9yjcw6e3zzxpwjxe2c8gqht8ol9t6mb.zip"};
//    QStringList fileNameList = {"Ex1"};

//    downloadManager->executeDownloads(downloadList, fileNameList);
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


//QString path = url.path();
//QString basename = QFileInfo(path).fileName();

//if (basename.isEmpty())
//    basename = "download";

//if (QFile::exists(basename)) {
//    // already exists, don't overwrite
//    int i = 0;
//    basename += '.';
//    while (QFile::exists(basename + QString::number(i)))
//        ++i;

//    basename += QString::number(i);
//}

