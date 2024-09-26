#include "QtWidgetsApplication1.h"
#include <QTimer>
#include <QDebug>
#include "multiThread.h"
#include "columnfilter.h"
#include <QSettings>
#include <QScrollBar>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QTime>
#include "newsession_dialog.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "my_spdlog.h"
#include "TraceTreeWidgetItem.h"

QSize getItemSize(QTreeWidgetItem* item, int column, const QFont& font) {
	QFontMetrics fontMetrics(font);
	QString text = item->text(column);
	QSize textSize = fontMetrics.size(Qt::TextSingleLine, text);
	return QSize(textSize.width() + 20, textSize.height());
}

QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent)
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
	for (QPushButton* button : headerButtonList)
	{
		delete button;
		button = nullptr;
	}
	headerButtonList.clear();

	delete filter;
	filter = nullptr;

	spdlog::drop_all();
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
	calc_thread->queue_ = &full_queue;

	connect(calc_thread, &multiThread::popToRoot, this, &QtWidgetsApplication1::on_pop_to_root);

	resetLayout();
	resetStatusBar();


	setupTreeTrace();
	//init_mylogger();
	INITLOG("path");

	filter = new columnFilterDialog(this);
	ui.treetrace->setAttribute(Qt::WA_OpaquePaintEvent);
	ui.treetrace->setAttribute(Qt::WA_NoSystemBackground);
	showMaximized();
}

void QtWidgetsApplication1::init_mylogger()
{
	try
	{
		auto logger = spdlog::basic_logger_mt("trace_logger", "logs/basic-log.txt");
		logger->info("Welcome to spdlog!");
		logger->error("Some error message with arg: {}", 1);
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
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

	kv = settings.value("app/padding", 1).toInt();
	padding = kv;

	QList<QVariant> modules = settings.value("app/modules").toList();
	qDebug() << "Modules -> " << modules;
	for (QVariant m : modules) {
		monitor_modules.append(m.toString());
	}
}

void QtWidgetsApplication1::_updateCurrentState()
{
}

void QtWidgetsApplication1::updateProgressLeft()
{
	QString runStatus = "";
	if (calc_thread->isSTOPPED())
		runStatus = "READY. ";
	if (!calc_thread->isSTOPPED())
		runStatus = "RUNNING. ";
	if (calc_thread->isPAUSED())
		runStatus = "PAUSED. ";
	int canframe_count = calc_thread->full_count_canframes - padding;
	if (canframe_count <= 0)
		canframe_count = 0;
	int canparser_count = calc_thread->full_count_canparser - padding;
	if (canparser_count <= 0)
		canparser_count = 0;
	QString status_string = runStatus + "CAN Frames: " + QString::number(canframe_count)
		+ ". PDUs: " + QString::number(canparser_count);
	ui.statusBar->showMessage(status_string);

	int ncount = ui.treetrace->topLevelItemCount();
	QString strLeft = QString("Current: %1").arg(ncount);
	leftLabel->setText(strLeft);
}

void QtWidgetsApplication1::updateProgressTimer()
{
	QString strRight; //  = QString("Page Capacity: %1").arg(page_capacity);
	QDateTime end_time = QDateTime::currentDateTime();
	qint64 secondsDifference = start_time.secsTo(end_time);
	QTime time = QTime::fromMSecsSinceStartOfDay(secondsDifference * 1000);
	strRight = time.toString("hh:mm:ss");
	rightLabel->setText(strRight);
}

void QtWidgetsApplication1::updateState()
{
	if (timer_isRunning) return;
	timer_isRunning = true;
	update_tracewindow();
	updateProgressLeft();
	timer_isRunning = false;
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
	// will be removed later.
	//QAction* action = new QAction("Temp Action");
	//ui.menu->addAction(action);
	//connect(action, &QAction::triggered, this, &QtWidgetsApplication1::onActionTriggered);

	// icons
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
	setWindowIcon(QIcon(":/QtWidgetsApplication1/res/spreadsheet.png"));
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


void QtWidgetsApplication1::clearance()
{
	calc_thread->full_count_canframes = 0;
	calc_thread->full_count_canparser = 0;
	//for (QTreeWidgetItem* it : full_queue)
	//{
	//    delete it;
	//    it = nullptr;
	//}
	full_queue.clear();
	ui.treetrace->clear();
}

bool QtWidgetsApplication1::new_session()
{
	int ncount = ui.treetrace->topLevelItemCount();
	if (!ncount) return true;
	if (ncount) {
		leftLabel->setText(QString("Previous Count: %1").arg(ncount));
	}
	if (showNewSession()) {
		clearance();
	}
	return true;
}

void QtWidgetsApplication1::startTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== START TRACE CLICKED ====");

	if (!new_session())
		return;
	start_time = QDateTime::currentDateTime();
	LOGGER_INFO(log_, "==== BEFORE RESUME TRACE ====");
	resumeTrace();
	LOGGER_INFO(log_, "==== AFTER RESUME TRACE ====");
	LOGGER_INFO(log_, "\n");
}
void QtWidgetsApplication1::resumeTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== RESUME TRACE PROCESS ====");

	uint32_t samples = 1000;
	LOGGER_INFO(log_, "==== mysub_can_frames IS NULL? {} ====", mysub_can_frames==nullptr);
	LOGGER_INFO(log_, "==== mysub_can_parser IS NULL? {} ====", mysub_can_parser==nullptr);

	if (mysub_can_frames == nullptr) {
		mysub_can_frames = new ZoneMasterCanMessageDataSubscriber(dds_domainid);
		qRegisterMetaType <can_frame>("can_frame");
	}

	if (mysub_can_parser == nullptr) {
		mysub_can_parser = new ZoneMasterCanParserSubscriber(dds_domainid);
		qRegisterMetaType <canframe>("canframe");
	}
	calc_thread->restartThread();
	LOGGER_INFO(log_, "==== THREAD RESTARTED ====");
	calc_thread->setSubscriber(mysub_can_frames, samples, ui.treetrace);
	LOGGER_INFO(log_, "==== CAN SUB SET ====");
	calc_thread->setCanParserSubscriber(mysub_can_parser, samples, ui.treetrace);
	LOGGER_INFO(log_, "==== PDU SUB SET ====");
	calc_thread->start();
	timer->start(TIMER_HEARTBEAT);
	LOGGER_INFO(log_, "==== HEARTBEAT STARTED ====");
	timer_dustbin->start(TIMER_HEARTBEAT);
	frozen = false;
	progress_secs = start_time.secsTo(end_time);
	updateToolbar();
	LOGGER_INFO(log_, "==== THREAD PAUSE STATUS -> {} ====", calc_thread->isPAUSED());
	LOGGER_INFO(log_, "==== END OF RESUMING ====");
}

