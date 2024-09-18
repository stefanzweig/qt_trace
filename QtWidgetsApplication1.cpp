#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
#include "multiThread.h"
#include "columnfilter.h"
#include <QSettings>
#include <QScrollBar>
#include <QLabel>
#include <QMessageBox>


QSize getItemSize(QTreeWidgetItem* item, int column, const QFont& font) {
    QFontMetrics fontMetrics(font);
    QString text = item->text(column);
    QSize textSize = fontMetrics.size(Qt::TextSingleLine, text);
    return QSize(textSize.width() + 20, textSize.height());
}

QTreeWidgetItem* deepCopyItem(QTreeWidgetItem* item) {
    QTreeWidgetItem* newItem = new QTreeWidgetItem(*item);

    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        newItem->addChild(deepCopyItem(child)); // 递归复制子项
    }
    return newItem;
}

QList<QTreeWidgetItem*> getVisibleItems(QTreeWidget* tree) {
    QList<QTreeWidgetItem*> visibleItems;
    QRect viewportRect = tree->viewport()->rect();  // 获取可视区域

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = tree->topLevelItem(i);
        QRect itemRect = tree->visualItemRect(item);  // 获取项的矩形区域

        if (viewportRect.intersects(itemRect)) {
            visibleItems.append(item);
        }
        for (int j = 0; j < item->childCount(); ++j) {
            QTreeWidgetItem* child = item->child(j);
            QRect childRect = tree->visualItemRect(child);
            if (viewportRect.intersects(childRect)) {
                visibleItems.append(child);
            }
        }
    }
    return visibleItems;
}

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

    get_default_configurations();
    createActions();

    timer = new QTimer();
    timer_dustbin = new QTimer();
    connect(timer, &QTimer::timeout, this, &QtWidgetsApplication1::updateState);
    connect(timer_dustbin, &QTimer::timeout, this, &QtWidgetsApplication1::dustbin);

    calc_thread = new multiThread();
    calc_thread->monitor_modules = monitor_modules;
    qDebug() << "Copied List 1:" << calc_thread->monitor_modules;

    connect(calc_thread, &multiThread::popToRoot, this, &QtWidgetsApplication1::on_pop_to_root);

    resetLayout();
    resetStatusBar();
    _updateCurrentState();

    setupTreeTrace();
    filter = new columnFilterDialog(this);
    ui.treetrace->setAttribute(Qt::WA_OpaquePaintEvent);
    ui.treetrace->setAttribute(Qt::WA_NoSystemBackground);
    showMaximized();
}

void QtWidgetsApplication1::get_default_configurations()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    int kv = settings.value("app/DDS_DomainId", 90).toInt();
    qDebug() << "Setting -> " << kv;
    dds_domainid = kv;
    kv = settings.value("app/page_capacity", 300).toInt();
    page_capacity = kv;
    kv = settings.value("app/count_per_page", 300).toInt();
    count_per_page = kv;
    QList<QVariant> modules = settings.value("app/modules").toList();
    qDebug() << "Modules -> " << modules;
    for (QVariant m : modules) {
        monitor_modules.append(m.toString());
    }
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

    int ncount = ui.treetrace->topLevelItemCount();
    QString strRight = QString("Page Capacity: %1").arg(page_capacity);
    QString strLeft = QString("Current: %1").arg(ncount);
    rightLabel->setText(strRight);
    leftLabel->setText(strLeft);
}

void QtWidgetsApplication1::updateState()
{
    update_tracewindow();
    //ui.treetrace->scrollToBottom();
    _updateCurrentState();
}

