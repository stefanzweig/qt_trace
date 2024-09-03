#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
#include "multiThread.h"
#include "columnfilter.h"


QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    init();
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{
    if (this->timer) {
        if (timer->isActive())
            timer->stop();
        delete timer;
        timer = nullptr;
    }
    if (this->timer_dustbin) {
        if (timer_dustbin->isActive())
            timer_dustbin->stop();
        delete timer_dustbin;
        timer_dustbin = nullptr;
    }
    if (mysub_can_frames != nullptr) {
        delete mysub_can_frames;
        mysub_can_frames = nullptr;
    }
    if (mysub_can_parser != nullptr) {
        delete mysub_can_parser;
        mysub_can_parser = nullptr;
    }
    if (calc_thread != nullptr) {
        calc_thread->stopFlag();

        calc_thread->quit();
        calc_thread->wait();

        delete calc_thread;
        calc_thread = nullptr;
    }

}

void QtWidgetsApplication1::init()
{
    ui.setupUi(this);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ui.widget->sizePolicy().hasHeightForWidth());
    ui.Lower->setSizePolicy(sizePolicy);
    //ui.toolbar_search->setVisible(true);
    
    // ���嶯�����˵��͹�����
    createActions();

    // ���嶨ʱ��
    timer = new QTimer();
    timer_dustbin = new QTimer();
    connect(timer, &QTimer::timeout, this, &QtWidgetsApplication1::updateState);
    connect(timer_dustbin, &QTimer::timeout, this, &QtWidgetsApplication1::dustbin);

    calc_thread = new multiThread();
    //connect(calc_thread, &multiThread::traceItemUpdate_internal, [=]() {qDebug() << "lambda"; });
    connect(calc_thread, &multiThread::popToRoot, this, &QtWidgetsApplication1::on_pop_to_root);


    // ���ò��� todo
    resetLayout();

    // ����״̬ todo
    _updateCurrentState();
    setupTreeTrace();

    filter = new columnFilterDialog(this);

    showMaximized();
}

void QtWidgetsApplication1::_updateCurrentState()
{
    QString runStatus = "";
    if (calc_thread->isSTOPPED())
        runStatus = "READY. ";
    if (!calc_thread->isSTOPPED())
        runStatus = "RUNNING. ";
    if (calc_thread->isPAUSED())
        runStatus = "PAUSED. ";
    QString status_string = runStatus + "CAN Frames: " + QString::number(this->calc_thread->full_count_canframes)
        + ". PDUs: " + QString::number(this->calc_thread->full_count_canparser);
    ui.statusBar->showMessage(status_string);
}

void QtWidgetsApplication1::updateState()
{
    while (!item_queue.isEmpty()) {
        QTreeWidgetItem* item = item_queue.dequeue();
        ui.treetrace->addTopLevelItem(item);
        trace_items.enqueue(item);
    }
    _updateCurrentState();
}

void QtWidgetsApplication1::createActions()
{
    //qDebug() << "createActions...";
    connect(ui.actionstart, &QAction::triggered, this, &QtWidgetsApplication1::startTrace);
    connect(ui.actionstop, &QAction::triggered, this, &QtWidgetsApplication1::stopTrace);
    connect(ui.actionpause, &QAction::triggered, this, &QtWidgetsApplication1::pauseTrace);
    connect(ui.pushButton_search, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonSearchClicked);
}

void QtWidgetsApplication1::resetLayout()
{
    //qDebug() << "resetLayout...";

    // will be removed later.
    //QAction* action = new QAction("Temp Action");
    //ui.menu->addAction(action);
    //connect(action, &QAction::triggered, this, &QtWidgetsApplication1::onActionTriggered);

    // icons
    //ui.menu->setIcon(QIcon(":/QtWidgetsApplication1/res/sqlitestudio.ico"));

    ui.actionpause->setIcon(QIcon(":/QtWidgetsApplication1/res/pauseTrace.png"));
    ui.actionstart->setIcon(QIcon(":/QtWidgetsApplication1/res/startTrace.png"));
    ui.actionstop->setIcon(QIcon(":/QtWidgetsApplication1/res/stopTrace.png"));
    ui.actionmode->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));

    // toolbar actions
    ui.toolbar->addAction(ui.actionstart);
    ui.toolbar->addAction(ui.actionstop);
    ui.toolbar->addAction(ui.actionpause);
    ui.toolbar->addAction(ui.actionmode);
    initialHeaders();

    ui.treetrace->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treetrace, &QTreeWidget::customContextMenuRequested, this, &QtWidgetsApplication1::prepareMenu);
    datachoice = ui.comboBox;
    connect(datachoice, &QComboBox::currentTextChanged, this, &QtWidgetsApplication1::ChangeHeader);
}

