#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
#include "multiThread.h"
//#include "DataTreeView.h"


QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    init();
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{
    if (this->timer) {
        if (timer->isActive())
            timer->stop();
        delete timer;
        timer = nullptr;
    }
    if (mysub_can_frames != nullptr) {
        delete mysub_can_frames;
        mysub_can_frames = nullptr;
    }
    if (calc_thread != nullptr) {
        calc_thread->stopFlag();

        calc_thread->quit();
        calc_thread->wait();

        delete calc_thread;
        calc_thread = nullptr;
    }
    if (model != nullptr) {
        delete model;
        model = nullptr;
    }
}

void QtWidgetsApplication1::init()
{
    ui.setupUi(this);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ui.widget->sizePolicy().hasHeightForWidth());
    ui.Lower->setSizePolicy(sizePolicy);
    //ui.toolbar_search->setVisible(true);
    
    // 定义动作，菜单和工具栏
    createActions();

    // 定义定时器
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &QtWidgetsApplication1::updateState);

    calc_thread = new multiThread();
    connect(calc_thread, &multiThread::traceItemUpdate_internal, [=]() {qDebug() << "lambda"; });


    // 重置布局 todo
    resetLayout();

    // 更新状态 todo
    _updateCurrentState();
    setupTreeTrace();

    showMaximized();
}

void QtWidgetsApplication1::_updateCurrentState()
{
    ui.statusBar->showMessage(QString::number(this->full_count));
    //foreach(QAction * action, ui.toolbar->actions()) {
    //    bool checked = false;
    //    action->setChecked(checked);
    //}
}

void QtWidgetsApplication1::updateState()
{
    _updateCurrentState();
}

void QtWidgetsApplication1::createActions()
{
    qDebug() << "createActions...";
    connect(ui.actionstart, &QAction::triggered, this, &QtWidgetsApplication1::startTrace);
    connect(ui.actionstop, &QAction::triggered, this, &QtWidgetsApplication1::stopTrace);
    connect(ui.actionpause, &QAction::triggered, this, &QtWidgetsApplication1::pauseTrace);
}

void QtWidgetsApplication1::resetLayout()
{
    qDebug() << "resetLayout...";

    // will be removed later.
    QAction* action = new QAction("Temp Action");
    ui.menu->addAction(action);
    connect(action, &QAction::triggered, this, &QtWidgetsApplication1::onActionTriggered);

    // icons
    ui.menu->setIcon(QIcon(":/QtWidgetsApplication1/res/sqlitestudio.ico"));

    ui.actionpause->setIcon(QIcon(":/QtWidgetsApplication1/res/pauseTrace.png"));
    ui.actionstart->setIcon(QIcon(":/QtWidgetsApplication1/res/startTrace.png"));
    ui.actionstop->setIcon(QIcon(":/QtWidgetsApplication1/res/stopTrace.png"));

    // toolbar actions
    ui.toolbar->addAction(ui.actionstart);
    ui.toolbar->addAction(ui.actionstop);
    ui.toolbar->addAction(ui.actionpause);
    initialHeaders();
}

void QtWidgetsApplication1::onActionTriggered()
{
    qDebug() << tr("菜单项已触发！");
}

void QtWidgetsApplication1::startTrace()
{
    qDebug() << "startTrace...";
    std::cout << "Starting subscriber." << std::endl;

    uint32_t samples = 10;

    if (mysub_can_frames == nullptr) {
        mysub_can_frames = new ZoneMasterCanMessageDataSubscriber();
        qRegisterMetaType <can_frame>("can_frame");
        // connect(&mysub_can_frames->listener_, &SubListener::traceItemUpdate_internal, this, &QtWidgetsApplication1::formatRow);
        //connect(&mysub_can_frames->listener_, &SubListener::traceItemUpdate_internal_str, this, &QtWidgetsApplication1::formatRow_str);
        connect(&mysub_can_frames->listener_, &SubListener::traceItemUpdate_internal_cf, this, &QtWidgetsApplication1::formatRow_canframe);
    }
    calc_thread->restartThread();
    calc_thread->setSubscriber(mysub_can_frames, samples, ui.treetrace); // nonsense
    calc_thread->start();
    timer->start(1000);
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
    calc_thread->stopThread();
    mysub_can_frames = nullptr;
    timer->stop();
}

void QtWidgetsApplication1::pauseTrace()
{
    qDebug() << "pauseTrace...";
    calc_thread->pauseThread();
    timer->stop();
}


void QtWidgetsApplication1::formatRow(int x)
{
    qDebug() << "formatRow..."<< x;
}

void QtWidgetsApplication1::formatRow_str(QString s)
{
    qDebug() << "formatRow..." << s;
}


void QtWidgetsApplication1::formatRow_canframe(can_frame cf)
{
    //qDebug() << "formatRow...cf->";
    //qDebug() << cf.ID << endl;
    full_canframes.append(cf);
    setupdatamodel();
}

void QtWidgetsApplication1::setupTreeTrace()
{
    //DataTreeView* t = ui.treetrace;
    QTreeWidget* t = ui.treetrace;
    t->setSelectionBehavior(QTreeView::SelectRows);
    t->setSelectionMode(QTreeView::SingleSelection);
    t->setFocusPolicy(Qt::NoFocus);

    t->header()->setHighlightSections(true);
    //t->header()->setDefaultAlignment(Qt::AlignCenter);
    //t->header()->setDefaultSectionSize(100);
    t->header()->setStretchLastSection(true);
    t->header()->setSortIndicator(0, Qt::AscendingOrder);
    t->setWindowTitle(QObject::tr("CAN Frames"));
    //t->show();
}

void QtWidgetsApplication1::setupdatamodel()
{
    QStringList str = {};
    for (int i =0; i< full_canframes.count(); i++)
    {
        if (full_canframes[i].Timestamp <= last_imestamp) continue;

        full_count++; // message counts
        int count_in_page = full_count % count_per_page;
        int last_page_index = full_count / count_per_page;
        if (last_page_index > 0) {
            if (last_page_index != current_page)
                if (count_in_page != 0) {
                    QTreeWidget* t = ui.treetrace;
                    t->clear();
                    current_page = last_page_index;
                    qDebug() << full_count << endl;
                }
        }
        
        last_imestamp = full_canframes[i].Timestamp;
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(full_canframes[i].Timestamp/1000000);
        str.append(timestamp.toString("hh:mm:ss.zzz"));
        str.append(QString::number(full_canframes[i].Chn));
        str.append("0x" +QString::number(full_canframes[i].ID, 16));
        str.append(full_canframes[i].Name);
        str.append(full_canframes[i].Dir);
        str.append(QString::number(full_canframes[i].DLC));
        QString myData = full_canframes[i].Data_Str;
        str.append(myData);
        str.append(full_canframes[i].EventType);
        str.append(QString::number(full_canframes[i].DataLength));
        str.append(full_canframes[i].BusType);
        QTreeWidgetItem* Item = new QTreeWidgetItem(str);
        QTreeWidget* t = ui.treetrace;
        t->addTopLevelItem(Item);
        t->setIndentation(20);
    }
}

void QtWidgetsApplication1::initialHeaders()
{
    QTreeWidget* tree = ui.treetrace;
    QHeaderView* header = tree->header();
    header->setDefaultSectionSize(200);
    header->setSectionResizeMode(QHeaderView::Interactive);
    tree->setHeaderLabels(initialHeader);
    tree->setSortingEnabled(true);
    tree->setColumnWidth(0, 250);
    tree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    tree->invisibleRootItem()->setHidden(true);
}
