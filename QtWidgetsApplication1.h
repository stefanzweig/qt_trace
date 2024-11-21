#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <QTimer>
#include <QQueue>
#include <QLabel>

#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/ZoneMasterCanParserSubscriber.h"
#include "topicData/ZoneMasterLinMessageDataSubscriber.h"
#include "topicData/ZoneMasterLinParserDataSubscriber.h"

#include "multiThread.h"
#include "zm_struct.h"
#include "columnfilter.h"
#include "spdlog/spdlog.h"
#include <QReadWriteLock>
#include "TraceTreeWidgetItem.h"
#include "StateManager.h"
#include "lexer.h"
#include "usagedialog.h"
#include "dialog/configdialog.h"

#define TIMER_HEARTBEAT 100
#define MAX_ITEM_COUNT 5000

enum DISPLAY_MODE
{
    APPEND = 0,
    AGGREGATION = 1,
};

// for the found items highlight 2024-11-02 20:48:04
class Found_Item
{
public:
    QTreeWidgetItem* item;
    int col = -1;
    void highlight(QTreeWidget* widget)
    {
        widget->setCurrentItem(item);
        item->setBackground(col, QColor(255, 255, 0));
    };
    void dehighlight()
    {
        item->setBackground(col, QColor(255, 255, 255));
    };
};

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget* parent = nullptr);
    ~QtWidgetsApplication1();

protected:
    void createActions();
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override {
        QMainWindow::resizeEvent(event);
        this->adjust_filter_buttons();
    }
    void form_conditions_simple(QString findstr);
    void form_conditions_compiler(QString findstr);

private:
    Ui::QtWidgetsApplication1Class ui;
    void init();
    void shortcuts();
    void setupTreeTrace();
    void initialHeaders();
    void get_default_configurations();
    void compare_item();
    void traceStyleQSS();
    void init_mylogger();
    TraceTreeWidgetItem* read_item_from_queue(int index);
    TraceTreeWidgetItem* read_item_from_dumb(int index);
    bool parser2condition(QList<Token> token_list);
    const QString treewidget_header_style = \
        "QHeaderView::section {"
        "   font-weight: bold;"
        "   background-color: #FFFFFF;"
        "   color: #464646;"
        "   padding: 1px;"
        "   border: none; "
        "   border-bottom: 1px solid #000;"
        "}";

    QTimer* timer = nullptr;
    QTimer* timer_dustbin = nullptr;
    ZoneMasterCanMessageDataSubscriber* mysub_can_frames = nullptr;
    ZoneMasterCanParserSubscriber* mysub_can_parser = nullptr;

    ZoneMasterLinMessageDataSubscriber* mysub_lin_frames = nullptr;
    ZoneMasterLinParserSubscriber* mysub_lin_parser = nullptr;

    multiThread* calc_thread = nullptr;
    QQueue<TraceTreeWidgetItem*> shown_queue;
    QQueue<TraceTreeWidgetItem*> full_queue_stream;
    QQueue<TraceTreeWidgetItem*> filtered_queue;
    int filtered_size() { return filtered_queue.size(); }
    uint64_t last_timestamp = 0;
    uint64_t last_timestamp_canparser = 0;

    uint64_t full_count_canframes = 0;
    uint64_t full_count_canparser = 0;
    uint64_t full_count_linframes = 0;
    uint64_t full_count_linparser = 0;

    uint64_t count_per_page = 4000; // the full length of data in the window when it is paused.
    int page_capacity = 300; // the trace windows maximal rows of data
    bool isHex = true; // repr in hex
    int max_page_count = 0; // (full_count_canframes / count_per_page)
    int current_page_index = 0; // the current page index of all items
    int current_item_index = 0; // the index of the item inside a page

    QStringList initialHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "EventType", "DataLength", "BusType", "Data" };
    QStringList CurrentHeader = initialHeader;
    QStringList headers = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC", "EventType", "DataLength", "BusType", "Data" };
    QStringList linHeader = { "Time[ms]", "Chn", "ID", "Name", "Dir", "DataLength", "EventType", "BusType", "Data" };
    QStringList ethHeader = { "Time[ms]","Chn","Dir","Source MAC","Destination MAC","Source IP","Destination IP","Protocol","Payload Data","VLAN ID","Transport Layer","Service","Service Instance","Method","Message Type","Port(s)","VLAN Priority" };
    QComboBox* datachoice = nullptr;

    QList<QPushButton*> headerButtonList;
    columnFilterDialog* filter = nullptr;  // filter dialog
    UsageDialog* usagedialog = nullptr;
    ConfigDialog* configdialog = nullptr;
    QHash<QString, QString> filterConfig;
    QVector<QVector<bool>> selectedStates;

    QLabel* leftLabel = nullptr;
    QWidget* leftWidget = nullptr;
    QHBoxLayout* leftLayout = nullptr;
    QLabel* rightLabel = nullptr;
    QWidget* rightWidget = nullptr;
    QHBoxLayout* rightLayout = nullptr;

    // further configurations
    DISPLAY_MODE display_mode = DISPLAY_MODE::APPEND;
    int visible_height = 700;
    int item_height = 20;  /* default item height in the trace widget. */
    int item_width = 20;
    int dds_domainid = 90;
    int last_data_index = -1;
    int padding = 1;
    bool frozen = false;
    bool initial_trace = true;
    QList<QString> monitor_modules;
    QHash<QString, QList<QVariant>> new_filters;
    bool filter_pop = false;
    QDateTime start_time;
    QDateTime end_time;
    qint64 progress_secs;
    int paused_instant_index = 0;
    QString last_status = "INITIAL";
    bool timer_isRunning = false; // whether it is in a process of timer, a lock.
    QReadWriteLock rwLock;
    StateManager state_manager;
    QMutex m_mutex;
    int debuglog = 1; // not in use so far
    int maximum_total = 400000;
    QSet<QString> passed_uuid_set;
    QMap<int, QStringList> list_map;
    bool full_search = false;
    int  condition_type = 0; // 0 - no, 1 - column, 2 - signal, 3 - mixed
    QSet<QTreeWidgetItem*> invisibles;
    QStringList USAGE;