void QtWidgetsApplication1::stopTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== STOP TRACE CLICKED ====");

	calc_thread->stopThread();
	timer->stop();
	timer_dustbin->stop();

	mysub_can_frames = nullptr;
	mysub_can_parser = nullptr;

	initial_trace = true;
	updateToolbar();
	updateProgressLeft();

	last_status = "STOPPED";

	LOGGER_INFO(log_, "==== GOING TO FREEZE ====");

	frozen = true;
	LOGGER_INFO(log_, "==== FROZEN STATUS {} ====", frozen);
	freeze_treetrace_items(count_per_page);
	LOGGER_INFO(log_, "==== END OF STOPPING TRACE ====");
	LOGGER_INFO(log_, "\n");

}

void QtWidgetsApplication1::pauseTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== PAUSE BUTTON CLICKED ====");
	//this->stopTrace();

	if (1) {
		if (calc_thread->isPAUSED()) {
			LOGGER_INFO(log_, "==== CONTINUE AFTER PAUSE ====");
			frozen = false;
			ui.treetrace->clear();
			initial_trace = true;

			LOGGER_INFO(log_, "==== FROZEN STATUS {} ====", frozen);
			LOGGER_INFO(log_, "==== BEFORE RESUME TRACE ====");
			LOGGER_INFO(log_, "==== QUEUE SIZE BEFORE RESUMING {} ====", full_queue.size()-padding);
			resumeTrace();
			LOGGER_INFO(log_, "==== AFTER RESUME TRACE ====");
			LOGGER_INFO(log_, "\n");
			return;
		}
		LOGGER_INFO(log_, "==== PAUSE TRACE ====");
		calc_thread->pauseThread();
		timer->stop();
		paused_index = full_queue.size()-padding;
		LOGGER_INFO(log_, "==== THREAD PAUSE STATUS -> {} ====", calc_thread->isPAUSED());
		LOGGER_INFO(log_, "PAUSED FULL QUEUE -> {}", paused_index);
		LOGGER_INFO(log_, "==== QUEUE SIZE WHEN PAUSE {} ====", full_queue.size() - padding);
		last_status = "PAUSED";
		LOGGER_INFO(log_, "BEFORE FREEZING");
		freeze_treetrace_items(count_per_page);
		LOGGER_INFO(log_, "AFTER FREEZING");
		updateToolbar();
		updateProgressLeft();
	}
	LOGGER_INFO(log_, "==== END OF PAUSE BUTTON CLICKED ====");
	LOGGER_INFO(log_, "\n");
}


