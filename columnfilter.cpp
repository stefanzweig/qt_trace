#include "columnfilter.h"
#include <iostream>

columnFilterDialog::columnFilterDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::columnFilterDialog)
{
    init();
}

columnFilterDialog::~columnFilterDialog()
{
    delete ui;
}

void columnFilterDialog::init() {
    ui->setupUi(this);
}

void columnFilterDialog::closeEvent(QCloseEvent* e) {
    std::cout << "column filter dialog closed" << std::endl;
    QList<QString> filterconfig;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        if (ui->tableWidget->item(i, 0)->checkState() == 2) {
            filterconfig << ui->tableWidget->item(i, 0)->text();
        }
        std::cout << "check state " << ui->tableWidget->item(i, 0)->checkState() << std::endl;
    }
    emit filterConfig(filterconfig);
}


void columnFilterDialog::on_resetFilterButton_clicked() {
    std::cout << "reset filter button clicked" << std::endl;
    emit resetFilterFlag(columnIndex);
}
