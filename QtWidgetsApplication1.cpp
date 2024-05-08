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
    createActions();

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(updateCurrentState()));

    updateCurrentState();
    resetLayout();

}

void QtWidgetsApplication1::updateCurrentState() 
{
    qDebug() << "Updating...";
}

void QtWidgetsApplication1::createActions()
{
    qDebug() << "createActions...";
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
    //qDebug() << "菜单项已触发！";
    HelloWorldSubscriber mysub;
    bool use_environment_qos = false;
    if (mysub.init(use_environment_qos))
    {
        mysub.run();
    }
}