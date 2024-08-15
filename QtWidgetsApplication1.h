#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <QTimer>
#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "multiThread.h"
#include "treemodel.h"
#include "demo.h"
#include "zm_struct.h"

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
    void setupTreeTrace();

    Ui::QtWidgetsApplication1Class ui;
    QTimer* timer = nullptr;
    ZoneMasterCanMessageDataSubscriber* mysub_ = nullptr;
    multiThread* calc_thread = nullptr;
    TreeModel* model = nullptr;
    Demo* demo_model = nullptr;
    QVector<can_frame> full_canframes;

private slots:
    void updateState();
    void updateCurrentState();
    void onActionTriggered();
    void formatRow(int x);
    void formatRow_str(QString s);
    void formatRow_canframe(can_frame cf);

    void startTrace();
    void stopTrace();
    void pauseTrace();


signals:
    void resetLayout();
};
