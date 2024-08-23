/********************************************************************************
** Form generated from reading UI file 'columnfilterdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef COLUMNFILTERDIALOG_H
#define COLUMNFILTERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_columnFilterDialog
{
public:
    QVBoxLayout *verticalLayout;
    QPushButton *resetFilterButton;
    //dgm
    QPushButton* confirmFilterButton;
    //dgm
    QFrame *line;
    QTableWidget *tableWidget;

    void setupUi(QDialog *columnFilterDialog)
    {
        if (columnFilterDialog->objectName().isEmpty())
            columnFilterDialog->setObjectName(QString::fromUtf8("columnFilterDialog"));
        columnFilterDialog->resize(200, 320);
        verticalLayout = new QVBoxLayout(columnFilterDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        resetFilterButton = new QPushButton(columnFilterDialog);
        resetFilterButton->setObjectName(QString::fromUtf8("resetFilterButton"));
        resetFilterButton->setContextMenuPolicy(Qt::ActionsContextMenu);
        resetFilterButton->setStyleSheet(QString::fromUtf8("QPushButton{text-align:left;}"));
        resetFilterButton->setAutoDefault(false);
        resetFilterButton->setFlat(true);

        verticalLayout->addWidget(resetFilterButton);

        //dgm
        confirmFilterButton = new QPushButton(columnFilterDialog);
        confirmFilterButton->setObjectName(QString::fromUtf8("resetFilterButton"));
        confirmFilterButton->setContextMenuPolicy(Qt::ActionsContextMenu);
        confirmFilterButton->setStyleSheet(QString::fromUtf8("QPushButton{text-align:left;}"));
        confirmFilterButton->setAutoDefault(false);
        confirmFilterButton->setFlat(true);
        verticalLayout->addWidget(confirmFilterButton);
        //dgm

        line = new QFrame(columnFilterDialog);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        tableWidget = new QTableWidget(columnFilterDialog);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));

        verticalLayout->addWidget(tableWidget);


        retranslateUi(columnFilterDialog);

        resetFilterButton->setDefault(false);


        QMetaObject::connectSlotsByName(columnFilterDialog);
    } // setupUi

    void retranslateUi(QDialog *columnFilterDialog)
    {
        columnFilterDialog->setWindowTitle(QCoreApplication::translate("columnFilterDialog", "Dialog", nullptr));
        resetFilterButton->setText(QCoreApplication::translate("columnFilterDialog", "Reset Filter", nullptr));
        //dgm
        confirmFilterButton->setText(QCoreApplication::translate("columnFilterDialog", "Confirm Filter", nullptr));
        //dgm
    } // retranslateUi

};

namespace Ui {
    class columnFilterDialog: public Ui_columnFilterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // COLUMNFILTERDIALOG_H