void QtWidgetsApplication1::formatRow(int x)
{
	qDebug() << "formatRow..." << x;
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
			single_config.append(s);
		}
	}
	if (single_config.size() == 1) {
		new_filters.remove(colName);
		return;
	}
	new_filters.insert(colName, single_config);
}


void QtWidgetsApplication1::on_pop_to_root(QTreeWidgetItem* item)
{
	//if (timer_isRunning) return;
	//timer_isRunning = true;
	auto log_ = GETLOG("WORKFLOW");
	rwLock.lockForWrite();
	LOGGER_INFO(log_, "==== WRITE LOCK ====");
	if (item != NULL) {
		full_queue.enqueue(item);
		LOGGER_INFO(log_, "ENQUEUING -> {}", item->text(0).toStdString());
	}
	rwLock.unlock();
	LOGGER_INFO(log_, "==== WRITE UNLOCK ====");
	//timer_isRunning = false;
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
	updateProgressTimer();
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
	/*   QList<QTreeWidgetItem*> visibleItems =
		   getVisibleItems(ui.treetrace);
	   qDebug() << "Visual Items ->" << visibleItems.size();
   */
}

void QtWidgetsApplication1::update_tracewindow()
{
	auto log_workflow = GETLOG("WORKFLOW");
	LOGGER_INFO(log_workflow, "======= UPDATE_TRACEWINDOW =======");
	LOGGER_INFO(log_workflow, "LAST STATUS -> {}", last_status.toStdString());
	int queue_size = full_queue.size() - padding;
	//if (queue_size > 0)
		//qDebug() << "QUEUE_SIZE -> " << queue_size << "LAST ITEM COLUMN COUNTS" << full_queue.at(queue_size-1)->columnCount();

	QTreeWidget* tree = ui.treetrace;
	QTreeWidgetItem* invisible_root_item = tree->invisibleRootItem();

	QSize itemSize;
	itemSize = getItemSize(invisible_root_item, 0, ui.treetrace->font());
	int v_height = ui.treetrace->viewport()->height();
	int capacity = visible_height / itemSize.height() - 1;

	visible_height = v_height;
	item_height = itemSize.height();
	item_width = itemSize.width();
	LOGGER_INFO(log_workflow, "UPDATE_TRACEWINDOW CAPACITY -> {}", capacity);

	int tree_count = ui.treetrace->topLevelItemCount();
	LOGGER_INFO(log_workflow, "TREE COUNT -> {}", tree_count);
	if (tree_count >= capacity) {
		LOGGER_INFO(log_workflow, "==== REDRAW TREE ====");
		refresh_full_tree(capacity);
	}
	else if (tree_count) {
		LOGGER_INFO(log_workflow, "==== HALF TREE ====");
		fill_partial_tree(capacity); // 追加
	}
	else if (!tree_count) {
		LOGGER_INFO(log_workflow, "==== EMPTY TREE ====");
		fill_empty_tree(capacity); // 填空一条
	}
	ui.treetrace->scrollToBottom();
	LOGGER_INFO(log_workflow, "======= END of update_tracewindow =======");
	LOGGER_INFO(log_workflow, "\n");
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
	auto log_draw = GETLOG("WORKFLOW");

	int queue_size = full_queue.size() - padding;
	LOGGER_INFO(log_draw, "DRAW QUEUE SIZE -> {}", queue_size);
	LOGGER_INFO(log_draw, "PADDING -> {}", padding);

	if (queue_size <= 0) return;

	QTreeWidget* tree = ui.treetrace;
	QTreeWidgetItem* it = nullptr;
	int changes = std::min(queue_size, capacity);
	LOGGER_INFO(log_draw, "DRAW CHANGES -> {}", changes);
	if (changes) {
		int count = tree->topLevelItemCount();
		LOGGER_INFO(log_draw, "TREE COUNT -> {}", count);
		for (int i = 1; i <= changes; i++) {
			int idx = queue_size - i;
			LOGGER_INFO(log_draw, "DRAW ITEM -> {}", idx);
			bool bchanged = false;
			it = full_queue.at(idx);
			if (filter_pass_item(it)) {
				if (count > 0)
				{
					QTreeWidgetItem* lastItem = tree->topLevelItem(count - i);
					LOGGER_INFO(log_draw, "ORIGINAL -> {}", lastItem->text(0).toStdString());
					LOGGER_INFO(log_draw, "REPLACED -> {}", it->text(0).toStdString());
					for (int k = 0; k < it->columnCount(); k++) {
						if (lastItem->text(k) != it->text(k)) {
							lastItem->setText(k, it->text(k));
							bchanged = true;
						}
					}
					if (bchanged) {
						LOGGER_INFO(log_draw, "NACH -> {}", lastItem->text(0).toStdString());
					}
				}
			}
		}
	}

}

