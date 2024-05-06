#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>


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

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(updateCurrentState()));

    updateCurrentState();

}

void QtWidgetsApplication1::updateCurrentState() 
{
    qDebug() << "Updating...";
}
