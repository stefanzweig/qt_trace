#include "DataTreeItemDelegate.h"
#include <QPainter>
#include <QDebug>

DataTreeItemDelegate::DataTreeItemDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

QSize DataTreeItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    return size;
}

void DataTreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
     QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyledItemDelegate::paint(painter, opt, index);
    return;
}
