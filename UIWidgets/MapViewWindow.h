#ifndef MAPVIEWWINDOW_H
#define MAPVIEWWINDOW_H

#include <QWidget>

class SimCenterMapcanvasWidget;

class QPushButton;
class QVBoxLayout;

class MapViewWindow : public QWidget
{
public:
    MapViewWindow(SimCenterMapcanvasWidget* map, QWidget* parent = nullptr);
    ~MapViewWindow();

    SimCenterMapcanvasWidget *getMapCanvasWidget() const;

protected:

    void showEvent(QShowEvent* e) override;

    void closeEvent(QCloseEvent *e) override;

private:

    SimCenterMapcanvasWidget* mapCanvasWidget;
    QVBoxLayout* mainLayout;
    QPushButton* closeButton;
    QSize prevSize;
};

#endif // MAPVIEWWINDOW_H