void QtWidgetsApplication1::prepareMenu(const QPoint& pos)
{
    QTreeWidget* tree = ui.treetrace;
    QTreeWidgetItem* nd = tree->itemAt(pos);
    QAction* newAct = new QAction(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"), tr("&Clean All"), this);
    newAct->setStatusTip(tr("Clean All"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newDev()));
    QMenu menu(this);
    menu.addAction(newAct);
    QPoint pt(pos);
    menu.exec(tree->mapToGlobal(pos));
}

void QtWidgetsApplication1::newDev()
{
    qDebug() << tr("Clean All");
    ui.treetrace->clear();
}

void QtWidgetsApplication1::onActionTriggered()
{
    qDebug() << tr("�˵����Ѵ�����");
}

void QtWidgetsApplication1::startTrace()
{
    qDebug() << "startTrace...";
    std::cout << "Starting subscriber." << std::endl;

    uint32_t samples = 100;

    if (mysub_can_frames == nullptr) {
        mysub_can_frames = new ZoneMasterCanMessageDataSubscriber();
        qRegisterMetaType <can_frame>("can_frame");
    }

    if (mysub_can_parser == nullptr) {
        mysub_can_parser = new ZoneMasterCanParserSubscriber();
        qRegisterMetaType <canframe>("canframe");
    }
    calc_thread->restartThread();
    calc_thread->setSubscriber(mysub_can_frames, samples, ui.treetrace);
    calc_thread->setCanParserSubscriber(mysub_can_parser, samples, ui.treetrace);
    calc_thread->start();
    timer->start(TIMER_HEARTBEAT);
    timer_dustbin->start(5000);
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
    calc_thread->stopThread();
    timer->stop();
    timer_dustbin->stop();
    mysub_can_frames = nullptr;
    mysub_can_parser = nullptr;
    updateState();
}

void QtWidgetsApplication1::pauseTrace()
{
    qDebug() << "pauseTrace...";
    calc_thread->pauseThread();
    timer->stop();
    timer_dustbin->stop();
    updateState();
}


void QtWidgetsApplication1::formatRow(int x)
{
    qDebug() << "formatRow..."<< x;
}

void QtWidgetsApplication1::formatRow_str(QString s)
{
    qDebug() << "formatRow..." << s;
}


void QtWidgetsApplication1::formatRow_canframe(can_frame cf)
{
    //qDebug() << "formatRow...cf->";
    //qDebug() << cf.ID << endl;
    //full_canframes.append(cf);
    //setupdatamodel();
}

void QtWidgetsApplication1::formatRow_canparser(unsigned long long i)
{
    //qDebug() << "canparser id ->" << i;
    //for (int k = 0; k < full_canframes.size(); k++) {
    //    if (full_canframes[k].Timestamp == i) {
    //        qDebug() << "MATCHED ->" << full_canframes[k].ID << "INDEX ->" << k;
    //    }
    //}
}

void QtWidgetsApplication1::internal_canparser(canframe frame)
{
    //qDebug() << "canframe ->" << QString::fromStdString(frame.name());
    //full_canparserdata.append(frame);
    //setupdatamodel_canparser();
}
void QtWidgetsApplication1::setupTreeTrace()
{
    QTreeWidget* t = ui.treetrace;
    t->setSelectionBehavior(QTreeView::SelectRows);
    t->setSelectionMode(QTreeView::SingleSelection);
    t->setFocusPolicy(Qt::NoFocus);

    t->header()->setHighlightSections(true);
    //t->header()->setDefaultAlignment(Qt::AlignCenter);
    //t->header()->setDefaultSectionSize(100);
    t->header()->setStretchLastSection(true);
    t->header()->setSortIndicator(0, Qt::AscendingOrder);
    t->setWindowTitle(QObject::tr("CAN Frames"));
    //t->show();
}

void QtWidgetsApplication1::initialHeaders()
{
    QTreeWidget* tree = ui.treetrace;
    QHeaderView* header = tree->header();
    header->setDefaultSectionSize(200);
    header->setSectionResizeMode(QHeaderView::Interactive);
    tree->setHeaderLabels(initialHeader);
    tree->setSortingEnabled(true);
    tree->setColumnWidth(0, 250);
    tree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    tree->invisibleRootItem()->setHidden(true);


    for (int i = 0; i < header->count(); i++) {
        QPushButton* button;
        button = new QPushButton(header);
        connect(button, &QPushButton::clicked, this, &QtWidgetsApplication1::headerButtonClicked);
        button->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));
        headerButtonList << button;
        int length = header->sectionPosition(i) + header->sectionSize(i);
        button->setGeometry(length - 20, 3, 17, 18);
    }
}

