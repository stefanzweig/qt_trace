#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
#include "multiThread.h"
#include "DataTreeView.h"


QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    // ui.setupUi(this);
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
    if (mysub_ != nullptr) {
        delete mysub_;
        mysub_ = nullptr;
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
    if (demo_model != nullptr) {
        delete demo_model;
        demo_model = nullptr;
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
    updateCurrentState();
    setupTreeTrace();

    showMaximized();
}

void QtWidgetsApplication1::updateCurrentState() 
{
    qDebug() << "Updating...";

    // the following lines don't work.
    foreach(QAction * action, ui.toolbar->actions()) {
        bool checked = false;
        action->setChecked(checked);
    }
}

void QtWidgetsApplication1::updateState()
{
    //qDebug() << "State...";
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

    if (mysub_ == nullptr) {
        mysub_ = new ZoneMasterCanMessageDataSubscriber();
        qRegisterMetaType <can_frame>("can_frame");
        // connect(&mysub_->listener_, &SubListener::traceItemUpdate_internal, this, &QtWidgetsApplication1::formatRow);
        //connect(&mysub_->listener_, &SubListener::traceItemUpdate_internal_str, this, &QtWidgetsApplication1::formatRow_str);
        connect(&mysub_->listener_, &SubListener::traceItemUpdate_internal_cf, this, &QtWidgetsApplication1::formatRow_canframe);
    }
    calc_thread->restartThread();
    calc_thread->setSubscriber(mysub_, samples, ui.treetrace); // nonsense
    calc_thread->start();
    timer->start(1000);
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
    calc_thread->stopThread();
    mysub_ = nullptr;
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

    //t->header()->setHighlightSections(true);
    //t->header()->setDefaultAlignment(Qt::AlignCenter);
    //t->header()->setDefaultSectionSize(100);
    //t->header()->setStretchLastSection(true);
    //t->header()->setSortIndicator(0, Qt::AscendingOrder);

    //QFile file(":/QtWidgetsApplication1/default.txt");
    //file.open(QIODevice::ReadOnly);
    //model = new TreeModel(file.readAll());
    //file.close();
    //t->setModel(model);

    demo_model = new Demo();
    //t->setModel(demo_model);
    t->setWindowTitle(QObject::tr("Simple Tree Model"));
    //t->show();
}

void QtWidgetsApplication1::setupdatamodel()
{
    qDebug() << QString::number(full_canframes.count()) << endl;
    // todo foreach cf in full_canframes, bind it to the widget.
    QStringList str = {};
    for (int i =0; i< full_canframes.count(); i++)
    {
        if (full_canframes[i].Timestamp <= last_imestamp) continue;
        last_imestamp = full_canframes[i].Timestamp;
        qDebug() << "last_imestamp..." << last_imestamp << endl;
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(full_canframes[i].Timestamp/1000000);
        str.append(timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"));
        str.append(QString::number(full_canframes[i].Chn));
        str.append("0x" +QString::number(full_canframes[i].ID, 16));
        str.append(full_canframes[i].Name);
        str.append(full_canframes[i].Dir);
        str.append(QString::number(full_canframes[i].DLC));
        QString myData = "my data";
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