void QtWidgetsApplication1::createActions()
{
    //qDebug() << "createActions...";
    connect(ui.actionstart, &QAction::triggered, this, &QtWidgetsApplication1::startTrace);
    connect(ui.actionstop, &QAction::triggered, this, &QtWidgetsApplication1::stopTrace);
    connect(ui.actionpause, &QAction::triggered, this, &QtWidgetsApplication1::pauseTrace);
    connect(ui.actionmode, &QAction::triggered, this, &QtWidgetsApplication1::display_mode_switch);
    connect(ui.pushButton_search, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonSearchClicked);
    connect(ui.actionAbout, &QAction::triggered, this, &QtWidgetsApplication1::about);
    connect(ui.actionreset, &QAction::triggered, this, &QtWidgetsApplication1::reset_all_filters);
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

    ui.actionpause->setIcon(QIcon(":/QtWidgetsApplication1/res/pause.svg"));
    ui.actionstart->setIcon(QIcon(":/QtWidgetsApplication1/res/play.svg"));
    ui.actionstop->setIcon(QIcon(":/QtWidgetsApplication1/res/maximize.svg"));
    ui.actionreset->setIcon(QIcon(":/QtWidgetsApplication1/res/tick.svg"));

    // toolbar actions
    ui.toolbar->addAction(ui.actionstart);
    ui.toolbar->addAction(ui.actionstop);
    ui.toolbar->addAction(ui.actionpause);
    ui.toolbar->addAction(ui.actionmode);
    ui.toolbar->addAction(ui.actionreset);
    initialHeaders();

    //ui.treetrace->setContextMenuPolicy(Qt::CustomContextMenu);
    // connect(ui.treetrace, &QTreeWidget::customContextMenuRequested, this, &QtWidgetsApplication1::prepareMenu);
    
    connect(ui.treetrace->header(), &QHeaderView::sectionResized, this, &QtWidgetsApplication1::on_header_section_resized);
    connect(ui.treetrace->horizontalScrollBar(), &QScrollBar::valueChanged, this, &QtWidgetsApplication1::on_horizontal_scroll);

    datachoice = ui.comboBox;
    connect(datachoice, &QComboBox::currentTextChanged, this, &QtWidgetsApplication1::ChangeHeader);
    connect(ui.treetrace->verticalScrollBar(), &QScrollBar::valueChanged, this, &QtWidgetsApplication1::trace_scroll_changed);
    updateToolbar();
}

void QtWidgetsApplication1::resetStatusBar()
{
    leftLabel = new QLabel("Left Information", this);
    leftWidget = new QWidget(this);
    leftLayout = new QHBoxLayout(leftWidget);
    leftLayout->addWidget(leftLabel);
    leftLayout->addStretch();

    rightLabel = new QLabel("Right Information", this);
    rightWidget = new QWidget(this);
    rightLayout = new QHBoxLayout(rightWidget);
    rightLayout->addStretch();
    rightLayout->addWidget(rightLabel);

    ui.statusBar->addPermanentWidget(leftWidget);
    ui.statusBar->addPermanentWidget(rightWidget);
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
    qDebug() << tr("菜单项已触发！");
}

bool QtWidgetsApplication1::new_session()
{
    int ncount = ui.treetrace->topLevelItemCount();
    if (ncount) {
        leftLabel->setText(QString("Previous Count: %1").arg(ncount));
    }
    if (true) {
        calc_thread->full_count_canframes = 0;
        calc_thread->full_count_canparser = 0;
        full_queue.clear();
    }
    return true;
}

void QtWidgetsApplication1::startTrace()
{
    qDebug() << "startTrace...";

    if (!new_session())
        return;

    ui.treetrace->clear();
    resumeTrace();
}
void QtWidgetsApplication1::resumeTrace()
{
    uint32_t samples = 100;
    if (mysub_can_frames == nullptr) {
        mysub_can_frames = new ZoneMasterCanMessageDataSubscriber(dds_domainid);
        qRegisterMetaType <can_frame>("can_frame");
    }

    if (mysub_can_parser == nullptr) {
        mysub_can_parser = new ZoneMasterCanParserSubscriber(dds_domainid);
        qRegisterMetaType <canframe>("canframe");
    }
    calc_thread->restartThread();
    calc_thread->setSubscriber(mysub_can_frames, samples, ui.treetrace);
    calc_thread->setCanParserSubscriber(mysub_can_parser, samples, ui.treetrace);
    calc_thread->start();
    timer->start(TIMER_HEARTBEAT);
    frozen = false;
    updateToolbar();
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
    calc_thread->stopThread();
    timer->stop();
    mysub_can_frames = nullptr;
    mysub_can_parser = nullptr;
    initial_trace = true;
    updateToolbar();
    _updateCurrentState();
    qDebug() << "stopTrace...DONE";
    frozen = true;
    freeze_treetrace_items(count_per_page);

}

void QtWidgetsApplication1::pauseTrace()
{
    if (calc_thread->isPAUSED()) {
        frozen = false;
        ui.treetrace->clear();
        initial_trace = true;
        resumeTrace();
        return;
    }
    qDebug() << "pauseTrace...";
    calc_thread->pauseThread();
    timer->stop();
    //timer_dustbin->stop();
    //updateState();
    freeze_treetrace_items(count_per_page);
    updateToolbar();
    _updateCurrentState();
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
}

void QtWidgetsApplication1::formatRow_canparser(unsigned long long i)
{
}

void QtWidgetsApplication1::internal_canparser(canframe frame)
{
}

void QtWidgetsApplication1::setupTreeTrace()
{
    QTreeWidget* t = ui.treetrace;
    t->setSelectionBehavior(QTreeView::SelectRows);
    t->setSelectionMode(QTreeView::SingleSelection);
    t->setFocusPolicy(Qt::NoFocus);
    t->header()->setHighlightSections(true);
    t->header()->setStretchLastSection(true);
    t->header()->setSortIndicator(0, Qt::AscendingOrder);
    //t->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);


    // setting the widths of the columns

    /*
    int width = t->viewport()->width();
    int col_count = t->columnCount();
    for (int i = 0; i < col_count; i++) {
        t->resizeColumnToContents(i);
    }
    */
    
    // end of setting

    t->setWindowTitle(QObject::tr("CAN Frames"));
    QFont font("SimSun", 8);
    t->setFont(font);
}

void QtWidgetsApplication1::initialHeaders()
{
    QTreeWidget* tree = ui.treetrace;
    QHeaderView* header = tree->header();
    header->setDefaultSectionSize(150);
    header->setSectionResizeMode(QHeaderView::Interactive);
    tree->setHeaderLabels(initialHeader);
    tree->setSortingEnabled(true);
    tree->setColumnWidth(0, 150);
    tree->header()->setStretchLastSection(true);
    tree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    tree->invisibleRootItem()->setHidden(true);

    for (int i = 0; i < header->count(); i++) {
        QPushButton* button;
        button = new QPushButton(header);
        connect(button, &QPushButton::clicked, this, &QtWidgetsApplication1::headerButtonClicked);
        button->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));
        headerButtonList << button;
    }
    
    adjust_filter_buttons();
}

void QtWidgetsApplication1::headerButtonClicked()
{
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

    filter->columnIndex = columnButton;
    filter->new_checks;
    int column_index = filter->columnIndex;
    QString colName = initialHeader[column_index];
    filter->colName = colName;
    QList<QVariant> vlist = filter->new_checks.value(colName);
    qDebug() << vlist.length();

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
        QString text = pItem->text();
        if (vlist.contains(text)) {
            pItem->setCheckState(Qt::Checked);
        }
        else {
            pItem->setCheckState(Qt::Unchecked);
            
        }
        //pItem->setCheckState(filter->m_selectedStates[columnButton].at(i) ? Qt::Checked : Qt::Unchecked);
    }
    
    if (!filter_pop) {
        connect(filter, &columnFilterDialog::filter_apply, this, &QtWidgetsApplication1::applyFilter);
        filter_pop = true;
    }
    filter->exec();
}

void QtWidgetsApplication1::applyFilter(QList<QList<QString>> items, int count)
{
    int column_index = filter->columnIndex;
    QString colName = initialHeader[column_index];
    if (items.size() == 0) {
        headerButtonList[column_index]->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));
    }
    else {
        headerButtonList[column_index]->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel_icon_selected.ico"));
    }
    QList<QVariant> single_config;
    single_config.append(column_index);
    for (int k = 0; k < items.size(); k++) {
        for (int i = 0; i < 1; i++) {
            QString s = items[k][i];
            //qDebug() << "FILTER -> " << s;
            single_config.append(s);
        }
    }
    if (single_config.size() == 1) { 
        new_filters.remove(colName); 
        return; 
    }
    new_filters.insert(colName, single_config);

    //this->calc_thread->setFilterOption(colName, items);
    //hide_filtered_items(column_index, items);
    //if (single_config.size() == 1) { new_filters.erase(); }
}


void QtWidgetsApplication1::on_pop_to_root(QTreeWidgetItem* item)
{
    full_queue.enqueue(item);
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
    return;
}

