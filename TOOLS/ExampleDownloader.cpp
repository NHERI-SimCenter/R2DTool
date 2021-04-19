#include "ExampleDownloader.h"
#include "SimCenterTreeView.h"

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

    auto id1 = QUuid::createUuid().toString();
    auto id2 = QUuid::createUuid().toString();

    exampleTreeView->addItemToTree("Ex1..",id1);
    exampleTreeView->addItemToTree("Ex2..",id2);
}


ExampleDownloader::~ExampleDownloader()
{

}


