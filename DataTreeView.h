#pragma once
#include <QTreeView>
#include <QList>
#include <QUrl>

class QMenu;
class QStandardItemModel;

class DataTreeView : public QTreeView
{
	Q_OBJECT
public:
    explicit DataTreeView(QWidget* parent = 0);
    ~DataTreeView();

};

