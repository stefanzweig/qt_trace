#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include <QTimer>

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

private slots:
    void updateCurrentState();
    void onActionTriggered();

signals:
    void resetLayout();
};
