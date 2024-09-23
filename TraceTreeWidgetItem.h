#pragma once
#include <qtreewidget.h>

class TraceTreeWidgetItem :
    public QTreeWidgetItem
{
public:
    TraceTreeWidgetItem(QTreeWidgetItem* parent = nullptr, int type = QTreeWidgetItem::Type) :
        QTreeWidgetItem(parent, type) {
        source_ = "canframe";
    }
    TraceTreeWidgetItem(const QStringList& strings, int type = Type) : QTreeWidgetItem(strings, type) {
        source_ = "canframe";
    }

    QVariant data(int column, int role) const override;
    bool operator<(const QTreeWidgetItem& other) const override;
    void setSource(QString source) {
        source_ = source;
    }
    QString getSource() {
        return source_;
    }

private:
    QString source_;
};
