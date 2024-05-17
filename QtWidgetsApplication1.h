#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <QTimer>
#include "topicData/CanMessageDataWorkerSubscriber.h"

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

    Ui::QtWidgetsApplication1Class ui;
    QTimer* timer = nullptr;
    CanMessageDataWorkerSubscriber* mysub_ = nullptr;

private slots:
    void updateState();
    void updateCurrentState();
    void onActionTriggered();

    void startTrace();
    void stopTrace();
    void pauseTrace();


signals:
    void resetLayout();
};
