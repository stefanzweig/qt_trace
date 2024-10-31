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
    void setUsage(QStringList str_list);
private:
    void init();
};