void QtWidgetsApplication1::updateToolbar()
{
    if (display_mode) {
        ui.actionmode->setIcon(QIcon(":/QtWidgetsApplication1/res/process.svg"));
        ui.actionmode->setToolTip("Updating...");
    }
    else
    {
        ui.actionmode->setIcon(QIcon(":/QtWidgetsApplication1/res/plus.svg"));
        ui.actionmode->setToolTip("Appending...");
    }
    if (calc_thread->isSTOPPED()) {
        ui.actionstart->setEnabled(true);
        ui.actionstop->setEnabled(true);
    }
    if (!calc_thread->isSTOPPED()) {
        ui.actionstart->setEnabled(false);
        ui.actionstop->setEnabled(true);
    }
    if (calc_thread->isPAUSED()) {
        ui.actionstart->setEnabled(false);
        ui.actionstop->setEnabled(true);
        ui.actionpause->setIcon(QIcon(":/QtWidgetsApplication1/res/pause-a.svg"));
    }
    else {
        ui.actionpause->setIcon(QIcon(":/QtWidgetsApplication1/res/pause.svg"));
    }
}

void QtWidgetsApplication1::display_mode_switch()
{
    if (display_mode)
        display_mode = 0;
    else
        display_mode = 1;
    updateToolbar();
    update_tracewindow();
}
void QtWidgetsApplication1::get_refreshed_items()
{
    QList<QTreeWidgetItem*> visibleItems = 
        getVisibleItems(ui.treetrace);
    qDebug() << "Visual Items ->" << visibleItems.size();
}

void QtWidgetsApplication1::update_tracewindow()
{
    int queue_size = full_queue.size()-1;
    if (queue_size)
        qDebug() << "QUEUE_SIZE -> " << queue_size << "LAST ITEM COLUMN COUNTS" << full_queue[queue_size - 1]->columnCount();

     QSize itemSize;
     QTreeWidget* tree = ui.treetrace;
     QTreeWidgetItem* invisible_root_item = tree->invisibleRootItem();

     itemSize = getItemSize(invisible_root_item, 0, ui.treetrace->font());
     int v_height = ui.treetrace->viewport()->height();
     int capacity = visible_height / itemSize.height() - 1;
    
     visible_height = v_height;
     item_height = itemSize.height();
     item_width = itemSize.width();
     //qDebug() << "CAPACITY -> " << capacity;

     int tree_count = tree->topLevelItemCount();
     if (tree_count >= capacity) {
         refresh_full_tree(capacity);
     }
     else if (tree_count) {
         fill_partial_tree(capacity);
     }
     else if (!tree_count) {
         fill_empty_tree(capacity);
     }
     ui.treetrace->scrollToBottom();
}

void QtWidgetsApplication1::refresh_full_tree(int capacity)
{
    qDebug() << "FULL ->" << page_capacity;
    QTreeWidget* tree = ui.treetrace;
    int tree_count = tree->topLevelItemCount();
    if (tree_count < page_capacity) {
        fill_up_to_count(page_capacity);
        return;
    }
    else {
        draw_trace_window(capacity);
        return;
    }
}

void QtWidgetsApplication1::draw_trace_window(int capacity)
{
    qDebug() << "DRAW TRACE WINDOW ->" << capacity;

    int queue_size = full_queue.size()-1;
    if (!queue_size) return;

    QTreeWidget* tree = ui.treetrace;
    QTreeWidgetItem* it = nullptr;
    int changes = std::min(queue_size, capacity);
    if (changes) {
        qDebug() << "DRAW CHANGES ->" << changes;
        int count = tree->topLevelItemCount();
        for (int i = 1; i <= changes; i++) {
            it = full_queue.at(queue_size - i);
            if (filter_pass_item(it)) {
                if (count > 0) 
                {
                    QTreeWidgetItem* lastItem = tree->topLevelItem(count-i);
                    for (int k = 0; k < it->columnCount(); k++) {
                        if (lastItem->text(k) != it->text(k))
                            lastItem->setText(k, it->text(k));
                    }
                }
            }
        }
    }

}


void QtWidgetsApplication1::fill_up_to_count(int count)
{
    qDebug() << "UPTO ->" << count;

    int queue_size = full_queue.size()-1;
    if (!queue_size) return;

    QTreeWidget* tree = ui.treetrace;
    QTreeWidgetItem* it = nullptr;
    int tree_count = tree->topLevelItemCount();
    int gap = count - tree_count;
    if (gap <= 0) return;
    int changes = std::min(queue_size, gap);
    if (changes) {
        qDebug() << "UPTO PAGE CHANGES ->" << changes;
        for (int i = 1; i <= changes; i++) {
            it = full_queue.at(queue_size - i);
            if (filter_pass_item(it)) {
                ui.treetrace->addTopLevelItem(it);
            }
        }
    }
}