void QtWidgetsApplication1::headerButtonClicked()
{
    //qDebug() << "headerButtonClicked" << endl;

    QPushButton* filterButton = qobject_cast<QPushButton*>(sender());
    int columnButton = headerButtonList.indexOf(filterButton);
    QList<QString> filterConfig;

    int header_count = ui.treetrace->columnCount();
    filter->header_count = header_count;
    filter->columnButton = columnButton;
    if (filter->m_selectedStates.size() != 0) {}
    else {
        filter->m_selectedStates.reserve(header_count);
        int distinct_count = 100;
        for (int i = 0; i < header_count; ++i) {
            filter->m_selectedStates.append(QVector<bool>(distinct_count, false));
        }
    }
    for (int i = 0; i < ui.treetrace->invisibleRootItem()->childCount(); i++)
    {

        if (filterConfig.count(ui.treetrace->invisibleRootItem()->child(i)->text(columnButton)) > 0)
            continue;
        else {
            filterConfig << ui.treetrace->invisibleRootItem()->child(i)->text(columnButton);
        }
    }
    filterConfig.removeAll("");
    filterConfig.sort();

    filter->ui->tableWidget->setRowCount(filterConfig.count());
    filter->ui->tableWidget->setColumnCount(1);
    filter->ui->tableWidget->horizontalHeader()->setVisible(false);
    filter->ui->tableWidget->verticalHeader()->setVisible(false);
    filter->ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    for (int i = 0; i < filterConfig.count(); i++) {
        QTableWidgetItem* pItem = new QTableWidgetItem(filterConfig.at(i));
        pItem->setCheckState(Qt::Unchecked);
        filter->ui->tableWidget->setItem(i, 0, pItem);
        pItem->setCheckState(filter->m_selectedStates[columnButton].at(i) ? Qt::Checked : Qt::Unchecked);
    }
    filter->columnIndex = columnButton;
    connect(filter, &columnFilterDialog::filter_apply, this, &QtWidgetsApplication1::applyFilter);
    filter->exec();
}

void QtWidgetsApplication1::applyFilter(QList<QList<QString>> items, int count)
{
    bool judgement = false;
    int column_index = filter->columnIndex;
    QString colName = this->initialHeader[column_index];
    //qDebug() << "COLUMN -> " << colName;
    if (items.size() == 0) {
        colName = "";
        headerButtonList[column_index]->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));
    }
    else {
        headerButtonList[column_index]->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel_icon_selected.ico"));
        //for (int k = 0; k < items.size(); k++) {
        //    for (int i = 0; i < 1; i++) {
        //        QString s = items[k][i];
        //        qDebug() << "FILTER -> " << s;
        //    }
        //}
    }
    this->calc_thread->setFilterOption(colName, items);
    hide_filtered_items(column_index, items);
}


void QtWidgetsApplication1::on_pop_to_root(QTreeWidgetItem* item)
{
    //ui.treetrace->addTopLevelItem(item);
    item_queue.enqueue(item);
}

void QtWidgetsApplication1::ChangeHeader(const QString& text)
{
    //qDebug() << "ChangeHeader -> " << text;
    if (text == "Initial") {
        ui.treetrace->setColumnCount(initialHeader.count()); //set column number
        ui.treetrace->setHeaderLabels(initialHeader);        //set header labels
        CurrentHeader = initialHeader;                        //save current header for data match
    }
    if (text == "CAN") {
        ui.treetrace->setColumnCount(headers.count());
        ui.treetrace->setHeaderLabels(headers);
        CurrentHeader = headers;
    }
    if (text == "LIN") {
        ui.treetrace->setColumnCount(linHeader.count());
        ui.treetrace->setHeaderLabels(linHeader);
        CurrentHeader = linHeader;
        
    }
    if (text == "ETH") {
        ui.treetrace->setColumnCount(ethHeader.count());
        ui.treetrace->setHeaderLabels(ethHeader);
        CurrentHeader = ethHeader;
    }
}

void QtWidgetsApplication1::ButtonSearchClicked()
{
    calc_thread->pauseThread();
    // search...
}

void QtWidgetsApplication1::hide_filtered_items(int column_idx, QList<QList<QString>> items)
{
    //qDebug() << "COL -> " << column_idx << "ITEMS -> " << items.size();
    QStringList filtered_value = {};
    if (items.size() > 0) {
        for (int k = 0; k < items.size(); k++) {
            for (int i = 0; i < 1; i++) {
                filtered_value.append(items[k][i]);
            }
        }
    }
    QTreeWidgetItem* invisible_root_item = ui.treetrace->invisibleRootItem();
    int child_count = invisible_root_item->childCount();
    for (int i = 0; i < child_count; ++i) {
        QTreeWidgetItem* item = invisible_root_item->child(i);
        QString txt = item->text(column_idx);
        if (filtered_value.size() > 0) {
            if (filtered_value.contains(txt)) {
                item->setHidden(false);
            }
            else {
                item->setHidden(true);
            }
        }
        else
            item->setHidden(false);
    }
}

void QtWidgetsApplication1::dustbin()
{
    int ui_count = ui.treetrace->topLevelItemCount();
    int counter = ui_count - MAX_ITEM_COUNT;
    if (ui_count > MAX_ITEM_COUNT) {
        qDebug() << "before dustbin -> " << counter;
        while (counter>0 && trace_items.size()>0) {
            QTreeWidgetItem* item = trace_items.dequeue();
            int index = ui.treetrace->indexOfTopLevelItem(item);
            if (index != -1) {
                ui.treetrace->takeTopLevelItem(index);
                delete item;
            }
            counter--;
        }
    }
    ui_count = ui.treetrace->topLevelItemCount();
    qDebug() << "Tree COUNTER -> " << ui_count;
    //ui.treetrace->scrollToBottom();
}