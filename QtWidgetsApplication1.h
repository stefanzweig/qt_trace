#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <QTimer>
#include <QQueue>
#include <QLabel>
#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/ZoneMasterCanParserSubscriber.h"
#include "multiThread.h"
#include "zm_struct.h"
#include "columnfilter.h"

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
    void compare_item();

    QTimer* timer = nullptr;
    QTimer* timer_dustbin = nullptr;
    ZoneMasterCanMessageDataSubscriber* mysub_can_frames = nullptr;
    ZoneMasterCanParserSubscriber* mysub_can_parser = nullptr;
    multiThread* calc_thread = nullptr;
    QQueue<QTreeWidgetItem*> full_queue;

    uint64_t last_timestamp = 0;
    uint64_t last_timestamp_canparser = 0;
    uint64_t full_count_canframes = 0;
    uint64_t full_count_canparser = 0;
    uint64_t count_per_page = 500;
    uint64_t current_page = 0;
    bool isHex = true; // repr in hex

    QStringList initialHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "EventType", "DataLength", "BusType", "Data"};
    QStringList CurrentHeader = initialHeader;
    QStringList headers = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "EventType", "DataLength", "BusType", "Data" };
    QStringList linHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DataLength", "EventType", "BusType", "Data" };
    QStringList ethHeader = { "Time[ms]","Chn","Dir","Source MAC","Destination MAC","Source IP","Destination IP","Protocol","Payload Data","VLAN ID","Transport Layer","Service","Service Instance","Method","Message Type","Port(s)","VLAN Priority" };
    QComboBox* datachoice = nullptr;

    QList<QPushButton*> headerButtonList;
    columnFilterDialog* filter = nullptr;  // filter dialog
    QHash<QString, QString> filterConfig;
    QVector<QVector<bool>> selectedStates;

    QLabel* leftLabel = nullptr;
    QWidget* leftWidget = nullptr;
    QHBoxLayout* leftLayout = nullptr;
    QLabel* rightLabel = nullptr;
    QWidget* rightWidget = nullptr;
    QHBoxLayout* rightLayout = nullptr;

    // further configurations
    int display_mode = 0; // 0 - append, 1 - update
    int visible_height = 700;
    int item_height = 20;  /* default item height in the trace widget. */
    int item_width = 20;
    int page_capacity = 4000;
    int dds_domainid = 90;
    int last_data_index = -1;
    int padding = 1;
    bool frozen = false;
    bool initial_trace = true;
    QList<QString> monitor_modules;
    QHash<QString, QList<QVariant>> new_filters;
    bool filter_pop = false;

private slots:
    void startTrace();
    void stopTrace();
    void pauseTrace();
    void resetLayout();
    void display_mode_switch();
    void resetStatusBar();
    void resumeTrace();

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
    void update_tracewindow();
    void get_refreshed_items();
    void trace_scroll_changed(int value);
    bool new_session();
    void freeze_treetrace_items(int ncount);
    void adjust_filter_buttons();
    void on_header_section_resized();
    void on_horizontal_scroll();
    void about();
    void reset_all_filters();
    bool filter_pass_item(QTreeWidgetItem* it);
    void update_tracewidget_outdate();
    void refresh_full_tree(int capacity);
    void fill_partial_tree(int capacity);
    void fill_empty_tree(int capacity);
    void fill_up_to_count(int count);
    void draw_trace_window(int capacity);
};