void QtWidgetsApplication1::fill_up_to_count(int count)
{
	qDebug() << "UPTO ->" << count;

	int queue_size = full_queue.size() - padding;
	if (queue_size <= 0) return;

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
			if (it != nullptr && filter_pass_item(it)) {
				ui.treetrace->addTopLevelItem(it);
			}
		}
	}
}

void QtWidgetsApplication1::fill_partial_tree(int capacity)
{
	auto log_partial = GETLOG("WORKFLOW");
	int queue_size = full_queue.size() - padding;
	LOGGER_INFO(log_partial, "FULL QUEUE SIZE IN PARTIAL FUNC -> {}", queue_size);
	if (queue_size <= 0) return;

	QTreeWidget* tree = ui.treetrace;
	int tree_count = tree->topLevelItemCount();
	int gap = capacity - tree_count;
	LOGGER_INFO(log_partial, "GAP -> {}", gap);
	if (gap <= 0) return;
	int changes = std::min(queue_size-1, gap);
	LOGGER_INFO(log_partial, "PARTIAL CHANGES -> {}", changes);

	// mark from here

	if (changes>0) {
		for (int i = 1; i < changes; i++) {
			int idx = queue_size - i;
			LOGGER_INFO(log_partial, "PARTIAL INDEX -> {}", idx);
			if (idx >= 0) {
				QTreeWidgetItem* it = nullptr;
				//it = full_queue.at(idx);
				it = read_item_from_queue(idx);
				
				//if (it /* && filter_pass_item(it) */) {
				//	ui.treetrace->addTopLevelItem(it);
				//}
			}
		}
	}

	// end of mark
	LOGGER_INFO(log_partial, "END OF PARTIAL FILLING");
	LOGGER_INFO(log_partial, "====================");
}

void QtWidgetsApplication1::fill_empty_tree(int capacity)
{
	//auto log_empty = GETLOG("EMPTY_TREE");
	auto log_empty = GETLOG("WORKFLOW");

	qDebug() << "EMPTY TREE -> " << capacity;
	int queue_size = full_queue.size() - padding;
	LOGGER_INFO(log_empty, "FULL QUEUE SIZE IN EMPTY FUNC -> {}", queue_size);
	if (queue_size <= 0) return;

	qDebug() << "FULLQUEUE SIZE ->" << queue_size;
	int changes = std::min(queue_size, capacity);
	if (changes > 0) {
		changes = 1;
		qDebug() << "EMPTY CHANGES ->" << changes;
		LOGGER_INFO(log_empty, "EMPTY CHANGES -> {}", changes);
		for (int i = 0; i < changes; i++) {
			QTreeWidgetItem* it = nullptr;
			int idx = queue_size - i;
			//if (last_status == "PAUSED") // 从“暂停”状态过来的
			//{
			//    if (idx > pause_index) {
			//        it = full_queue.at(idx);
			//        LOGGER_INFO(log_empty, "PAUSED INDEX -> {}", idx);
			//    }
			//}
			//else // 从“空闲”状态过来的。
			{
				it = full_queue.at(idx);
				LOGGER_INFO(log_empty, "READY INDEX -> {}", idx);
			}
			if (it != nullptr) {
				qDebug() << "ITEM INDEX ->" << idx;
				LOGGER_INFO(log_empty, "ITEM INDEX -> {}", idx);
				qDebug() << "ITEM  TIMESTAMP ->" << it->text(0);
				LOGGER_INFO(log_empty, "ITEM SOURCE -> {}", ((TraceTreeWidgetItem*)it)->getSource().toStdString());
				if (filter_pass_item(it)) {
					ui.treetrace->addTopLevelItem(it);
				}
			}
		}
	}
	LOGGER_INFO(log_empty, "EXITING EMPTY TREE.");
}

