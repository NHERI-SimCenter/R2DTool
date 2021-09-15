#ifndef SimCenterMapcanvasWidget_H
#define SimCenterMapcanvasWidget_H

#include <QWidget>

class QgsMapCanvas;

class SimCenterMapcanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimCenterMapcanvasWidget(const QString &name, QgsMapCanvas *mainMapCanvas);

    QgsMapCanvas *mapCanvas();

    QgsMapCanvas *getMainCanvas() const;

protected:

    void resizeEvent(QResizeEvent *e) override;

    void showEvent(QShowEvent* e) override;

private slots:

    void showLabels(bool show);

    void showAnnotations(bool show);
private:

    QgsMapCanvas *thisMapCanvas = nullptr;

    QgsMapCanvas *mainCanvas = nullptr;

};

#endif // SimCenterMapcanvasWidget_H
