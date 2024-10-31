/********************************************************************************
** Form generated from reading UI file 'usagedialogJPShrC.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef USAGEDIALOGJPSHRC_H
#define USAGEDIALOGJPSHRC_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UsageDialog
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPlainTextEdit *plainTextEdit;

    void setupUi(QDialog *UsageDialog)
    {
        if (UsageDialog->objectName().isEmpty())
            UsageDialog->setObjectName(QString::fromUtf8("UsageDialog"));
        UsageDialog->resize(400, 297);
        layoutWidget = new QWidget(UsageDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 250, 381, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        plainTextEdit = new QPlainTextEdit(UsageDialog);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setGeometry(QRect(10, 20, 381, 221));

        retranslateUi(UsageDialog);
        QObject::connect(okButton, SIGNAL(clicked()), UsageDialog, SLOT(accept()));

        QMetaObject::connectSlotsByName(UsageDialog);
    } // setupUi

    void retranslateUi(QDialog *UsageDialog)
    {
        UsageDialog->setWindowTitle(QCoreApplication::translate("UsageDialog", "Usage", nullptr));
        okButton->setText(QCoreApplication::translate("UsageDialog", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UsageDialog: public Ui_UsageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // USAGEDIALOGJPSHRC_H
