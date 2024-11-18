#pragma once
#include <QDialog>
#include "ui_configdialog.h"

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);
    ~ConfigDialog();
    Ui::ConfigDialog* ui;
private:
    void init();
};
