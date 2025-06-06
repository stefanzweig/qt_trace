#include "columnfilter.h"
#include <iostream>
#include <QDebug>

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
    if (!connected) {
        connect(ui->confirmFilterButton, &QPushButton::clicked, this, &columnFilterDialog::on_confirmFilterButton_clicked);
        connect(ui->resetFilterButton, &QPushButton::clicked, this, &columnFilterDialog::on_resetFilterButton_clicked);
        ui->resetFilterButton->setIcon(QIcon(":/QtWidgetsApplication1/res/tick.svg"));
        ui->confirmFilterButton->setIcon(QIcon(":/QtWidgetsApplication1/res/info.svg"));

        ui->resetFilterButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border-radius: 3px; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #397d3b; }");

        ui->confirmFilterButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border-radius: 3px; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #397d3b; }");

        ui->confirmFilterButton->setAutoDefault(true);
        setWindowTitle(tr("Filter"));
        connected = true;
    }
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
    qDebug() << "reset filter button clicked";
    reset_column_flag[columnButton] = 0;
    comfirm_button_flag[columnButton] = 0;
    twoSelectedItems.clear();
    selectedItemscount = 0;

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
        m_selectedStates[columnButton][i] = false;
    new_checks.remove(colName);
    emit filter_apply(twoSelectedItems, selectedItemscount);
    accept();
}

void columnFilterDialog::on_confirmFilterButton_clicked() {

    QList<QList<QString>>  selectedItems;
    QList<QVariant> newChecks;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem* item = ui->tableWidget->item(i, 0);
        if (item->checkState() == Qt::Checked) 
        {
            QList<QString> newRow;
            newRow.append(item->text());
            newChecks.append(item->text());
            selectedItems.append(newRow);
            m_selectedStates[columnButton][i] = true;
        }
        else {
            m_selectedStates[columnButton][i] = false;
        }
    }
    new_checks.insert(colName, newChecks);
    accept();
    emit filter_apply(selectedItems, selectedItemscount);
}
