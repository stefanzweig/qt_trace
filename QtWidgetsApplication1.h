#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <QTimer>
#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/ZoneMasterCanParserSubscriber.h"
#include "multiThread.h"
//#include "treemodel.h"
//#include "demo.h"
#include "zm_struct.h"
#include "columnfilter.h"

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

protected:
    void createActions();

private:
    void init();
    void setupTreeTrace();
    void setupdatamodel();
    void setupdatamodel_canparser();
    void initialHeaders();
    void _updateCurrentState();
    Ui::QtWidgetsApplication1Class ui;
    QTimer* timer = nullptr;
    ZoneMasterCanMessageDataSubscriber* mysub_can_frames = nullptr;
    ZoneMasterCanParserSubscriber* mysub_can_parser = nullptr;
    multiThread* calc_thread = nullptr;
    //TreeModel* model = nullptr;
    //Demo* demo_model = nullptr;
    QVector<can_frame> full_canframes;
    QVector<canframe> full_canparserdata;
    uint64_t last_timestamp = 0;
    uint64_t last_timestamp_canparser = 0;
    uint64_t full_count_canframes = 0;
    uint64_t full_count_canparser = 0;
    uint64_t count_per_page = 3000;
    uint64_t current_page = 0;
    bool isHex = true; // repr in hex
    QStringList initialHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "Data", "EventType", "DataLength", "BusType" };
    QList<QPushButton*> headerButtonList;
    columnFilterDialog* filter = nullptr;
    QHash<QString, QString> filterConfig;

private slots:
    void updateState();
    void onActionTriggered();
    void formatRow(int x);
    void formatRow_str(QString s);
    void formatRow_canframe(can_frame cf);
    void formatRow_canparser(unsigned long long i);
    void internal_canparser(canframe frame);
    void applyFilter(QList<QList<QString>> items, int count);

    void startTrace();
    void stopTrace();
    void pauseTrace();
    void resetLayout();

    void headerButtonClicked();
    void prepareMenu(const QPoint& pos);
    void newDev();
};
