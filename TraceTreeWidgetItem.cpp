#include "TraceTreeWidgetItem.h"

QVariant TraceTreeWidgetItem::data(int column, int role) const {
    return QTreeWidgetItem::data(column, role);
}

bool TraceTreeWidgetItem::operator<(const QTreeWidgetItem& other) const {
    /* treat dot as a separator for a more natural sorting */
    int column = treeWidget() ? treeWidget()->sortColumn() : 0;
    const QString txt1 = text(column);
    const QString txt2 = other.text(column);
    int start1 = 0, start2 = 0;
    return true;

    //for (;;) {
    //    int end1 = txt1.indexOf(QLatin1Char('.'), start1);
    //    int end2 = txt2.indexOf(QLatin1Char('.'), start2);
    //    QString part1 = end1 == -1 ? txt1.sliced(start1, txt1.size() - start1)
    //        : txt1.sliced(start1, end1 - start1);
    //    QString part2 = end2 == -1 ? txt2.sliced(start2, txt2.size() - start2)
    //        : txt2.sliced(start2, end2 - start2);
    //    int comp = collator_.compare(part1, part2);
    //    if (comp == 0)
    //        comp = part1.size() - part2.size(); // a workaround for QCollator's bug
    //    if (comp != 0)
    //        return comp < 0;
    //    if (end1 == -1 || end2 == -1)
    //        return end1 < end2;
    //    start1 = end1 + 1;
    //    start2 = end2 + 1;
    //}
}

