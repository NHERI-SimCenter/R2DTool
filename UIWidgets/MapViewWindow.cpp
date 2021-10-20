#include "MapViewWindow.h"
#include "SimCenterMapcanvasWidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QGuiApplication>
#include <QScreen>

MapViewWindow::MapViewWindow(SimCenterMapcanvasWidget* map, QWidget* parent) : QWidget(parent), mapCanvasWidget(map)
{
    this->setWindowFlag(Qt::Window);

    closeButton = new QPushButton("Close");
    connect(closeButton,&QPushButton::pressed,this,&QWidget::close);

    mainLayout = new QVBoxLayout(this);

    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(mapCanvasWidget);
    mainLayout->addWidget(closeButton,Qt::AlignBottom);
}


MapViewWindow::~MapViewWindow()
{
    mainLayout->removeWidget(mapCanvasWidget);
}


void MapViewWindow::showEvent(QShowEvent* e)
{
    if(prevSize.isEmpty())
    {
        QRect rec = QGuiApplication::primaryScreen()->geometry();
        int height = this->height()<int(0.5*rec.height())?int(0.5*rec.height()):this->height();
        int width  = this->width()<int(0.75*rec.width())?int(0.75*rec.width()):this->width();

        this->resize(width, height);
    }
    else
        this->resize(prevSize);

}

void MapViewWindow::closeEvent(QCloseEvent *e)
{
    prevSize = this->size();
}

SimCenterMapcanvasWidget *MapViewWindow::getMapCanvasWidget() const
{
    return mapCanvasWidget;
}