void QtWidgetsApplication1::trace_scroll_changed(int value)
{
	//qDebug() << "QScrollBar Changed -> " << value;
	QScrollBar* scrollBar = ui.treetrace->verticalScrollBar();
	if (scrollBar->value() == scrollBar->maximum()) { return; }

	auto log_ = GETLOG("WORKFLOW");
	calc_thread->pauseThread();
	timer->stop();
	paused_index = full_queue.size()-padding;
	last_status = "PAUSED";
	updateToolbar();

	if (frozen)
		return;
	else {
		LOGGER_INFO(log_, "SCROLLED FULL QUEUE -> {}", paused_index);
		LOGGER_INFO(log_, "SCROLLED BEFORE FREEZING");
		freeze_treetrace_items(count_per_page);
		frozen = true;
		LOGGER_INFO(log_, "SCROLLED AFTER FREEZING");
	}

}

void QtWidgetsApplication1::compare_item()
{
	qDebug() << "the full data count ->" << last_data_index;
	QTreeWidgetItem* last = full_queue.at(last_data_index - 1);
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
	auto log_frozen = GETLOG("WORKFLOW");

	LOGGER_INFO(log_frozen, "NOTHING. EXITING...");
	LOGGER_INFO(log_frozen, "====================");
	return;

	if (frozen) return;

	int size = full_queue.size() - padding;
	LOGGER_INFO(log_frozen, "SIZE -> {}", size);
	if (size <= 0) return;

	qDebug() << "TARGET COUNT -> " << ncount;
	LOGGER_INFO(log_frozen, "TARGET -> {}", ncount);
	qDebug() << "BEFORE CLEARING -> " << size;
	ui.treetrace->clear();
	qDebug() << "AFETR CLEARING -> " << full_queue.size() - padding;
	int total = std::min(size, ncount) - 5;
	qDebug() << "TOTAL -> " << total;
	LOGGER_INFO(log_frozen, "TOTAL -> {}", total);
	QTreeWidgetItem* it = nullptr;
	QList<QTreeWidgetItem*> item_list;
	item_list.clear();
	for (int i = 0; i < total; i++)
	{
		int idx = size - i;
		LOGGER_INFO(log_frozen, "INDEX -> {}", idx);
		it = full_queue.at(idx);
		LOGGER_INFO(log_frozen, "ITEM_COL -> {}", it->text(0).toStdString());
		if (it != nullptr && filter_pass_item(it)) {
			item_list.append(it);
		}
	}
	if (item_list.size())
	{
		ui.treetrace->addTopLevelItems(item_list);
		int frozen_size = item_list.size();
		qDebug() << "FROZEN -> " << frozen_size;
		LOGGER_INFO(log_frozen, "FRONZEN -> {}", frozen_size);
	}
}

void QtWidgetsApplication1::adjust_filter_buttons()
{
	QTreeWidget* tree = ui.treetrace;
	QHeaderView* header = tree->header();
	for (int i = 0; i < headerButtonList.count(); i++) {
		QPushButton* button = headerButtonList[i];
		int length = header->sectionPosition(i) + header->sectionSize(i);
		button->setGeometry(length - 20, 2, 16, 16);
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
	if (it == NULL) return false;
	if (calc_thread->isRUN() && it) {
		try
		{
			int children_count = it->childCount();
			if (children_count > 0) {
				return false;
			}
		}
		catch (...)
		{
		}
	}
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

bool QtWidgetsApplication1::showNewSession()
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::warning(this, "Warning",
		tr("New Session?"),
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) {
		return true;
	}
	else {
		return false;
	}
}

QTreeWidgetItem* QtWidgetsApplication1::read_item_from_queue(int index)
{
	auto log_ = GETLOG("WORKFLOW");
	QTreeWidgetItem* item = nullptr;
	rwLock.lockForRead();
	LOGGER_INFO(log_, "==== READ LOCK ====");
	item = this->full_queue.at(index);
	//LOGGER_INFO(log_, "ITEM CONTENT -> {}", item->text(0).toStdString());
	rwLock.unlock();
	LOGGER_INFO(log_, "==== READ UNLOCK ====");
	return item;
}