void QtWidgetsApplication1::fill_partial_tree(int capacity)
{
    qDebug() << "PARTIAL ->" << capacity;

    int queue_size = full_queue.size()-1;
    if (!queue_size) return;
    
    QTreeWidget* tree = ui.treetrace;
    QTreeWidgetItem* it = nullptr;
    int tree_count = tree->topLevelItemCount();
    int gap = capacity - tree_count;
    if (gap <= 0) return;
    int changes = std::min(queue_size, gap);
    if (changes) {
        qDebug() << "PARTIAL CHANGES ->" << changes;
        for (int i = 1; i <= changes; i++) {
            it = full_queue.at(queue_size - i);
            if (filter_pass_item(it)) {
                ui.treetrace->addTopLevelItem(it);
            }
        }
    }
}

void QtWidgetsApplication1::fill_empty_tree(int capacity)
{
    qDebug() << "EMPTY ->"<< capacity;
    int queue_size = full_queue.size()-1;
    if (!queue_size) return;
    QTreeWidgetItem* it=nullptr;
    int changes = std::min(queue_size, capacity);
    if (changes) {
        qDebug() << "EMPTY CHANGES ->" << changes;
        for (int i = 1; i <= changes; i++) {
            it = full_queue.at(queue_size-i);
            if (filter_pass_item(it)) {
                ui.treetrace->addTopLevelItem(it);
            }
        }
    }
}

void QtWidgetsApplication1::trace_scroll_changed(int value)
{
    //qDebug() << "QScrollBar Changed -> " << value;
    QScrollBar* scrollBar = ui.treetrace->verticalScrollBar();
    if (scrollBar->value() == scrollBar->maximum()) { return; }

    calc_thread->pauseThread();
    timer->stop();
    updateToolbar();
    _updateCurrentState();
    if (frozen)
        return;
    else {
        //freeze_treetrace_items(count_per_page);
        //frozen = true;
    }
        
}

void QtWidgetsApplication1::compare_item()
{
    qDebug() << "the full data count ->" << last_data_index;
    QTreeWidgetItem* last = full_queue[last_data_index - 1];
    qDebug() << "item data ->";
    for (int i = 0; i < last->columnCount(); i++) {
        qDebug() << "item col ->" << i << last->text(i);
    }
    qDebug() << "item data DONE";

    QTreeWidgetItem* invisible_root_item = ui.treetrace->invisibleRootItem();
    int child_count = invisible_root_item->childCount();
    QTreeWidgetItem* lastchild = invisible_root_item->child(child_count - 1);
    qDebug() << "item data ->";
    for (int i = 0; i < lastchild->columnCount(); i++) {
        qDebug() << "item col ->" << i << lastchild->text(i);
    }
    qDebug() << "item data DONE";
}

void QtWidgetsApplication1::freeze_treetrace_items(int ncount)
{
    if (frozen) return;
    qDebug() << "BEFORE CLEARING -> " << full_queue.size()-1;
    ui.treetrace->clear();
    qDebug() << "AFETR CLEARING -> " << full_queue.size()-1;
    int size = full_queue.size()-1;
    int total = std::min(size, ncount);
    QTreeWidgetItem* item;
    QList<QTreeWidgetItem*> items;
    while (total > 0) {
        item = full_queue[size - total];
        if (filter_pass_item(item))
            items.append(item);
        total--;
    }
    if (items.size()) {
        ui.treetrace->addTopLevelItems(items);
        qDebug() << "FROZEN -> " << items.size();
    }

}

void QtWidgetsApplication1::adjust_filter_buttons()
{
    QTreeWidget* tree = ui.treetrace;
    QHeaderView* header = tree->header();
    for (int i = 0; i < headerButtonList.count(); i++) {
        QPushButton* button = headerButtonList[i];
        int length = header->sectionPosition(i) + header->sectionSize(i);
        button->setGeometry(length-20, 2, 17, 17);
    }
}

void QtWidgetsApplication1::on_header_section_resized()
{
    adjust_filter_buttons();
}

void QtWidgetsApplication1::on_horizontal_scroll()
{
    adjust_filter_buttons();
}

