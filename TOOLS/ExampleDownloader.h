#ifndef ExampleDownloader_H
#define ExampleDownloader_H

#include <QDialog>
#include <unordered_map>

class SimCenterTreeView;
class NetworkDownloadManager;

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
    explicit ExampleDownloader(QWidget *parent = 0);
    ~ExampleDownloader();

    void addExampleToDownload(const QString url, const QString name, const QString description, const QString inputFile);

private slots:


private:

    SimCenterTreeView* exampleTreeView;

    std::unique_ptr<NetworkDownloadManager> downloadManager;

    std::unordered_map<QString, R2DExample> exampleContainer;

};

#endif // ExampleDownloader_H
