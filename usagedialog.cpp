#include "usagedialog.h"

UsageDialog::UsageDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::UsageDialog)
{
    init();
}

UsageDialog::~UsageDialog()
{
    delete ui;
}

void UsageDialog::init() {
    ui->setupUi(this);
    ui->plainTextEdit->setReadOnly(true);
}

void UsageDialog::setUsage(QStringList str_list) {
    QString s = str_list.join("\n");
    ui->plainTextEdit->setPlainText(s);
}