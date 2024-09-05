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
#include <QQueue>

#define TIMER_HEARTBEAT 50
#define MAX_ITEM_COUNT 5000

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

protected:
    void createActions();

private:
    Ui::QtWidgetsApplication1Class ui;
    void init();
    void setupTreeTrace();
    void initialHeaders();
    void _updateCurrentState();
    void hide_filtered_items(int idx, QList<QList<QString>> items);
    void get_default_configurations();

    QTimer* timer = nullptr;
    QTimer* timer_dustbin = nullptr;
    ZoneMasterCanMessageDataSubscriber* mysub_can_frames = nullptr;
    ZoneMasterCanParserSubscriber* mysub_can_parser = nullptr;
    multiThread* calc_thread = nullptr;
    QQueue<QTreeWidgetItem*> full_queue;
    QQueue<QTreeWidgetItem*> trace_items;

    uint64_t last_timestamp = 0;
    uint64_t last_timestamp_canparser = 0;
    uint64_t full_count_canframes = 0;
    uint64_t full_count_canparser = 0;
    uint64_t count_per_page = 3000;
    uint64_t current_page = 0;
    bool isHex = true; // repr in hex

    QStringList initialHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "Data", "EventType", "DataLength", "BusType" };
    QStringList CurrentHeader = initialHeader;
    QStringList headers = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "Data", "EventType", "DataLength", "BusType" };
    QStringList linHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DataLength", "Data", "EventType", "BusType" };
    QStringList ethHeader = { "Time[ms]","Chn","Dir","Source MAC","Destination MAC","Source IP","Destination IP","Protocol","Payload Data","VLAN ID","Transport Layer","Service","Service Instance","Method","Message Type","Port(s)","VLAN Priority" };
    QComboBox* datachoice = nullptr;

    QList<QPushButton*> headerButtonList;
    columnFilterDialog* filter = nullptr;  // filter dialog
    QHash<QString, QString> filterConfig;
    QVector<QVector<bool>> selectedStates;

    // further configurations
    int display_mode = 0; // 0 - append, 1 - update
    int visible_height = 700;
    int item_height = 20;  /* default item height in the trace widget. */
    int item_width = 20;
    int page_capacity = 300;
    int dds_domainid = 90;

private slots:
    void startTrace();
    void stopTrace();
    void pauseTrace();
    void resetLayout();
    void display_mode_switch();

    void updateState();
    void onActionTriggered();
    void formatRow(int x);
    void formatRow_str(QString s);
    void formatRow_canframe(can_frame cf);
    void formatRow_canparser(unsigned long long i);
    void internal_canparser(canframe frame);
    void applyFilter(QList<QList<QString>> items, int count);
    void on_pop_to_root(QTreeWidgetItem* item);

    void headerButtonClicked();
    void prepareMenu(const QPoint& pos);
    void newDev();
    void ChangeHeader(const QString& text);
    void ButtonSearchClicked();
    void dustbin();
    void updateToolbar();
    void update_tracewidget();
    void update_tracewidget_refresh();
    void trace_scroll_changed(int value);
};
