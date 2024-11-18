#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    init();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::init() {
    ui->setupUi(this);
}
