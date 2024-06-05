#include "DataTreeView.h"
#include <QMenu>

DataTreeView::DataTreeView(QWidget* parent) :
    QTreeView(parent)
{
    contextMenu = new QMenu(this);

    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    itemDelegate = new DataTreeItemDelegate();
    setItemDelegate(itemDelegate);

}

DataTreeView::~DataTreeView()
{
    delete contextMenu;
    delete itemDelegate;
}