/********************************************************************************
** Form generated from reading UI file 'QtWidgetsApplication1.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTWIDGETSAPPLICATION1_H
#define UI_QTWIDGETSAPPLICATION1_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtWidgetsApplication1Class
{
public:
    QAction *actionstart;
    QAction *actionpause;
    QAction *actionstop;
    QAction *actionConfig;
    QAction *actionmode;
    QAction *actionAbout;
    QAction *actionreset;
    QAction *actionreplay;
    QAction *actionHelp;
    QAction *actionclear;
    QWidget *ZSTraceWindow;
    QVBoxLayout *verticalLayout;
    QMenuBar *menubar;
    QMenu *menu;
    QMenu *menuHelp;
    QToolBar *toolbar;
    QWidget *mainWidgt;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QWidget *bodyWidget;
    QVBoxLayout *verticalLayout_4;
    QWidget *widget;
    QVBoxLayout *gridLayout_2;
    QWidget *Upper;
    QHBoxLayout *hLayout_1;
    QPlainTextEdit *mysearch;
    QPushButton *pb_prev_result;
    QPushButton *pb_next_result;
    QPushButton *pushButton_search;
    QWidget *Lower;
    QVBoxLayout *hLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QComboBox *comboBox;
    QLabel *label_Filtered;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pb_First;
    QPushButton *pb_Prev;
    QLabel *label_Current;
    QPushButton *pb_Next;
    QPushButton *pb_Last;
    QComboBox *comboBox_Page;
    QPushButton *pb_Goto;
    QHBoxLayout *horizontalLayout_99;
    QSplitter *splitter1;
    QDockWidget *left_statistic_widget;
    QTreeWidget *treetrace;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtWidgetsApplication1Class)
    {
        if (QtWidgetsApplication1Class->objectName().isEmpty())
            QtWidgetsApplication1Class->setObjectName(QString::fromUtf8("QtWidgetsApplication1Class"));
        QtWidgetsApplication1Class->setWindowModality(Qt::WindowModal);
        QtWidgetsApplication1Class->resize(869, 426);
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(QtWidgetsApplication1Class->sizePolicy().hasHeightForWidth());
        QtWidgetsApplication1Class->setSizePolicy(sizePolicy);
        actionstart = new QAction(QtWidgetsApplication1Class);
        actionstart->setObjectName(QString::fromUtf8("actionstart"));
        actionpause = new QAction(QtWidgetsApplication1Class);
        actionpause->setObjectName(QString::fromUtf8("actionpause"));
        actionstop = new QAction(QtWidgetsApplication1Class);
        actionstop->setObjectName(QString::fromUtf8("actionstop"));
        actionConfig = new QAction(QtWidgetsApplication1Class);
        actionConfig->setObjectName(QString::fromUtf8("actionConfig"));
        actionConfig->setEnabled(false);
        actionConfig->setVisible(false);
        actionConfig->setIconVisibleInMenu(false);
        actionConfig->setShortcutVisibleInContextMenu(false);
        actionmode = new QAction(QtWidgetsApplication1Class);
        actionmode->setObjectName(QString::fromUtf8("actionmode"));
        actionAbout = new QAction(QtWidgetsApplication1Class);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionreset = new QAction(QtWidgetsApplication1Class);
        actionreset->setObjectName(QString::fromUtf8("actionreset"));
        actionreplay = new QAction(QtWidgetsApplication1Class);
        actionreplay->setObjectName(QString::fromUtf8("actionreplay"));
        actionHelp = new QAction(QtWidgetsApplication1Class);
        actionHelp->setObjectName(QString::fromUtf8("actionHelp"));
        actionclear = new QAction(QtWidgetsApplication1Class);
        actionclear->setObjectName(QString::fromUtf8("actionclear"));
        ZSTraceWindow = new QWidget(QtWidgetsApplication1Class);
        ZSTraceWindow->setObjectName(QString::fromUtf8("ZSTraceWindow"));
        sizePolicy.setHeightForWidth(ZSTraceWindow->sizePolicy().hasHeightForWidth());
        ZSTraceWindow->setSizePolicy(sizePolicy);
        ZSTraceWindow->setAcceptDrops(true);
        ZSTraceWindow->setWindowTitle(QString::fromUtf8("Form"));
        verticalLayout = new QVBoxLayout(ZSTraceWindow);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        menubar = new QMenuBar(ZSTraceWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(menubar->sizePolicy().hasHeightForWidth());
        menubar->setSizePolicy(sizePolicy1);
        menubar->setAcceptDrops(false);
        menubar->setDefaultUp(false);
        menu = new QMenu(menubar);
        menu->setObjectName(QString::fromUtf8("menu"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));

        verticalLayout->addWidget(menubar);

        toolbar = new QToolBar(ZSTraceWindow);
        toolbar->setObjectName(QString::fromUtf8("toolbar"));

        verticalLayout->addWidget(toolbar);

        mainWidgt = new QWidget(ZSTraceWindow);
        mainWidgt->setObjectName(QString::fromUtf8("mainWidgt"));
        horizontalLayout = new QHBoxLayout(mainWidgt);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        splitter = new QSplitter(mainWidgt);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        bodyWidget = new QWidget(splitter);
        bodyWidget->setObjectName(QString::fromUtf8("bodyWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(4);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(bodyWidget->sizePolicy().hasHeightForWidth());
        bodyWidget->setSizePolicy(sizePolicy2);
        verticalLayout_4 = new QVBoxLayout(bodyWidget);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        widget = new QWidget(bodyWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy3);
        gridLayout_2 = new QVBoxLayout(widget);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, -1, 0);
        Upper = new QWidget(widget);
        Upper->setObjectName(QString::fromUtf8("Upper"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(1);
        sizePolicy4.setHeightForWidth(Upper->sizePolicy().hasHeightForWidth());
        Upper->setSizePolicy(sizePolicy4);
        hLayout_1 = new QHBoxLayout(Upper);
        hLayout_1->setObjectName(QString::fromUtf8("hLayout_1"));
        mysearch = new QPlainTextEdit(Upper);
        mysearch->setObjectName(QString::fromUtf8("mysearch"));
        mysearch->setEnabled(true);
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(19);
        sizePolicy5.setVerticalStretch(1);
        sizePolicy5.setHeightForWidth(mysearch->sizePolicy().hasHeightForWidth());
        mysearch->setSizePolicy(sizePolicy5);
        mysearch->setMinimumSize(QSize(256, 30));
        mysearch->setMaximumSize(QSize(16777215, 30));

        hLayout_1->addWidget(mysearch);

        pb_prev_result = new QPushButton(Upper);
        pb_prev_result->setObjectName(QString::fromUtf8("pb_prev_result"));
        QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(pb_prev_result->sizePolicy().hasHeightForWidth());
        pb_prev_result->setSizePolicy(sizePolicy6);
        pb_prev_result->setMinimumSize(QSize(20, 20));
        pb_prev_result->setMaximumSize(QSize(24, 24));

        hLayout_1->addWidget(pb_prev_result);

        pb_next_result = new QPushButton(Upper);
        pb_next_result->setObjectName(QString::fromUtf8("pb_next_result"));
        QSizePolicy sizePolicy7(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(pb_next_result->sizePolicy().hasHeightForWidth());
        pb_next_result->setSizePolicy(sizePolicy7);
        pb_next_result->setMaximumSize(QSize(24, 24));

        hLayout_1->addWidget(pb_next_result);

        pushButton_search = new QPushButton(Upper);
        pushButton_search->setObjectName(QString::fromUtf8("pushButton_search"));
        QSizePolicy sizePolicy8(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy8.setHorizontalStretch(1);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(pushButton_search->sizePolicy().hasHeightForWidth());
        pushButton_search->setSizePolicy(sizePolicy8);
        pushButton_search->setMinimumSize(QSize(0, 24));
        pushButton_search->setMaximumSize(QSize(16777215, 24));

        hLayout_1->addWidget(pushButton_search);


        gridLayout_2->addWidget(Upper);

        Lower = new QWidget(widget);
        Lower->setObjectName(QString::fromUtf8("Lower"));
        QSizePolicy sizePolicy9(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy9.setHorizontalStretch(0);
        sizePolicy9.setVerticalStretch(15);
        sizePolicy9.setHeightForWidth(Lower->sizePolicy().hasHeightForWidth());
        Lower->setSizePolicy(sizePolicy9);
        hLayout_2 = new QVBoxLayout(Lower);
        hLayout_2->setObjectName(QString::fromUtf8("hLayout_2"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        comboBox = new QComboBox(Lower);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        horizontalLayout_6->addWidget(comboBox);

        label_Filtered = new QLabel(Lower);
        label_Filtered->setObjectName(QString::fromUtf8("label_Filtered"));
        QSizePolicy sizePolicy10(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy10.setHorizontalStretch(0);
        sizePolicy10.setVerticalStretch(0);
        sizePolicy10.setHeightForWidth(label_Filtered->sizePolicy().hasHeightForWidth());
        label_Filtered->setSizePolicy(sizePolicy10);
        label_Filtered->setMinimumSize(QSize(60, 0));
        label_Filtered->setAlignment(Qt::AlignCenter);

        horizontalLayout_6->addWidget(label_Filtered);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        pb_First = new QPushButton(Lower);
        pb_First->setObjectName(QString::fromUtf8("pb_First"));

        horizontalLayout_6->addWidget(pb_First);

        pb_Prev = new QPushButton(Lower);
        pb_Prev->setObjectName(QString::fromUtf8("pb_Prev"));

        horizontalLayout_6->addWidget(pb_Prev);

        label_Current = new QLabel(Lower);
        label_Current->setObjectName(QString::fromUtf8("label_Current"));
        sizePolicy7.setHeightForWidth(label_Current->sizePolicy().hasHeightForWidth());
        label_Current->setSizePolicy(sizePolicy7);
        label_Current->setMinimumSize(QSize(80, 0));
        label_Current->setAlignment(Qt::AlignCenter);

        horizontalLayout_6->addWidget(label_Current);

        pb_Next = new QPushButton(Lower);
        pb_Next->setObjectName(QString::fromUtf8("pb_Next"));

        horizontalLayout_6->addWidget(pb_Next);

        pb_Last = new QPushButton(Lower);
        pb_Last->setObjectName(QString::fromUtf8("pb_Last"));

        horizontalLayout_6->addWidget(pb_Last);

        comboBox_Page = new QComboBox(Lower);
        comboBox_Page->setObjectName(QString::fromUtf8("comboBox_Page"));
        sizePolicy7.setHeightForWidth(comboBox_Page->sizePolicy().hasHeightForWidth());
        comboBox_Page->setSizePolicy(sizePolicy7);
        comboBox_Page->setMinimumSize(QSize(80, 0));

        horizontalLayout_6->addWidget(comboBox_Page);

        pb_Goto = new QPushButton(Lower);
        pb_Goto->setObjectName(QString::fromUtf8("pb_Goto"));

        horizontalLayout_6->addWidget(pb_Goto);


        hLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_99 = new QHBoxLayout();
        horizontalLayout_99->setObjectName(QString::fromUtf8("horizontalLayout_99"));
        splitter1 = new QSplitter(Lower);
        splitter1->setObjectName(QString::fromUtf8("splitter1"));
        splitter1->setOrientation(Qt::Horizontal);
        left_statistic_widget = new QDockWidget(splitter1);
        left_statistic_widget->setObjectName(QString::fromUtf8("left_statistic_widget"));
        QSizePolicy sizePolicy11(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy11.setHorizontalStretch(0);
        sizePolicy11.setVerticalStretch(3);
        sizePolicy11.setHeightForWidth(left_statistic_widget->sizePolicy().hasHeightForWidth());
        left_statistic_widget->setSizePolicy(sizePolicy11);
        left_statistic_widget->setMinimumSize(QSize(95, 26));
        left_statistic_widget->setMaximumSize(QSize(200, 524287));
        splitter1->addWidget(left_statistic_widget);
        treetrace = new QTreeWidget(splitter1);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treetrace->setHeaderItem(__qtreewidgetitem);
        treetrace->setObjectName(QString::fromUtf8("treetrace"));
        QSizePolicy sizePolicy12(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy12.setHorizontalStretch(0);
        sizePolicy12.setVerticalStretch(5);
        sizePolicy12.setHeightForWidth(treetrace->sizePolicy().hasHeightForWidth());
        treetrace->setSizePolicy(sizePolicy12);
        splitter1->addWidget(treetrace);

        horizontalLayout_99->addWidget(splitter1);


        hLayout_2->addLayout(horizontalLayout_99);


        gridLayout_2->addWidget(Lower);


        verticalLayout_4->addWidget(widget);

        splitter->addWidget(bodyWidget);

        horizontalLayout->addWidget(splitter);


        verticalLayout->addWidget(mainWidgt);

        QtWidgetsApplication1Class->setCentralWidget(ZSTraceWindow);
        mainToolBar = new QToolBar(QtWidgetsApplication1Class);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        QtWidgetsApplication1Class->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtWidgetsApplication1Class);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        QtWidgetsApplication1Class->setStatusBar(statusBar);

        menubar->addAction(menu->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menu->addAction(actionstart);
        menu->addAction(actionpause);
        menu->addAction(actionstop);
        menu->addAction(actionmode);
        menu->addAction(actionreset);
        menu->addAction(actionreplay);
        menu->addAction(actionclear);
        menuHelp->addAction(actionHelp);
        menuHelp->addAction(actionAbout);

        retranslateUi(QtWidgetsApplication1Class);

        comboBox_Page->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(QtWidgetsApplication1Class);
    } // setupUi

    void retranslateUi(QMainWindow *QtWidgetsApplication1Class)
    {
        QtWidgetsApplication1Class->setWindowTitle(QCoreApplication::translate("QtWidgetsApplication1Class", "ZoneMaster Tracer", nullptr));
        actionstart->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "start", nullptr));
        actionpause->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "pause", nullptr));
        actionstop->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "stop", nullptr));
        actionConfig->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Config", nullptr));
        actionmode->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "display mode", nullptr));
        actionAbout->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "&About", nullptr));
        actionreset->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "resetAll", nullptr));
        actionreplay->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "replay", nullptr));
        actionHelp->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Help", nullptr));
        actionclear->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "clear", nullptr));
        menu->setTitle(QCoreApplication::translate("QtWidgetsApplication1Class", "Trace", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("QtWidgetsApplication1Class", "Help", nullptr));
        pb_prev_result->setText(QString());
        pb_next_result->setText(QString());
        pushButton_search->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Search", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("QtWidgetsApplication1Class", "CAN/LIN", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("QtWidgetsApplication1Class", "CAN", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("QtWidgetsApplication1Class", "LIN", nullptr));
        comboBox->setItemText(3, QCoreApplication::translate("QtWidgetsApplication1Class", "Some/IP", nullptr));
        comboBox->setItemText(4, QCoreApplication::translate("QtWidgetsApplication1Class", "ETH", nullptr));

        label_Filtered->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Normal", nullptr));
        pb_First->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "<<", nullptr));
        pb_Prev->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "<", nullptr));
        label_Current->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "1", nullptr));
        pb_Next->setText(QCoreApplication::translate("QtWidgetsApplication1Class", ">", nullptr));
        pb_Last->setText(QCoreApplication::translate("QtWidgetsApplication1Class", ">>", nullptr));
        comboBox_Page->setCurrentText(QString());
        pb_Goto->setText(QCoreApplication::translate("QtWidgetsApplication1Class", "Go", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtWidgetsApplication1Class: public Ui_QtWidgetsApplication1Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTWIDGETSAPPLICATION1_H
