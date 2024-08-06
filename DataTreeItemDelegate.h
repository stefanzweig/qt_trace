#pragma once
#include <QStyledItemDelegate>

class DataTreeItemDelegate :
    public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DataTreeItemDelegate(QObject* parent = 0);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

};

