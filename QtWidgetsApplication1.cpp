#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
#include "HelloWorld/HelloWorldSubscriber.h"


QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    // ui.setupUi(this);
    init();
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{
}

void QtWidgetsApplication1::init()
{
    ui.setupUi(this);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ui.widget->sizePolicy().hasHeightForWidth());
    ui.Lower->setSizePolicy(sizePolicy);
    ui.toolbar_search->setVisible(true);
    
    // 定义动作，菜单和工具栏
    createActions();

    // 定义定时器
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(updateCurrentState()));

    // 更新状态 todo
    updateCurrentState();

    // 重置布局 todo
    resetLayout();
}

void QtWidgetsApplication1::updateCurrentState() 
{
    qDebug() << "Updating...";
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

    QAction* action = new QAction("Temp Action");
    ui.menu->addAction(action);
    connect(action, &QAction::triggered, this, &QtWidgetsApplication1::onActionTriggered);
}

void QtWidgetsApplication1::onActionTriggered()
{
    qDebug() << "菜单项已触发！";
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
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
}

void QtWidgetsApplication1::pauseTrace()
{
    qDebug() << "pauseTrace...";
}