void QtWidgetsApplication1::about()
{
    QMessageBox::about(this, tr("ZoneTracer"),
       tr("<center><b>ZoneTracer</b> \n"
            "v0.1a</center>"));
}

void QtWidgetsApplication1::reset_all_filters()
{
    qDebug() << "reset_all_filters";
    if (!new_filters.isEmpty()) {
        for (auto it = new_filters.begin(); it != new_filters.end(); ++it) {
            int col_index = it.value()[0].toInt();
            headerButtonList[col_index]->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));
        }
        new_filters.clear();
    }
    filter->new_checks.clear();
}

bool QtWidgetsApplication1::filter_pass_item(QTreeWidgetItem* it)
{
    if (new_filters.isEmpty())
        return true;
    bool matched = true;
    for (const auto& key : new_filters.keys()) {
        QList<QVariant> vlist = new_filters.value(key);
        int col_index = vlist[0].toInt();
        int length = vlist.length();
        QList<QVariant> slice = vlist.mid(1, length);
        if (!slice.contains(it->text(col_index)))
            matched = false;
    }
    return matched;
}

void QtWidgetsApplication1::update_tracewidget_outdate()
{
    //ui.treetrace->setUpdatesEnabled(false);
    //ui.treetrace->setAnimated(false);
    //return;

    int tree_count = ui.treetrace->topLevelItemCount();
    if (tree_count > 0 && !initial_trace)
    {
        //if (initial_trace) return;
        QTreeWidgetItem* invisible_root_item = ui.treetrace->invisibleRootItem();
        QSize itemSize;
        itemSize = getItemSize(invisible_root_item, 0, ui.treetrace->font());
        int v_height = ui.treetrace->viewport()->height();
        int capacity = visible_height / itemSize.height() - 1;

        visible_height = v_height;
        item_height = itemSize.height();
        item_width = itemSize.width();

        QTreeWidgetItem* item = nullptr;
        QList<QTreeWidgetItem*> items;
        int size = full_queue.size()-1;
        int counter = std::min(capacity, size);

        qDebug() << "CAPACITY -> " << capacity;
        qDebug() << "FULL QUEUE SIZE -> " << size;
        qDebug() << "COUNTER -> " << counter;

        while (size && counter > 0) {
            item = full_queue[size - counter];
            items.append(item);
            counter--;
        }

        last_data_index = size - 1;
        int item_size = items.size();
        int t_count = ui.treetrace->topLevelItemCount();
        int changes = std::min(item_size, t_count);

        if (item_size) {
            for (int i = 0; i < changes; i++) {
                if (tree_count > i) {
                    //qDebug() << "CHANGES-I-ITEM -> " << i;
                    QTreeWidgetItem* treeitem = invisible_root_item->child(i);
                    QTreeWidgetItem* it = items[i];

                    if (it && filter_pass_item(it)) {
                        try {
                            int colCount = it->columnCount();
                            if (colCount) {
                                for (int k = 0; k < it->columnCount(); k++) {
                                    if (treeitem->text(k) != it->text(k))
                                        treeitem->setText(k, it->text(k));
                                }
                            }
                        }
                        catch (...) {
                        }
                    }
                }
            }
        }
    }
    else {
        QTreeWidgetItem* item = nullptr;
        QList<QTreeWidgetItem*> items;
        int v_height = ui.treetrace->viewport()->height();
        int capacity = v_height / item_height - 1;
        if (capacity < page_capacity)
            capacity = page_capacity;

        int counter = capacity;
        int size = full_queue.size()-1;
        int counter1 = 0, a = 0;
        if (initial_trace) {
            while (size && a < capacity) {
                if (counter1 < size) {
                    item = full_queue[counter1];
                    if (filter_pass_item(item)) {
                        ui.treetrace->addTopLevelItem(item);
                        a++;
                    }
                    counter1++;
                }
                else { break; }
            }
            if (a >= capacity)
                initial_trace = false;
        }
        else if (size > counter)
        {
            QList<QTreeWidgetItem*> items_append;
            while (size && counter > 0) {
                item = full_queue[size - counter];
                if (filter_pass_item(item))
                    items_append.append(item);
                counter--;
            }
            if (items_append.size()) {
                ui.treetrace->addTopLevelItems(items_append);
            }
        }
    }
    ui.treetrace->scrollToBottom();
}

