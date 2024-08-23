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
    connect(ui->confirmFilterButton, &QPushButton::clicked, this, &columnFilterDialog::on_confirmFilterButton_clicked);
    connect(ui->resetFilterButton, &QPushButton::clicked, this, &columnFilterDialog::on_resetFilterButton_clicked);
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
    reset_column_flag[columnButton] = 0;
    comfirm_button_flag[columnButton] = 0;
    twoSelectedItems.clear();
    selectedItemscount = 0;
    accept();

}

void columnFilterDialog::on_confirmFilterButton_clicked() {
    std::cout << "on_confirmedButton_clicked clicked" << std::endl;

    int numColumns = 13;
    int numRows = 1000;
    for (int col = 0; col < numColumns; ++col) {
        QList<QString> newRow;
        for (int row = 0; row < numRows; ++row) {
            newRow.append(QString("item_%1_%2").arg(col).arg(row));
        }
        twoSelectedItems.append(newRow);
    }
    int rows = 13;
    int cols = 1000;
    m_selectedStates.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        m_selectedStates.append(QVector<bool>(cols, false));
    }

    reset_column_flag[columnButton] = 1;
    comfirm_button_flag[columnButton] = 1;
    selectedItemscount = 0;
    int intercount = 0;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QTableWidgetItem* item = ui->tableWidget->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            twoSelectedItems[columnButton][intercount] = (item->text());
            selectedItemscount++;
            m_selectedStates[columnButton][i] = true;
            intercount++;
        }
        else {
            m_selectedStates[columnButton][i] = false;
        }
    }
    accept();
    emit filter_apply(twoSelectedItems, selectedItemscount);
}
