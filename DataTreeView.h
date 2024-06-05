#pragma once
#include <QTreeView>
#include <QList>
#include <QUrl>
#include "DataTreeItemDelegate.h"

class QMenu;
class QStandardItemModel;

class DataTreeView : public QTreeView
{
	Q_OBJECT
public:
    explicit DataTreeView(QWidget* parent = 0);
    ~DataTreeView();
private:
    QMenu* contextMenu = nullptr;
    DataTreeItemDelegate* itemDelegate = nullptr;
};

