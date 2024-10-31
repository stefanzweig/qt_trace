#pragma once
#include <QDialog>
#include "ui_usagedialog.h"

class UsageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsageDialog(QWidget* parent = nullptr);
    ~UsageDialog();
    Ui::UsageDialog* ui;
private:
    void init();
};
