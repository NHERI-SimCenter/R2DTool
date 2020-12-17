#ifndef ASSETINPUTDELEGATE_H
#define ASSETINPUTDELEGATE_H

#include <QLineEdit>

#include <set>

class AssetInputDelegate : public QLineEdit
{
    Q_OBJECT

public:
    AssetInputDelegate();

    std::set<int> getSelectedComponentIDs() const;

    void insertSelectedCompoonent(const int id);

    void clear();

    int size();

public slots:
    void selectComponents(void);

signals:
    void componentSelectionComplete(void);

private:

    std::set<int> selectedComponentIDs;
};

#endif // ASSETINPUTDELEGATE_H
