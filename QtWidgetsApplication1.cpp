#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
//#include "HelloWorld/HelloWorldSubscriber.h"
#include "multiThread.h"


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
        delete calc_thread;
        calc_thread = nullptr;
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
    connect(calc_thread, SIGNAL(traceItemUpdate()), this, SLOT(formatRow()));

    // 重置布局 todo
    resetLayout();

    // 更新状态 todo
    updateCurrentState();

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
    qDebug() << "State...";
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
}

void QtWidgetsApplication1::onActionTriggered()
{
    qDebug() << tr("菜单项已触发！");
    // 下面的部分会有core dump问题
    //HelloWorldSubscriber mysub;
    //bool use_environment_qos = false;
    //if (mysub.init(use_environment_qos))
    //{
    //    mysub.run();
    //}
}

void QtWidgetsApplication1::startTrace()
{
    qDebug() << "startTrace...";
    std::cout << "Starting subscriber." << std::endl;

    uint32_t samples = 10;

    if (mysub_ == nullptr) { // 2024-05-17: in fact != should be == 
        // CanMessageDataWorkerSubscriber* mysub = new CanMessageDataWorkerSubscriber();
        mysub_ = new CanMessageDataWorkerSubscriber();
        //if (mysub->init())
        //{
        //    mysub->run(samples);
        //}
    }
    calc_thread->restartThread();
    calc_thread->setSubscriber(mysub_, samples);
    calc_thread->start(); // the real algorithm lies in calc_thread::run() function.
    //timer->start(1000);
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
    calc_thread->stopThread();
    mysub_ = nullptr;
    //if (mysub_ != nullptr) { 
    //    delete mysub_; 
    //    mysub_ = nullptr; 
    //}
    //timer->stop();
}

void QtWidgetsApplication1::pauseTrace()
{
    qDebug() << "pauseTrace...";
    calc_thread->pauseThread();
    //timer->stop();
}


void QtWidgetsApplication1::formatRow()
{
    qDebug() << "formatRow...";
}