private slots:
    void startTrace();
    void stopTrace();
    void pauseTrace();
    void replayTrace();
    void resetLayout();
    void toggle_display_mode();
    void switch_display_mode(DISPLAY_MODE display_mode);
    void resetStatusBar();
    void resumeTrace();
    void updateState();
    void onActionTriggered();
    void applyFilter(QList<QList<QString>> items, int count);
    void on_pop_to_root(TraceTreeWidgetItem* item);
    void headerButtonClicked();
    void prepareMenu(const QPoint& pos);
    void newDev();
    void ChangeHeader(const QString& text);
    void ButtonSearchClicked();
    void dustbin();
    void updateToolbar();
    void update_tracewindow();
    void trace_scroll_changed(int value);
    bool new_session();
    void freeze_treetrace_items(int ncount);
    void adjust_filter_buttons();
    void on_header_section_resized();
    void on_horizontal_scroll();
    void about();
    void reset_all_filters();
    void refresh_full_tree(int capacity);
    void fill_partial_tree(int capacity);
    void fill_empty_tree(int capacity);
    void fill_up_to_count(int count);
    void draw_trace_window(int capacity);
    void updateProgressTimer();
    bool showNewSession();
    void clearance();
    void updateContinuousProgress();
    void construct_page_data(TraceTreeWidgetItem* item);
    bool filter_pass_item(QTreeWidgetItem* it);
    bool filter_run_pass_item_without_children(QTreeWidgetItem* it);
    void show_fullpage();
    void show_fullpage_with_index(int index);
    void restore_full_queue();
    void safe_clear_trace();
    QString previous_state();
    void print_item_queue(QQueue<TraceTreeWidgetItem*> queue);
    void clear_queue(QQueue<TraceTreeWidgetItem*>& queue);
    void resume_from_pause_trace();
    void initialize_new_session();
    void update_latest_index(uint64_t index);
    void updateProgressRunStatus();
    void updateComoboPage();
    void ButtonFirstClicked();
    void ButtonPreviousClicked();
    void ButtonNextClicked();
    void ButtonLastClicked();
    void ButtonGotoClicked();
    void show_fullpage_with_findings();
    void highlight_previous_item(QTreeWidgetItem* item);
    void highlight_next_item(QTreeWidgetItem* item);
    void extract_item(QTreeWidgetItem* item);
    void collapse_item(QTreeWidgetItem* item);
    QQueue<QTreeWidgetItem*> get_filtered_queue_front();
    QQueue<QTreeWidgetItem*> get_filtered_queue_tail();
    void construct_filtered_queue(int full_count);
    void updateHeaderStyle(QTreeWidgetItem* item);
    void resetInvisibles();
    void construct_searching_string();
    void help_usage();
    void trace_config();
    void construct_usage();
    void current2center();
    void collapse_all();
    void clearlog();


signals:
    void record_latest_index(uint64_t index);
    void buttonClicked();

protected:
    bool application_in_find = false;
    int find_direction = 0; // 0 -> down, 1 -> up.
    QString last_findstring;
    void reset_find_status() {
        application_in_find = false;
        for (Found_Item* t : found_queue) {
            //if (current_find == t)
            //    current_find->dehighlight();
            delete t;
            t = nullptr;
        };
        found_queue.clear();
        current_find = nullptr;
        prev_find = nullptr;
        next_find = nullptr;
    }
    void enter_find_status() { application_in_find = true; }
    bool current_find_status() { return application_in_find; }
    int foundcount() { return found_queue.size(); }
    QQueue<Found_Item*> remove_duplicates(QQueue<Found_Item*> queue);
    bool findItem(QTreeWidgetItem* item, int col, const QStringList& target,
        QSet<QTreeWidgetItem*>& hiddens, QQueue<Found_Item*>& founds, int level = 0);

    void searchTree_bfs(QTreeWidgetItem* root, 
        const QStringList& targets,
        QQueue<Found_Item*>& queue,
        QSet<Found_Item*>& visited);

    QQueue<Found_Item*> found_queue;
    //QTreeWidgetItem* current_find = nullptr;
    Found_Item* current_find = nullptr;
    Found_Item* prev_find = nullptr;
    QTreeWidgetItem* next_find = nullptr;
    // the above are the ones related to find.

    // the following functions should be moved to other section.
    // todo. 2024-11-01 14:28:53
    void showEvent(QShowEvent* event) override {
        // 连接自定义信号到槽
        connect(this, &QtWidgetsApplication1::buttonClicked, this, &QtWidgetsApplication1::ButtonSearchClicked);
        QWidget::showEvent(event);
    }
    bool focusNextPrevChild(bool next) override {
        return QWidget::focusNextPrevChild(next);
    }
};
