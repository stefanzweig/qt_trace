/********************************************************************************
** Form generated from reading UI file 'configdialogtMHhbY.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef CONFIGDIALOGTMHHBY_H
#define CONFIGDIALOGTMHHBY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ConfigDialog
{
public:
    QGridLayout* gridLayout_5;
    QWidget* mainWidget;
    QVBoxLayout* verticalLayout_5;
    QSplitter* splitter;
    QWidget* categoriesWidget;
    QVBoxLayout* verticalLayout;
    QLineEdit* categoriesFilter;
    QTreeWidget* categoriesTree;
    QStackedWidget* stackedWidget;
    QWidget* generalPage;
    QVBoxLayout* verticalLayout_3;
    QGroupBox* schemaEditingGroup;
    QGridLayout* gridLayout_12;
    QSpinBox* ddlHistorySizeSpin;
    QLabel* ddlHistorySizeLabel;
    QCheckBox* dontShowDdlPreview;
    QGroupBox* sqlQueriesGroup;
    QGridLayout* gridLayout_4;
    QSpinBox* queryHistorySizeSpin;
    QLabel* bindParamLimitLabel;
    QSpinBox* bindParamLimitSpin;
    QCheckBox* execQueryUnderCursorCheck;
    QLabel* queryHistorySizeLabel;
    QCheckBox* wrapLinesInSqlEditorCheck;
    QCheckBox* highlightCurrentQueryCheck;
    QGroupBox* updatesGroup;
    QVBoxLayout* verticalLayout_29;
    QCheckBox* checkForUpdatesCheck;
    QGroupBox* sessionGroup;
    QGridLayout* gridLayout_11;
    QCheckBox* sessionCheck;
    QCheckBox* multipleSessionsCheck;
    QGroupBox* statusFieldGroup_2;
    QVBoxLayout* verticalLayout_35;
    QCheckBox* checkBox_2;
    QSpacerItem* verticalSpacer;
    QDialogButtonBox* buttonBox;

    void setupUi(QDialog* ConfigDialog)
    {
        if (ConfigDialog->objectName().isEmpty())
            ConfigDialog->setObjectName(QString::fromUtf8("ConfigDialog"));
        ConfigDialog->resize(866, 580);
        gridLayout_5 = new QGridLayout(ConfigDialog);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        mainWidget = new QWidget(ConfigDialog);
        mainWidget->setObjectName(QString::fromUtf8("mainWidget"));
        mainWidget->setContextMenuPolicy(Qt::PreventContextMenu);
        verticalLayout_5 = new QVBoxLayout(mainWidget);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        splitter = new QSplitter(mainWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        categoriesWidget = new QWidget(splitter);
        categoriesWidget->setObjectName(QString::fromUtf8("categoriesWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(categoriesWidget->sizePolicy().hasHeightForWidth());
        categoriesWidget->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(categoriesWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        categoriesFilter = new QLineEdit(categoriesWidget);
        categoriesFilter->setObjectName(QString::fromUtf8("categoriesFilter"));

        verticalLayout->addWidget(categoriesFilter);

        categoriesTree = new QTreeWidget(categoriesWidget);
        QTreeWidgetItem* __qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        categoriesTree->setHeaderItem(__qtreewidgetitem);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/img/config_general.png"), QSize(), QIcon::Normal, QIcon::Off);
        QTreeWidgetItem* __qtreewidgetitem1 = new QTreeWidgetItem(categoriesTree);
        __qtreewidgetitem1->setIcon(0, icon);
#if QT_CONFIG(statustip)
        __qtreewidgetitem1->setStatusTip(0, QString::fromUtf8("generalPage"));
#endif // QT_CONFIG(statustip)
        categoriesTree->setObjectName(QString::fromUtf8("categoriesTree"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(categoriesTree->sizePolicy().hasHeightForWidth());
        categoriesTree->setSizePolicy(sizePolicy1);
        categoriesTree->setMinimumSize(QSize(150, 0));
        categoriesTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        categoriesTree->header()->setVisible(false);

        verticalLayout->addWidget(categoriesTree);

        splitter->addWidget(categoriesWidget);
        stackedWidget = new QStackedWidget(splitter);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(5);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy2);
        generalPage = new QWidget();
        generalPage->setObjectName(QString::fromUtf8("generalPage"));
        verticalLayout_3 = new QVBoxLayout(generalPage);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        schemaEditingGroup = new QGroupBox(generalPage);
        schemaEditingGroup->setObjectName(QString::fromUtf8("schemaEditingGroup"));
        gridLayout_12 = new QGridLayout(schemaEditingGroup);
        gridLayout_12->setObjectName(QString::fromUtf8("gridLayout_12"));
        ddlHistorySizeSpin = new QSpinBox(schemaEditingGroup);
        ddlHistorySizeSpin->setObjectName(QString::fromUtf8("ddlHistorySizeSpin"));
        ddlHistorySizeSpin->setMaximumSize(QSize(150, 16777215));
        ddlHistorySizeSpin->setMaximum(9999999);
        ddlHistorySizeSpin->setProperty("cfg", QVariant(QString::fromUtf8("General.DdlHistorySize")));

        gridLayout_12->addWidget(ddlHistorySizeSpin, 1, 1, 1, 1);

        ddlHistorySizeLabel = new QLabel(schemaEditingGroup);
        ddlHistorySizeLabel->setObjectName(QString::fromUtf8("ddlHistorySizeLabel"));

        gridLayout_12->addWidget(ddlHistorySizeLabel, 1, 0, 1, 1);

        dontShowDdlPreview = new QCheckBox(schemaEditingGroup);
        dontShowDdlPreview->setObjectName(QString::fromUtf8("dontShowDdlPreview"));
        dontShowDdlPreview->setProperty("cfg", QVariant(QString::fromUtf8("General.DontShowDdlPreview")));

        gridLayout_12->addWidget(dontShowDdlPreview, 0, 0, 1, 2);


        verticalLayout_3->addWidget(schemaEditingGroup);

        sqlQueriesGroup = new QGroupBox(generalPage);
        sqlQueriesGroup->setObjectName(QString::fromUtf8("sqlQueriesGroup"));
        gridLayout_4 = new QGridLayout(sqlQueriesGroup);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        queryHistorySizeSpin = new QSpinBox(sqlQueriesGroup);
        queryHistorySizeSpin->setObjectName(QString::fromUtf8("queryHistorySizeSpin"));
        queryHistorySizeSpin->setMaximumSize(QSize(150, 16777215));
        queryHistorySizeSpin->setMaximum(999999);
        queryHistorySizeSpin->setProperty("cfg", QVariant(QString::fromUtf8("General.SqlHistorySize")));

        gridLayout_4->addWidget(queryHistorySizeSpin, 1, 1, 1, 1);

        bindParamLimitLabel = new QLabel(sqlQueriesGroup);
        bindParamLimitLabel->setObjectName(QString::fromUtf8("bindParamLimitLabel"));

        gridLayout_4->addWidget(bindParamLimitLabel, 2, 0, 1, 1);

        bindParamLimitSpin = new QSpinBox(sqlQueriesGroup);
        bindParamLimitSpin->setObjectName(QString::fromUtf8("bindParamLimitSpin"));
        bindParamLimitSpin->setMaximum(999999);
        bindParamLimitSpin->setProperty("cfg", QVariant(QString::fromUtf8("General.BindParamsCacheSize")));

        gridLayout_4->addWidget(bindParamLimitSpin, 2, 1, 1, 1);

        execQueryUnderCursorCheck = new QCheckBox(sqlQueriesGroup);
        execQueryUnderCursorCheck->setObjectName(QString::fromUtf8("execQueryUnderCursorCheck"));
        execQueryUnderCursorCheck->setProperty("cfg", QVariant(QString::fromUtf8("General.ExecuteCurrentQueryOnly")));

        gridLayout_4->addWidget(execQueryUnderCursorCheck, 3, 0, 1, 1);

        queryHistorySizeLabel = new QLabel(sqlQueriesGroup);
        queryHistorySizeLabel->setObjectName(QString::fromUtf8("queryHistorySizeLabel"));

        gridLayout_4->addWidget(queryHistorySizeLabel, 1, 0, 1, 1);

        wrapLinesInSqlEditorCheck = new QCheckBox(sqlQueriesGroup);
        wrapLinesInSqlEditorCheck->setObjectName(QString::fromUtf8("wrapLinesInSqlEditorCheck"));
        wrapLinesInSqlEditorCheck->setProperty("cfg", QVariant(QString::fromUtf8("General.SqlEditorWrapWords")));

        gridLayout_4->addWidget(wrapLinesInSqlEditorCheck, 5, 0, 1, 1);

        highlightCurrentQueryCheck = new QCheckBox(sqlQueriesGroup);
        highlightCurrentQueryCheck->setObjectName(QString::fromUtf8("highlightCurrentQueryCheck"));
        highlightCurrentQueryCheck->setProperty("cfg", QVariant(QString::fromUtf8("General.SqlEditorCurrQueryHighlight")));

        gridLayout_4->addWidget(highlightCurrentQueryCheck, 4, 0, 1, 1);


        verticalLayout_3->addWidget(sqlQueriesGroup);

        updatesGroup = new QGroupBox(generalPage);
        updatesGroup->setObjectName(QString::fromUtf8("updatesGroup"));
        verticalLayout_29 = new QVBoxLayout(updatesGroup);
        verticalLayout_29->setObjectName(QString::fromUtf8("verticalLayout_29"));
        checkForUpdatesCheck = new QCheckBox(updatesGroup);
        checkForUpdatesCheck->setObjectName(QString::fromUtf8("checkForUpdatesCheck"));
        checkForUpdatesCheck->setProperty("cfg", QVariant(QString::fromUtf8("General.CheckUpdatesOnStartup")));

        verticalLayout_29->addWidget(checkForUpdatesCheck);


        verticalLayout_3->addWidget(updatesGroup);

        sessionGroup = new QGroupBox(generalPage);
        sessionGroup->setObjectName(QString::fromUtf8("sessionGroup"));
        gridLayout_11 = new QGridLayout(sessionGroup);
        gridLayout_11->setObjectName(QString::fromUtf8("gridLayout_11"));
        sessionCheck = new QCheckBox(sessionGroup);
        sessionCheck->setObjectName(QString::fromUtf8("sessionCheck"));
        sessionCheck->setProperty("cfg", QVariant(QString::fromUtf8("General.RestoreSession")));

        gridLayout_11->addWidget(sessionCheck, 0, 0, 1, 1);

        multipleSessionsCheck = new QCheckBox(sessionGroup);
        multipleSessionsCheck->setObjectName(QString::fromUtf8("multipleSessionsCheck"));
        multipleSessionsCheck->setProperty("cfg", QVariant(QString::fromUtf8("General.AllowMultipleSessions")));

        gridLayout_11->addWidget(multipleSessionsCheck, 1, 0, 1, 1);


        verticalLayout_3->addWidget(sessionGroup);

        statusFieldGroup_2 = new QGroupBox(generalPage);
        statusFieldGroup_2->setObjectName(QString::fromUtf8("statusFieldGroup_2"));
        verticalLayout_35 = new QVBoxLayout(statusFieldGroup_2);
        verticalLayout_35->setObjectName(QString::fromUtf8("verticalLayout_35"));
        checkBox_2 = new QCheckBox(statusFieldGroup_2);
        checkBox_2->setObjectName(QString::fromUtf8("checkBox_2"));
        checkBox_2->setProperty("cfg", QVariant(QString::fromUtf8("General.AutoOpenStatusField")));

        verticalLayout_35->addWidget(checkBox_2);


        verticalLayout_3->addWidget(statusFieldGroup_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        stackedWidget->addWidget(generalPage);
        splitter->addWidget(stackedWidget);

        verticalLayout_5->addWidget(splitter);


        gridLayout_5->addWidget(mainWidget, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(ConfigDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        gridLayout_5->addWidget(buttonBox, 4, 0, 1, 1);

        QWidget::setTabOrder(categoriesFilter, categoriesTree);
        QWidget::setTabOrder(categoriesTree, buttonBox);
        QWidget::setTabOrder(buttonBox, ddlHistorySizeSpin);
        QWidget::setTabOrder(ddlHistorySizeSpin, dontShowDdlPreview);
        QWidget::setTabOrder(dontShowDdlPreview, queryHistorySizeSpin);
        QWidget::setTabOrder(queryHistorySizeSpin, execQueryUnderCursorCheck);

        retranslateUi(ConfigDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ConfigDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ConfigDialog, SLOT(reject()));

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ConfigDialog);
    } // setupUi

    void retranslateUi(QDialog* ConfigDialog)
    {
        ConfigDialog->setWindowTitle(QCoreApplication::translate("ConfigDialog", "Configuration", nullptr));
        categoriesFilter->setPlaceholderText(QCoreApplication::translate("ConfigDialog", "Search", nullptr));

        const bool __sortingEnabled = categoriesTree->isSortingEnabled();
        categoriesTree->setSortingEnabled(false);
        QTreeWidgetItem* ___qtreewidgetitem = categoriesTree->topLevelItem(0);
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("ConfigDialog", "General", nullptr));
        categoriesTree->setSortingEnabled(__sortingEnabled);

        schemaEditingGroup->setTitle(QCoreApplication::translate("ConfigDialog", "Schema editing", nullptr));
#if QT_CONFIG(tooltip)
        ddlHistorySizeSpin->setToolTip(QCoreApplication::translate("ConfigDialog", "Number of DDL changes kept in history.", nullptr));
#endif // QT_CONFIG(tooltip)
        ddlHistorySizeLabel->setText(QCoreApplication::translate("ConfigDialog", "DDL history size:", nullptr));
        dontShowDdlPreview->setText(QCoreApplication::translate("ConfigDialog", "Don't show DDL preview dialog when committing schema changes", nullptr));
        sqlQueriesGroup->setTitle(QCoreApplication::translate("ConfigDialog", "SQL queries", nullptr));
#if QT_CONFIG(tooltip)
        queryHistorySizeSpin->setToolTip(QCoreApplication::translate("ConfigDialog", "Number of queries kept in the history.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        bindParamLimitLabel->setToolTip(QCoreApplication::translate("ConfigDialog", "<p>Maximum number of query parameters (:param, @param, $param, ?) stored in history. When you re-use parameter with the same name/position, SQLiteStudio will pre-initialize it with most recent memorized value (you will still be able to change it). Value of 1000 should be sufficient.</p>", nullptr));
#endif // QT_CONFIG(tooltip)
        bindParamLimitLabel->setText(QCoreApplication::translate("ConfigDialog", "Number of memorized query parameters", nullptr));
#if QT_CONFIG(tooltip)
        bindParamLimitSpin->setToolTip(QCoreApplication::translate("ConfigDialog", "<p>Maximum number of query parameters (:param, @param, $param, ?) stored in history. When you re-use parameter with the same name/position, SQLiteStudio will pre-initialize it with most recent memorized value (you will still be able to change it). Value of 1000 should be sufficient.</p>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        execQueryUnderCursorCheck->setToolTip(QCoreApplication::translate("ConfigDialog", "<html><head/><body><p>If there is more than one query in the SQL editor window, then (if this option is enabled) only a single query will be executed - the one under the keyboard insertion cursor. Otherwise all queries will be executed. You can always limit queries to be executed by selecting those queries before calling to execute. You can also use dedicated shortcuts for executing in one mode or the other (currently configured to %1 for single query execution and %2 for all queries execution).</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        execQueryUnderCursorCheck->setText(QCoreApplication::translate("ConfigDialog", "Execute only the query under the cursor", nullptr));
#if QT_CONFIG(tooltip)
        queryHistorySizeLabel->setToolTip(QCoreApplication::translate("ConfigDialog", "Number of queries kept in the history.", nullptr));
#endif // QT_CONFIG(tooltip)
        queryHistorySizeLabel->setText(QCoreApplication::translate("ConfigDialog", "History size:", nullptr));
#if QT_CONFIG(tooltip)
        wrapLinesInSqlEditorCheck->setToolTip(QCoreApplication::translate("ConfigDialog", "<html><head/><body><p>If enabled, lines longer than the editor width will be wrapped, so horizontal scrolling will not be needed.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        wrapLinesInSqlEditorCheck->setText(QCoreApplication::translate("ConfigDialog", "Wrap lines in SQL editor", nullptr));
#if QT_CONFIG(tooltip)
        highlightCurrentQueryCheck->setToolTip(QCoreApplication::translate("ConfigDialog", "<html><head/><body><p>Highlights entire query that is currently under the insertion cursor. It's the same query that will be executed when you hit &quot;Execute query&quot; hotkey or button (unless configured otherwise).</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        highlightCurrentQueryCheck->setText(QCoreApplication::translate("ConfigDialog", "Highlight current query", nullptr));
        updatesGroup->setTitle(QCoreApplication::translate("ConfigDialog", "Updates", nullptr));
        checkForUpdatesCheck->setText(QCoreApplication::translate("ConfigDialog", "Automatically check for updates at startup", nullptr));
        sessionGroup->setTitle(QCoreApplication::translate("ConfigDialog", "Session", nullptr));
        sessionCheck->setText(QCoreApplication::translate("ConfigDialog", "Restore last session (active MDI windows) after startup", nullptr));
        multipleSessionsCheck->setText(QCoreApplication::translate("ConfigDialog", "Allow multiple instances of the application at the same time", nullptr));
        statusFieldGroup_2->setTitle(QCoreApplication::translate("ConfigDialog", "Status Field", nullptr));
#if QT_CONFIG(tooltip)
        checkBox_2->setToolTip(QCoreApplication::translate("ConfigDialog", "<p>When user manually closes the Status panel, this option makes sure that if any new message is printed in the Status panel it will be reopened. If it's disabled, then Status panel can only be open manually by the user from the \"View\" menu.</p>", nullptr));
#endif // QT_CONFIG(tooltip)
        checkBox_2->setText(QCoreApplication::translate("ConfigDialog", "Always open Status panel when new message is printed", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ConfigDialog : public Ui_ConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // CONFIGDIALOGTMHHBY_H
