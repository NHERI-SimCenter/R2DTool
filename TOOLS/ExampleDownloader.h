#ifndef ExampleDownloader_H
#define ExampleDownloader_H

#include <QDialog>

class SimCenterTreeView;

class ExampleDownloader : public QDialog
{
    Q_OBJECT

public:
    explicit ExampleDownloader(QWidget *parent = 0);
    ~ExampleDownloader();

private slots:


private:

    SimCenterTreeView* exampleTreeView;

};

#endif // ExampleDownloader_H
