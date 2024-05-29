#ifndef DEMO_H
#define DEMO_H
#pragma once
#include <QAbstractTableModel>
#include <QTime>
#include <QTimer>

class Demo : public QAbstractTableModel
{
    Q_OBJECT

public:
    Demo()
    {
        int cCount = columnCount(index(0, 0));
        int rCount = rowCount(index(0, 0));

        //  populate model data with *static* values
        QString strTime = QTime::currentTime().toString();

        QStringList temp;
        for (int j = 0; j < cCount; j++)
            temp.append(strTime);
        for (int i = 0; i < rCount; i++)
            demoModelData.append(temp);

        //  nothing new here
        t = new QTimer(this);
        t->setInterval(1000);
        QObject::connect(t, SIGNAL(timeout()), this, SLOT(timerHit()));
        t->start();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {

        //  tells the *view* what to display
        //      if this was dynamic (e.g. like your original strTime implementation)
        //      then the view (QTreeView in main.cpp) will constantly update
        if (role == Qt::DisplayRole)
            return demoModelData.at(index.row()).at(index.column());    //  retrieve data from model

        return QVariant();
    }

    //  reimplemented from QAbstractTableModel
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override
    {
        if (role == Qt::DisplayRole)
        {
            demoModelData[index.row()][index.column()] = value.toString();  //  set the new data

            emit dataChanged(index, index);     //  explicitly emit dataChanged signal, notifies TreeView to update by
                                                //  calling this->data(index, Qt::DisplayRole)
        }

        return true;
    }

    int rowCount(const QModelIndex&) const override { return 10; }
    int columnCount(const QModelIndex&) const override { return 4; }

private slots:
    void timerHit()
    {
        QString strTime = QTime::currentTime().toString();
        setData(index(5, 0), QVariant(strTime), Qt::DisplayRole);   //  only changes index at (row = 5, col = 0)
    }

private:
    QTimer* t;

    QList<QStringList> demoModelData;       //  stores the table model's data

};
#endif // DEMO_H
