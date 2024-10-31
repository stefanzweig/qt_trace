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
}
