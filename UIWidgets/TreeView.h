#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

class TreeModel;
class TreeItem;
class VisualizationWidget;

class TreeView : public QTreeView
{
    Q_OBJECT

public:
    TreeView(QWidget *parent, VisualizationWidget* visWidget);

    TreeModel *getLayersModel() const;

    bool removeItemFromTree(const QString& itemName);

    TreeItem* addItemToTree(const QString itemText, const QString layerID, TreeItem* parent = nullptr);

    TreeItem* getTreeItem(const QString& itemName, const QString& parentName) const;

public slots:
    // Shows the "right-click" menu
    void showPopup(const QPoint &position);

private slots:
    // Runs the action that the user selects on the right-click menu
    void runAction();

private:

    TreeModel* layersModel;

    VisualizationWidget* theVisualizationWidget;
};

#endif // TREEVIEW_H
