#pragma once
#include <QTreeWidgetItem>
#include <QQueue>

QTreeWidgetItem* deepCopyItem(QTreeWidgetItem* item) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem(*item);

    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        newItem->addChild(deepCopyItem(child)); // 递归复制子项
    }
    return newItem;
}

QList<QTreeWidgetItem*> getVisibleItems(QTreeWidget* tree) {
    QList<QTreeWidgetItem*> visibleItems;
    QRect viewportRect = tree->viewport()->rect();  // 获取可视区域

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = tree->topLevelItem(i);
        QRect itemRect = tree->visualItemRect(item);  // 获取项的矩形区域

        if (viewportRect.intersects(itemRect)) {
            visibleItems.append(item);
        }
        for (int j = 0; j < item->childCount(); ++j) {
            QTreeWidgetItem* child = item->child(j);
            QRect childRect = tree->visualItemRect(child);
            if (viewportRect.intersects(childRect)) {
                visibleItems.append(child);
            }
        }
    }
    return visibleItems;
}

void copyQueue(const QQueue<QTreeWidgetItem*>& sourceQueue, QQueue<QTreeWidgetItem*>& destQueue) {
    for (const auto& item : sourceQueue) {
        destQueue.enqueue(static_cast<QTreeWidgetItem*>(item)->clone());
    }
}

void clearQueue(QQueue<QTreeWidgetItem*>& queue) {
    while (!queue.isEmpty()) {
        QTreeWidgetItem* item = queue.dequeue();
        delete item; // 释放内存
    }
}

/*
int main() {
    QQueue<QTreeWidgetItem*> queue2;
    queue2.enqueue(new TraceTreeWidgetItem(QStringList() << "Item 1"));
    queue2.enqueue(new TraceTreeWidgetItem(QStringList() << "Item 2"));
    queue2.enqueue(new TraceTreeWidgetItem(QStringList() << "Item 3"));

    QQueue<QTreeWidgetItem*> queue;
    copyQueue(queue2, queue);

    // 输出 queue 中的元素
    qDebug() << "Elements in queue:";
    for (const auto& item : queue) {
        qDebug() << item->text(0);
    }

    // 清理内存
    clearQueue(queue);
    clearQueue(queue2); // 也清理 queue2 中的元素

    return 0;
}
*/