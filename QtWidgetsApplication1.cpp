#include "QtWidgetsApplication1.h"
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTime>
#include <QTimer>
#include <QSplashScreen>
#include <QPixmap>
#include <QLineEdit>
#include <QShortcut>

#include "TraceTreeWidgetItem.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "my_spdlog.h"
#include "multiThread.h"
#include "columnfilter.h"
//#include "LineEditFilter.h"

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
	shortcuts();
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
	if (mysub_lin_frames != nullptr) {
		delete mysub_lin_frames;
		mysub_lin_frames = nullptr;
	}
	if (mysub_lin_parser != nullptr) {
		delete mysub_lin_parser;
		mysub_lin_parser = nullptr;
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

	delete usagedialog;
	usagedialog = nullptr;
	reset_find_status();

	state_manager.changeState(State::COMPLETE);
	state_manager.printHistory();

	print_item_queue(shown_queue);
	print_item_queue(full_queue_stream);

	clear_queue(shown_queue);
	clear_queue(full_queue_stream);
	clear_queue(filtered_queue);

	print_item_queue(shown_queue);
	print_item_queue(full_queue_stream);

	spdlog::drop_all();
}

void QtWidgetsApplication1::shortcuts()
{
	QShortcut* shortcut_help = new QShortcut(QKeySequence("Ctrl+H"), this);
	connect(shortcut_help, &QShortcut::activated, this, &QtWidgetsApplication1::help_usage);
}

void QtWidgetsApplication1::init()
{
	ui.setupUi(this);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(ui.widget->sizePolicy().hasHeightForWidth());
	ui.Lower->setSizePolicy(sizePolicy);
	ui.comboBox_Page->addItem(QString::number(1));
	ui.actionreplay->setVisible(false);

	get_default_configurations();
	createActions();

	timer = new QTimer();
	timer_dustbin = new QTimer();
	connect(timer, &QTimer::timeout, this, &QtWidgetsApplication1::updateState);
	connect(timer_dustbin, &QTimer::timeout, this, &QtWidgetsApplication1::dustbin);

	calc_thread = new multiThread();
	calc_thread->monitor_modules = monitor_modules;
	calc_thread->queue_ = &shown_queue;
	connect(calc_thread, &multiThread::popToRoot, this, &QtWidgetsApplication1::on_pop_to_root);
	connect(this, &QtWidgetsApplication1::record_latest_index, this, &QtWidgetsApplication1::update_latest_index);

	filter = new columnFilterDialog(this);
	resetLayout();
	resetStatusBar();
	setupTreeTrace();

	//
	reset_find_status();

	updateContinuousProgress();
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

	kv = settings.value("app/count_per_page", 4000).toInt();
	count_per_page = kv;

	kv = settings.value("app/padding", 0).toInt();
	padding = kv;

	kv = settings.value("app/debuglog", 1).toInt();
	debuglog = kv;

	kv = settings.value("app/maximum_total", 400000).toInt();
	maximum_total = kv;

	QList<QVariant> modules = settings.value("app/modules").toList();
	qDebug() << "Modules -> " << modules;
	for (QVariant m : modules) {
		monitor_modules.append(m.toString());
	}
}

void QtWidgetsApplication1::updateProgressRunStatus()
{
	QString runStatus = "READY. ";
	State state = state_manager.current_state();
	switch (state) {
	case State::INIT:
		runStatus = "READY. ";
		break;
	case State::RESUMED:
	case State::START:
		runStatus = "RUNNING. ";
		break;
	case State::PAUSE:
		runStatus = "PAUSED. ";
		break;
	case State::STOPPED:
	case State::COMPLETE:
		runStatus = "COMPLETED. ";
		break;
	default:
		runStatus = "READY. ";
	}

	int can_messages_count = calc_thread->full_count_canframes - padding;
	if (can_messages_count <= 0)
		can_messages_count = 0;
	int canpdu_count = calc_thread->full_count_canparser - padding;
	if (canpdu_count <= 0)
		canpdu_count = 0;
	int lin_messages_count = calc_thread->full_count_linframes - padding;
	if (lin_messages_count <= 0)
		lin_messages_count = 0;

	QString status_string = runStatus
		+ "CAN Messages: " + QString::number(can_messages_count)
		+ ". CAN PDUs: " + QString::number(canpdu_count)
		+ ". LIN Messages: " + QString::number(lin_messages_count)
		+ ". Total: " + QString::number(can_messages_count
			+ canpdu_count
			+ lin_messages_count);
	ui.statusBar->showMessage(status_string);
}

void QtWidgetsApplication1::updateContinuousProgress()
{
	updateProgressRunStatus();
	int page_index = 1;
	if (calc_thread->isRUN()) {
		int ncount = ui.treetrace->topLevelItemCount();
		page_index = paused_instant_index / page_capacity + 1;
		QString strLeft_run = QString("Count: %1/%2")
			.arg(ncount)
			.arg(page_capacity);
		leftLabel->setText(strLeft_run);
	}
	else {
		int ncount = ui.treetrace->topLevelItemCount();
		if (filtered_size() > 0) {
			page_index = filtered_size() / count_per_page + 1;
		}
		else {
			page_index = paused_instant_index / count_per_page + 1;
		}

		QString strLeft_idle = QString("Count: %1/%2")
			.arg(ncount)
			.arg(count_per_page);
		leftLabel->setText(strLeft_idle);
	}
	QString strPage = QString("%1").arg(page_index);
	ui.label_Current->setText(strPage);

	State current_state = state_manager.current_state();
	switch (current_state) {
	case State::PAUSE:
	case State::STOPPED:
		ui.comboBox_Page->setEnabled(true);
		ui.pb_First->setEnabled(true);
		ui.pb_Prev->setEnabled(true);
		ui.pb_Next->setEnabled(true);
		ui.pb_Last->setEnabled(true);
		ui.pb_Goto->setEnabled(true);
		break;
	default:
		ui.comboBox_Page->setCurrentIndex(0);
		ui.comboBox_Page->setEnabled(false);
		ui.pb_First->setEnabled(false);
		ui.pb_Prev->setEnabled(false);
		ui.pb_Next->setEnabled(false);
		ui.pb_Last->setEnabled(false);
		ui.pb_Goto->setEnabled(false);
		break;
	}

	bool nonfiltered = new_filters.isEmpty();
	QFont font("SimSun", 8);
	font.setBold(true);
	ui.label_Filtered->setFont(font);
	if (nonfiltered)
	{
		ui.label_Filtered->setStyleSheet("color: black;");
		ui.label_Filtered->setText("Normal");
	}
	else {
		ui.label_Filtered->setStyleSheet("color: red;");
		ui.label_Filtered->setText("Filtered");
	}

}

void QtWidgetsApplication1::updateProgressTimer()
{
	QString strRight;
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
	updateContinuousProgress();
	timer_isRunning = false;
}

void QtWidgetsApplication1::createActions()
{
	//qDebug() << "createActions...";
	connect(ui.actionstart, &QAction::triggered, this, &QtWidgetsApplication1::startTrace);
	connect(ui.actionstop, &QAction::triggered, this, &QtWidgetsApplication1::stopTrace);
	connect(ui.actionpause, &QAction::triggered, this, &QtWidgetsApplication1::pauseTrace);
	connect(ui.actionreplay, &QAction::triggered, this, &QtWidgetsApplication1::replayTrace);
	connect(ui.actionmode, &QAction::triggered, this, &QtWidgetsApplication1::display_mode_switch);
	connect(ui.pushButton_search, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonSearchClicked);
	connect(ui.actionAbout, &QAction::triggered, this, &QtWidgetsApplication1::about);
	connect(ui.actionreset, &QAction::triggered, this, &QtWidgetsApplication1::reset_all_filters);
	connect(ui.pb_First, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonFirstClicked);
	connect(ui.pb_Prev, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonPreviousClicked);
	connect(ui.pb_Next, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonNextClicked);
	connect(ui.pb_Last, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonLastClicked);
	connect(ui.pb_Goto, &QPushButton::clicked, this, &QtWidgetsApplication1::ButtonGotoClicked);
	connect(ui.actionHelp, &QAction::triggered, this, &QtWidgetsApplication1::help_usage);
	connect(ui.pb_prev_result, &QPushButton::clicked, [this]() {find_direction = 1; ButtonSearchClicked(); });
	connect(ui.pb_next_result, &QPushButton::clicked, [this]() {find_direction = 0; ButtonSearchClicked(); });
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
	ui.actionreplay->setIcon(QIcon(":/QtWidgetsApplication1/res/replay.svg"));
	ui.pb_next_result->setIcon(QIcon(":/QtWidgetsApplication1/res/arrow-down.png"));
	ui.pb_prev_result->setIcon(QIcon(":/QtWidgetsApplication1/res/arrow-up.png"));

	// toolbar actions
	ui.toolbar->addAction(ui.actionstart);
	ui.toolbar->addAction(ui.actionstop);
	ui.toolbar->addAction(ui.actionpause);
	ui.toolbar->addAction(ui.actionreplay);
	ui.toolbar->addAction(ui.actionmode);
	ui.toolbar->addAction(ui.actionreset);

	//ui.treetrace->setContextMenuPolicy(Qt::CustomContextMenu);
	// connect(ui.treetrace, &QTreeWidget::customContextMenuRequested, this, &QtWidgetsApplication1::prepareMenu);

	connect(ui.treetrace->header(), &QHeaderView::sectionResized, this, &QtWidgetsApplication1::on_header_section_resized);
	connect(ui.treetrace->horizontalScrollBar(), &QScrollBar::valueChanged, this, &QtWidgetsApplication1::on_horizontal_scroll);
	//connect(ui.treetrace, &QTreeWidget::currentItemChanged, this, &QtWidgetsApplication1::updateHeaderStyle);

	datachoice = ui.comboBox;
	datachoice->setEnabled(false);
	connect(datachoice, &QComboBox::currentTextChanged, this, &QtWidgetsApplication1::ChangeHeader);
	connect(ui.treetrace->verticalScrollBar(), &QScrollBar::valueChanged, this, &QtWidgetsApplication1::trace_scroll_changed);
	setWindowIcon(QIcon(":/QtWidgetsApplication1/res/spreadsheet.png"));
	updateToolbar();

	// mysearch
	ui.pushButton_search->setStyleSheet(
		"QPushButton:focus {"
		"   border: 2px solid blue;"
		"}");
	ui.mysearch->setStyleSheet(
		"QPlainTextEdit:focus {"
		"   background-color: yellow;"
		"   border: 2px solid blue;"
		"}");
	ui.mysearch->installEventFilter(this);
	ui.pushButton_search->installEventFilter(this);
	ui.pushButton_search->setFocusPolicy(Qt::TabFocus);
	QWidget::setTabOrder(ui.treetrace, ui.mysearch);
	QWidget::setTabOrder(ui.mysearch, ui.pushButton_search);
	QWidget::setTabOrder(ui.pushButton_search, ui.treetrace);
	ui.treetrace->setFocus();
}

void QtWidgetsApplication1::resetStatusBar()
{
	QString strLeftTemplate = "Count: 0";
	QString strRightTemplate = "00:00:00";
	leftLabel = new QLabel(strLeftTemplate, this);
	leftWidget = new QWidget(this);
	leftLayout = new QHBoxLayout(leftWidget);
	leftLayout->addWidget(leftLabel);
	//leftLayout->addStretch();

	rightLabel = new QLabel(strRightTemplate, this);
	rightWidget = new QWidget(this);
	rightLayout = new QHBoxLayout(rightWidget);
	//rightLayout->addStretch();
	rightLayout->addWidget(rightLabel);

	ui.statusBar->addPermanentWidget(leftWidget);
	ui.statusBar->addPermanentWidget(rightWidget);
	updateProgressRunStatus();
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
}

void QtWidgetsApplication1::onActionTriggered()
{
}


void QtWidgetsApplication1::clearance()
{
	calc_thread->full_count_canframes = 0;
	calc_thread->full_count_canparser = 0;
	calc_thread->full_count_linframes = 0;
	calc_thread->full_count_linparser = 0;
	passed_uuid_set.clear();
	safe_clear_trace();
}

void QtWidgetsApplication1::initialize_new_session()
{
	calc_thread->full_count_canframes = 0;
	calc_thread->full_count_canparser = 0;
	calc_thread->full_count_linframes = 0;
	calc_thread->full_count_linparser = 0;

	paused_instant_index = -1;
	passed_uuid_set.clear();

	clear_queue(shown_queue);
	clear_queue(full_queue_stream);
	clear_queue(filtered_queue);
}

bool QtWidgetsApplication1::new_session()
{
	int ncount = ui.treetrace->topLevelItemCount();
	if (!ncount) return true;

	if (showNewSession()) {
		invisibles.clear();
		ui.treetrace->clear();
		initialize_new_session();
	}
	return true;
}

void QtWidgetsApplication1::startTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "==== START TRACE CLICKED ====");
	state_manager.changeState(State::START);

	if (!new_session())
		return;
	start_time = QDateTime::currentDateTime();
	//LOGGER_INFO(log_, "==== BEFORE RESUME TRACE ====");
	resumeTrace();
	last_status = "STARTED";
	traceStyleQSS();
	//LOGGER_INFO(log_, "==== AFTER RESUME TRACE ====");
	//LOGGER_INFO(log_, "\n");
}

void QtWidgetsApplication1::resumeTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "==== RESUME TRACE PROCESS ====");

	uint32_t samples = 1000;
	invisibles.clear();
	reset_find_status();
	if (mysub_can_frames == nullptr) {
		mysub_can_frames = new ZoneMasterCanMessageDataSubscriber(dds_domainid);
		qRegisterMetaType <can_frame>("can_frame");
	}

	if (mysub_can_parser == nullptr) {
		mysub_can_parser = new ZoneMasterCanParserSubscriber(dds_domainid);
		qRegisterMetaType <canframe>("canframe");
	}

	if (mysub_lin_frames == nullptr) {
		mysub_lin_frames = new ZoneMasterLinMessageDataSubscriber(dds_domainid);
		qRegisterMetaType <linMessage>("linMessage");
	}

	if (mysub_lin_parser == nullptr) {
		mysub_lin_parser = new ZoneMasterLinParserSubscriber(dds_domainid);
		qRegisterMetaType <linFrames>("linFrames");
	}

	calc_thread->restartThread();
	//LOGGER_INFO(log_, "==== CAN SUB SET ====");
	calc_thread->setCanSubscriber(mysub_can_frames, samples, ui.treetrace);
	calc_thread->setCanParserSubscriber(mysub_can_parser, samples, ui.treetrace);
	calc_thread->setLinSubscriber(mysub_lin_frames, samples, ui.treetrace);
	calc_thread->setLinParserSubscriber(mysub_lin_parser, samples, ui.treetrace);
	//LOGGER_INFO(log_, "==== THREAD RESTARTED ====");
	calc_thread->start();
	timer->start(TIMER_HEARTBEAT);
	//LOGGER_INFO(log_, "==== HEARTBEAT STARTED ====");
	timer_dustbin->start(TIMER_HEARTBEAT);
	frozen = false;
	progress_secs = start_time.secsTo(end_time);
	updateToolbar();
	//LOGGER_INFO(log_, "==== THREAD PAUSE STATUS -> {} ====", calc_thread->isPAUSED());
	//LOGGER_INFO(log_, "==== END OF RESUMING ====");
}

void QtWidgetsApplication1::stopTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "==== STOP TRACE CLICKED ====");
	if (last_status == "STOPPED" || last_status == "READY") return;
	state_manager.changeState(State::STOPPED);

	calc_thread->stopThread();
	timer->stop();
	timer_dustbin->stop();

	mysub_can_frames = nullptr;
	mysub_can_parser = nullptr;
	mysub_lin_frames = nullptr;
	mysub_lin_parser = nullptr;

	initial_trace = true;
	last_status = "STOPPED";
	frozen = true;
	show_fullpage();
	updateToolbar();
	updateContinuousProgress();
	int lastIndex = ui.treetrace->topLevelItemCount() - 1;
	if (lastIndex >= 0) {
		ui.treetrace->setCurrentItem(ui.treetrace->topLevelItem(lastIndex));
	}
	traceStyleQSS();
	//LOGGER_INFO(log_, "==== END OF STOPPING TRACE ====");
	//LOGGER_INFO(log_, "\n");

}

void QtWidgetsApplication1::replayTrace()
{
	qDebug() << "REPLAY...";
}

void QtWidgetsApplication1::resume_from_pause_trace()
{
	auto log_ = GETLOG("WORKFLOW");
	if (last_status == "STOPPED" || last_status == "READY") return;
	if (calc_thread->isPAUSED()) {
		//LOGGER_INFO(log_, "==== CONTINUE AFTER PAUSE ====");
		frozen = false;
		initial_trace = true;
		state_manager.changeState(State::RESUMED);
		//LOGGER_INFO(log_, "==== BEFORE RESUME TRACE ====");
		resumeTrace();
		//LOGGER_INFO(log_, "==== AFTER RESUME TRACE ====");
		//LOGGER_INFO(log_, "\n");
		return;
	}
}

void QtWidgetsApplication1::pauseTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "==== PAUSE BUTTON CLICKED ====");
	if (last_status == "STOPPED" || last_status == "READY") return;

	updateProgressRunStatus();
	if (calc_thread->isPAUSED()) {
		ui.treetrace->clear();
		restore_full_queue();
		resume_from_pause_trace();
		traceStyleQSS();
		return;
	}
	//LOGGER_INFO(log_, "==== PAUSE TRACE ====");
	calc_thread->pauseThread();
	timer->stop();
	qDebug() << "INSTANT INDEX ->" << paused_instant_index;
	show_fullpage();
	last_status = "PAUSED";
	state_manager.changeState(State::PAUSE);
	updateToolbar();
	updateContinuousProgress();
	int lastIndex = ui.treetrace->topLevelItemCount() - 1;
	if (lastIndex >= 0) {
		ui.treetrace->setCurrentItem(ui.treetrace->topLevelItem(lastIndex));
	}
	traceStyleQSS();
	//LOGGER_INFO(log_, "==== END OF PAUSE BUTTON CLICKED ====");
	//LOGGER_INFO(log_, "\n");
}

void QtWidgetsApplication1::setupTreeTrace()
{
	QTreeWidget* t = ui.treetrace;
	QHeaderView* header = t->header();
	t->setSelectionBehavior(QTreeView::SelectRows);
	t->setFocusPolicy(Qt::WheelFocus);
	t->header()->setHighlightSections(true);
	t->header()->setStretchLastSection(true);
	t->setWindowTitle(QObject::tr("CAN/LIN"));
	t->setAttribute(Qt::WA_OpaquePaintEvent);
	t->setAttribute(Qt::WA_NoSystemBackground);
	t->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	t->setHeaderLabels(initialHeader);
	t->setSortingEnabled(true);
	t->setColumnWidth(0, 150);
	t->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	t->invisibleRootItem()->setHidden(true);
	t->installEventFilter(this);
	t->setToolTipDuration(0);
	header->setDefaultSectionSize(150);
	header->setSectionResizeMode(QHeaderView::Interactive);

	// no more font setting...2024-10-30 10:37:16
	//QFont font("SimSun", 8);
	//font.setBold(true);
	//t->setFont(font);
	initialHeaders();
	traceStyleQSS();
}

void QtWidgetsApplication1::traceStyleQSS()
{
	State state = state_manager.current_state();
	switch (state)
	{
	case State::START:
	case State::RESUMED:
		ui.treetrace->setStyleSheet(treewidget_header_style +
			"QTreeWidget::item:hover{\n"
			"	color: rgb(0, 0, 0);\n"
			"	background-color: rgb(255, 255, 255);\n"
			"}\n");
		break;
	default:
		ui.treetrace->setStyleSheet(treewidget_header_style);
		break;
	}
}

void QtWidgetsApplication1::initialHeaders()
{
	QHeaderView* header = ui.treetrace->header();
	int button_count = std::max(header->count(), 5);
	for (int i = 0; i < button_count; i++) {
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
	if (column_index >= CurrentHeader.count())
		return;
	QString colName = CurrentHeader[column_index];
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
	QString colName = CurrentHeader[column_index];

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

	if (!this->filtered_queue.isEmpty()) {
		for (TraceTreeWidgetItem* it_remove : filtered_queue) {
			delete it_remove;
		}
		filtered_queue.clear();
	}
	construct_filtered_queue(paused_instant_index);
}

void QtWidgetsApplication1::construct_filtered_queue(int full_count)
{
	assert(filtered_queue.size() == 0);
	for (int i = 0; i < full_count; i++) {
		TraceTreeWidgetItem* it = full_queue_stream.at(i)->clone();
		if (filter_pass_item(it)) {
			filtered_queue.enqueue(it);
		}
	}
}

void QtWidgetsApplication1::on_pop_to_root(TraceTreeWidgetItem* item)
{
	auto log_ = GETLOG("WORKFLOW");
	timer_isRunning = true;
	if (item != NULL) {
		//m_mutex.lock();
		item->handled = true;
		TraceTreeWidgetItem* item_backup = (TraceTreeWidgetItem*)item->clone();
		if (calc_thread->isPAUSED()) {
			full_queue_stream.enqueue(item_backup);
			//m_mutex.unlock();
		}
		else {
			shown_queue.enqueue(item_backup->clone());
			//m_mutex.unlock();
			full_queue_stream.enqueue(item_backup);
		}
		if (filter_pass_item(item_backup)) {
			filtered_queue.enqueue(item_backup->clone());
		}
		if (full_queue_stream.size() > maximum_total) {
			TraceTreeWidgetItem* it_rem1 = full_queue_stream.dequeue();
			delete it_rem1;
			it_rem1 = nullptr;
			if (shown_queue.size() > 3 * count_per_page) {
				TraceTreeWidgetItem* it_rem2 = shown_queue.dequeue();
				delete it_rem2;
				it_rem2 = nullptr;
			}
			if (filtered_queue.size() > maximum_total) {
				TraceTreeWidgetItem* it_rem3 = filtered_queue.dequeue();
				delete it_rem3;
				it_rem3 = nullptr;
			}
		}
		emit record_latest_index(full_queue_stream.size());
	}
	timer_isRunning = false;
}

void QtWidgetsApplication1::ChangeHeader(const QString& text)
{
	if (text == "CAN/LIN") {
		ui.treetrace->setColumnCount(initialHeader.count()); //set column number
		ui.treetrace->setHeaderLabels(initialHeader);        //set header labels
		CurrentHeader = initialHeader;                       //save current header for data match
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
	adjust_filter_buttons();
}

void QtWidgetsApplication1::ButtonSearchClicked()
{
	State current_state = this->state_manager.current_state();
	ui.pushButton_search->setFocus();
	if (current_state == State::START || current_state == State::RESUMED) {
		calc_thread->pauseThread();
		timer->stop();
		last_status = "PAUSED";
		state_manager.changeState(State::PAUSE);
	}
	show_fullpage_with_findings();
	updateToolbar();
	updateContinuousProgress();
	ui.treetrace->setFocus();
}


void QtWidgetsApplication1::dustbin()
{
	updateProgressRunStatus();
	updateProgressTimer();
	calc_thread->clear_items_queue();
}

void QtWidgetsApplication1::updateToolbar()
{
	ui.actionmode->setVisible(false);
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
	if (display_mode == DISPLAY_MODE::UPDATE)
		display_mode = DISPLAY_MODE::APPEND;
	else
		display_mode = DISPLAY_MODE::UPDATE;
	updateToolbar();
	update_tracewindow();
}

void QtWidgetsApplication1::update_tracewindow()
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "======= UPDATE_TRACEWINDOW =======");
	//LOGGER_INFO(log_, "LAST STATUS -> {}", last_status.toStdString());
	//m_mutex.lock();
	int queue_size = shown_queue.size() - padding;
	int queue_backup_size = full_queue_stream.size() - padding;
	qDebug() << "FULL -> " << queue_size << "BACKUP -> " << queue_backup_size;
	//LOGGER_INFO(log_, "FULLSIZE -> {}, BACKUP_SIZE -> {}", queue_size, queue_backup_size);

	QTreeWidget* tree = ui.treetrace;
	QTreeWidgetItem* invisible_root_item = tree->invisibleRootItem();

	QSize itemSize;
	itemSize = getItemSize(invisible_root_item, 0, ui.treetrace->font());
	int v_height = ui.treetrace->viewport()->height();
	int window_capacity = visible_height / itemSize.height() - 1;

	visible_height = v_height;
	item_height = itemSize.height();
	item_width = itemSize.width();
	//LOGGER_INFO(log_, "UPDATE_TRACEWINDOW CAPACITY -> {}", window_capacity);
	//LOGGER_INFO(log_, "PAGE CAPACITY -> {}", page_capacity);

	int tree_count = ui.treetrace->topLevelItemCount();
	//LOGGER_INFO(log_, "TREE COUNT -> {}", tree_count);

	if (tree_count == 0) {
		//LOGGER_INFO(log_, "==== EMPTY TREE ====");
		fill_empty_tree(window_capacity);
	}
	else if (tree_count > 0 && tree_count < page_capacity) {
		//LOGGER_INFO(log_, "==== HALF TREE ====");
		fill_partial_tree(page_capacity);
	}
	else {
		//LOGGER_INFO(log_, "==== REDRAW TREE ====");
		refresh_full_tree(window_capacity);
	}
	//m_mutex.unlock();
	ui.treetrace->scrollToBottom();
	//ui.treetrace->header()->setSortIndicator(0, Qt::AscendingOrder);
	//LOGGER_INFO(log_, "======= END of update_tracewindow =======");
	//LOGGER_INFO(log_, "\n");
}

void QtWidgetsApplication1::refresh_full_tree(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "==== REFRESH_FULL_TREE ====");
	draw_trace_window(capacity);
	//LOGGER_INFO(log_, "==== END OF REFRESH_FULL_TREE ====");
}

void QtWidgetsApplication1::draw_trace_window(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");
	//LOGGER_INFO(log_, "DRAW TRACE WINDOW -> {}", capacity);

	int queue_size = shown_queue.size() - padding;
	//LOGGER_INFO(log_, "DRAW QUEUE SIZE -> {}", queue_size);
	//LOGGER_INFO(log_, "PADDING -> {}", padding);

	if (queue_size <= 0) return;

	QTreeWidget* tree = ui.treetrace;
	TraceTreeWidgetItem* it = nullptr;
	int changes = std::min(queue_size, capacity);
	//LOGGER_INFO(log_, "DRAW CHANGES -> {}", changes);
	if (changes) {
		int count = tree->topLevelItemCount();
		//LOGGER_INFO(log_, "TREE COUNT -> {}", count);
		for (int i = 0; i < changes; i++) {
			int idx = queue_size - changes + i;
			//LOGGER_INFO(log_, "DRAW ITEM -> {}", idx);
			bool bchanged = false;
			it = shown_queue.at(idx);
			if (filter_pass_item(it)) {
				if (count > 0)
				{
					int item_idx = count - changes + i;
					TraceTreeWidgetItem* lastItem = (TraceTreeWidgetItem*)tree->topLevelItem(item_idx);
					for (int k = 0; k < it->columnCount(); k++) {
						lastItem->setText(k, it->text(k));
						lastItem->setDirty(true);
						lastItem->inc_ref();
						bchanged = true;
					}
					if (bchanged) {
						QStringList original_data = lastItem->get_original_data();
						int ref = lastItem->get_ref();
						//LOGGER_INFO(log_, "REF COUNTER -> {}", ref);
						//qDebug() << "REF COUNTER" << ref;
					}
				}
			}
		}
	}
}

void QtWidgetsApplication1::fill_up_to_count(int count)
{
	int queue_size = shown_queue.size() - padding;
	if (queue_size <= 0) return;

	QTreeWidget* tree = ui.treetrace;
	TraceTreeWidgetItem* it = nullptr;
	int tree_count = tree->topLevelItemCount();
	int gap = count - tree_count;
	if (gap <= 0) return;
	int changes = std::min(queue_size, gap);
	if (changes) {
		qDebug() << "UPTO PAGE CHANGES ->" << changes;
		for (int i = 0; i < changes; i++) {
			int idx = queue_size - changes + i;
			it = shown_queue.at(idx);
			if (it != nullptr && filter_pass_item(it)) {
				ui.treetrace->addTopLevelItem(it);
			}
		}
	}
}

void QtWidgetsApplication1::fill_partial_tree(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");
	//m_mutex.lock();
	int queue_size = shown_queue.size() - padding;
	if (queue_size <= 0) {
		//m_mutex.unlock();
		return;
	}

	QTreeWidget* tree = ui.treetrace;
	int tree_count = tree->topLevelItemCount();
	int gap = capacity - tree_count;
	qDebug() << "FILL PARTIAL UP TO -> " << capacity;
	//LOGGER_INFO(log_, "GAP -> {}", gap);
	if (gap <= 0) {
		//m_mutex.unlock();
		return;
	}
	int changes = std::min(queue_size, gap);
	//LOGGER_INFO(log_, "PARTIAL CHANGES -> {}", changes);

	if (changes > 0) {
		int queue_original_size = shown_queue.size();
		qDebug() << "ORIGINAL QUEUE SIZE -> " << queue_original_size;
		QList<QTreeWidgetItem*> item_list;
		for (int i = 0; i < changes; i++) {
			int idx = queue_size - changes + i;
			//LOGGER_INFO(log_, "PARTIAL INDEX -> {}", idx);
			if (idx >= 0) {
				TraceTreeWidgetItem* it0 = read_item_from_queue(idx);
				if (ui.treetrace->indexOfTopLevelItem(it0) != -1) { continue; }
				if (filter_pass_item(it0)) {
					ui.treetrace->addTopLevelItem(it0);
					tree_count = tree->topLevelItemCount();
					qDebug() << "TREE COUNT AFTER PARTIAL INSERT ->" << tree_count;
				}
			}
		}
	}
	//m_mutex.unlock();
	//LOGGER_INFO(log_, "END OF PARTIAL FILLING");
	//LOGGER_INFO(log_, "====================");
}

void QtWidgetsApplication1::fill_empty_tree(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");

	int queue_size = shown_queue.size() - padding;
	//LOGGER_INFO(log_, "FULL QUEUE SIZE IN EMPTY FUNC -> {}", queue_size);
	if (queue_size <= 0) return;

	int changes = std::min(queue_size, capacity);
	if (changes > 0) {
		changes = 1;
		//LOGGER_INFO(log_, "EMPTY CHANGES -> {}", changes);
		for (int i = 0; i < changes; i++) {
			TraceTreeWidgetItem* it = nullptr;
			int idx = queue_size - changes + i;
			it = this->read_item_from_queue(idx);
			//LOGGER_INFO(log_, "READY INDEX -> {}", idx);
			if (it != nullptr) {
				//LOGGER_INFO(log_, "ITEM INDEX -> {}", idx);
				if (filter_pass_item(it)) {
					ui.treetrace->addTopLevelItem(it);
				}
			}
		}
	}
	//LOGGER_INFO(log_, "EXITING EMPTY TREE.");
}

void QtWidgetsApplication1::trace_scroll_changed(int value)
{
	QScrollBar* scrollBar = ui.treetrace->verticalScrollBar();
	if (scrollBar->value() == scrollBar->maximum()) { return; }
	State current_state = this->state_manager.current_state();
	if (current_state == State::PAUSE) { return; }
	if (current_state == State::START || current_state == State::RESUMED) {
		auto log_ = GETLOG("WORKFLOW");
		calc_thread->pauseThread();
		timer->stop();
		show_fullpage();
		last_status = "PAUSED";
		state_manager.changeState(State::PAUSE);
	}
	updateToolbar();
	updateContinuousProgress();
}

void QtWidgetsApplication1::freeze_treetrace_items(int ncount)
{
	auto log_ = GETLOG("WORKFLOW");
	qDebug() << "SHOW_FULLPAGE...";
	show_fullpage();
	qDebug() << "END OF SHOW_FULLPAGE";
	/*
		return;
		if (frozen) return;
		int size = shown_queue.size() - padding;
		//LOGGER_INFO(log_, "SIZE -> {}", size);
		if (size <= 0) return;
		//LOGGER_INFO(log_, "TARGET -> {}", ncount);
		ui.treetrace->clear();
		int total = std::min(size, ncount) - 5;
		//LOGGER_INFO(log_, "TOTAL -> {}", total);
		QTreeWidgetItem* it = nullptr;
		QList<QTreeWidgetItem*> item_list;
		item_list.clear();
		for (int i = 0; i < total; i++)
		{
			int idx = size - i;
			//LOGGER_INFO(log_, "INDEX -> {}", idx);
			it = shown_queue.at(idx);
			if (it != nullptr && filter_pass_item(it)) {
				item_list.append(it);
			}
		}
		if (item_list.size())
		{
			ui.treetrace->addTopLevelItems(item_list);
			int frozen_size = item_list.size();
			//LOGGER_INFO(log_, "FRONZEN -> {}", frozen_size);
		}
	*/
}

void QtWidgetsApplication1::adjust_filter_buttons()
{
	QTreeWidget* tree = ui.treetrace;
	QHeaderView* header = tree->header();
	if (!headerButtonList.count())
		return;
	for (int i = 0; i < header->count(); i++) {
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
	//qDebug() << "reset_all_filters";
	if (!new_filters.isEmpty()) {
		for (auto it = new_filters.begin(); it != new_filters.end(); ++it) {
			int col_index = it.value()[0].toInt();
			headerButtonList[col_index]->setIcon(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"));
		}
		new_filters.clear();
	}
	filter->new_checks.clear();
	for (TraceTreeWidgetItem* it : this->filtered_queue) {
		delete it;
		it = nullptr;
	}
	filtered_queue.clear();
}

bool QtWidgetsApplication1::filter_pass_item(QTreeWidgetItem* it)
{
	if (it == NULL) return false;

	bool matched = true;
	QString uuidstr = ((TraceTreeWidgetItem*)it)->getUUID();
	if (passed_uuid_set.contains(uuidstr))
		return true;

	if (calc_thread->isRUN() && it) {
		matched = filter_run_pass_item_without_children(it);
		if (!matched) { return false; }
	}

	if (new_filters.isEmpty())
	{
		passed_uuid_set.insert(uuidstr);
		return true;
	}
	for (const auto& key : new_filters.keys()) {
		QList<QVariant> vlist = new_filters.value(key);
		int col_index = vlist[0].toInt();
		int length = vlist.length();
		QList<QVariant> slice = vlist.mid(1, length);
		if (!slice.contains(it->text(col_index)))
		{
			//for (const QVariant& value : slice) {
			//	qDebug() << "COL ->" << col_index << "Text ->" << it->text(col_index) << "Not Matched ->" << value;
			//}
			matched = false;
		}
	}
	if (matched) {
		passed_uuid_set.insert(uuidstr);
	}
	return matched;
}

bool QtWidgetsApplication1::filter_run_pass_item_without_children(QTreeWidgetItem* it)
{
	//if (calc_thread->isRUN() && it) {
	//	try
	//	{
	//		int children_count = it->childCount();
	//		if (children_count > 0) {
	//			return false;
	//		}
	//		else return true;
	//	}
	//	catch (...)
	//	{
	//	}
	//}
	//else
	return true;
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

TraceTreeWidgetItem* QtWidgetsApplication1::read_item_from_queue(int index)
{
	auto log_ = GETLOG("WORKFLOW");
	TraceTreeWidgetItem* item = nullptr;
	item = this->shown_queue.at(index);
	//LOGGER_INFO(log_, "ITEM CONTENT -> {}", item->text(0).toStdString());
	return item;
}

TraceTreeWidgetItem* QtWidgetsApplication1::read_item_from_dumb(int index)
{
	/* This function is a mock one to generate 10 rows data
	*  to the tracewidget to show. 2024-10-21 15:27:04
	*/

	QStringList str_pdu = {};
	for (int i = 0; i < 10; i++) {
		QString str = QString("COL: %1").arg(i);
		str_pdu.append(str);
	}
	TraceTreeWidgetItem* item = new TraceTreeWidgetItem(str_pdu);
	return item;
}

void QtWidgetsApplication1::construct_page_data(TraceTreeWidgetItem* item)
{
	/* This function construct the current page data.
	*  in order to show the tree in the pause or stopped state.
	*  in the class there should be a queue of items and limit the size
	*  to the *count_per_page*.
	*  because the queue stores the pointers, so this queue do not
	*  delete the item manually.
	*/
	//filtered_queue.enqueue(item->clone());
	//if (filtered_queue.size() > count_per_page)
		//filtered_queue.dequeue();
}

void QtWidgetsApplication1::show_fullpage()
{
	if (timer_isRunning) return;
	timer_isRunning = true;

	qDebug() << "SHOW_FULLPAGE()";

	ui.treetrace->clear();
	passed_uuid_set.clear();
	safe_clear_trace();
	updateComoboPage();
	QQueue<QTreeWidgetItem*> baseQueue = get_filtered_queue_tail();
	ui.treetrace->addTopLevelItems(baseQueue);
	timer_isRunning = false;
}

QQueue<QTreeWidgetItem*> QtWidgetsApplication1::get_filtered_queue_front()
{
	QQueue<QTreeWidgetItem*> queue_;
	int i = (paused_instant_index > count_per_page) ? paused_instant_index - count_per_page : 0;
	qDebug() << "PAUSED_INSTANT_INDEX ->" << paused_instant_index;
	for (; i < paused_instant_index; i++)
	{
		TraceTreeWidgetItem* traceItem = full_queue_stream.at(i)->clone();
		if (filter_pass_item(traceItem)) queue_.enqueue(traceItem);
	}
	qDebug() << "QUEUE_ SIZE ->" << queue_.size();
	return queue_;
}

QQueue<QTreeWidgetItem*> QtWidgetsApplication1::get_filtered_queue_tail()
{
	QQueue<QTreeWidgetItem*> queue_;
	int count = 0;
	for (int i = 1; i < paused_instant_index; i++)
	{
		int idx = paused_instant_index - i;
		TraceTreeWidgetItem* traceItem = full_queue_stream.at(idx)->clone();
		if (filter_pass_item(traceItem)) {
			queue_.enqueue(traceItem);
			count++;
		}
		if (count >= count_per_page) { break; }
	}
	qDebug() << "From Tail Count ->" << queue_.size();
	return queue_;
}

void QtWidgetsApplication1::restore_full_queue()
{
	if (!shown_queue.isEmpty())
		shown_queue.clear();

	int i = paused_instant_index >= this->count_per_page ? paused_instant_index - count_per_page : 0;
	for (; i < this->paused_instant_index; i++) {
		TraceTreeWidgetItem* it = full_queue_stream.at(i)->clone();
		shown_queue.append(it);
	}
	qDebug() << "SHOWN Size ->" << shown_queue.size();
}

void QtWidgetsApplication1::safe_clear_trace()
{
	passed_uuid_set.clear();
	clear_queue(shown_queue);
}

QString QtWidgetsApplication1::previous_state()
{
	return this->state_manager.previous_state();
}

void QtWidgetsApplication1::print_item_queue(QQueue<TraceTreeWidgetItem*> q)
{
	if (timer_isRunning) return;
	timer_isRunning = true;
	qDebug() << "PRINT QUEUE SIZE -> " << q.size();
	if (q.isEmpty()) {
		timer_isRunning = true;
		return;
	}
	for (const TraceTreeWidgetItem* item : q) {
		QString source = item->getSource();
		QString uuid = item->getUUID();
		qDebug() << "PRINT ITEM -> " << "UUID -> " << uuid << "SOURCE -> " << source;
	}
	timer_isRunning = true;
}

void QtWidgetsApplication1::clear_queue(QQueue<TraceTreeWidgetItem*>& queue)
{
	queue.clear();
}

void QtWidgetsApplication1::update_latest_index(uint64_t index)
{
	paused_instant_index = index;
	auto log_ = GETLOG("WORKFLOW");
	//qDebug() << "LATEST INDEX ->" << index;
	current_page_index = paused_instant_index / count_per_page;
	current_item_index = paused_instant_index % count_per_page;
	if (paused_instant_index >= maximum_total) {
		//LOGGER_INFO(log_, "PAUSED INDEX -> {}", paused_instant_index);
		//LOGGER_INFO(log_, "SHOWN_QUEUE MEMORY -> {}", sizeof(TraceTreeWidgetItem) * shown_queue.size());
		//LOGGER_INFO(log_, "FULL_QUEUE_STREAM MEMORY -> {}", sizeof(TraceTreeWidgetItem) * full_queue_stream.size());
		//LOGGER_INFO(log_, "FILTERED_QUEUE MEMORY -> {}", sizeof(TraceTreeWidgetItem) * filtered_queue.size());
	}
}

bool QtWidgetsApplication1::eventFilter(QObject* obj, QEvent* event) {
	if (obj == findChild<QPlainTextEdit*>()) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
				emit buttonClicked();
				return true;
			}
			if (keyEvent->key() == Qt::Key_Tab) {
				ui.pushButton_search->setFocus();
				return true;
			}
			if (keyEvent->key() == Qt::Key_Escape)
			{
				ui.treetrace->setFocus();
				return true;
			}
		}
	}
	if (obj == findChild<QPushButton*>()) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Tab) {
				ui.treetrace->setFocus();
				return true;
			}
		}
	}
	if (obj == ui.treetrace && event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Up) {
			qDebug() << "UP KEY PRESSED";
			QTreeWidgetItem* current = ui.treetrace->currentItem();
			if (current) {
				highlight_previous_item(current);
			}
			return true;
		}
		if (keyEvent->key() == Qt::Key_Down) {
			qDebug() << "DOWN KEY PRESSED";
			QTreeWidgetItem* current = ui.treetrace->currentItem();
			if (current) {
				highlight_next_item(current);
			}
			return true;
		}
		if (keyEvent->key() == Qt::Key_Tab) {
			QTreeWidgetItem* current = ui.treetrace->currentItem();
			if (current) {
				int nchild = current->childCount();
				if (nchild > 0) {
					if (current->isExpanded())
						collapse_item(current);
					else
						extract_item(current);
				}
				else { ui.mysearch->setFocus(); ui.mysearch->selectAll();}
			}
			else {
				ui.mysearch->setFocus();
				ui.mysearch->selectAll();
			}
			return true;
		}
		State state = state_manager.current_state();
		switch (keyEvent->key())
		{
		case Qt::Key_S:
			if (state == State::COMPLETE || state == State::INIT || state == State::STOPPED)
			{
				this->startTrace();
			}
			break;
		case Qt::Key_T:
		case Qt::Key_D:
			this->stopTrace();
			break;
		case Qt::Key_P:
		{
			find_direction = 1; 
			ButtonSearchClicked(); 
		}
			break;
		case Qt::Key_N:
		{
			find_direction = 0;
			ButtonSearchClicked();
		}
		break;
		case Qt::Key_A:
			this->pauseTrace();
			break;
		case Qt::Key_R:
			if (calc_thread->isPAUSED()) {
				this->pauseTrace();
			}
			break;
		case Qt::Key_Slash:
			ui.mysearch->setFocus();
			ui.mysearch->selectAll();
			break;
		case Qt::Key_F:
			if (keyEvent->modifiers() & Qt::ControlModifier)
			{
				ui.mysearch->setFocus();
				ui.mysearch->selectAll();
			}
			break;
		default:
			;
		}
	}
	return QWidget::eventFilter(obj, event);
}


void QtWidgetsApplication1::construct_searching_string()
{
}

void QtWidgetsApplication1::help_usage()
{
	qDebug() << "help help help";
	construct_usage();
	if (usagedialog == nullptr)
	{
		usagedialog = new UsageDialog();
		usagedialog->setModal(false);
		usagedialog->setUsage(USAGE);
	}
	usagedialog->show();
}

void QtWidgetsApplication1::construct_usage()
{
	USAGE.clear();
	USAGE << "GLOBAL:";
	USAGE << "    CTRL-H\tShow this usage.";
	USAGE << "    TAB\tSwitch cursor among trace and search.";
	USAGE << "TRACE:";
	USAGE << "    S\tStart trace.";
	USAGE << "    A\tToggle pause trace.";
	USAGE << "    D\tStop trace.";
	USAGE << "    TAB\texpand/collapse item if it has children.";
	USAGE << "    CTRL-F\tSwitch cursor to search edit.";
	USAGE << "    Up\tPrevious item.";
	USAGE << "    Down\tNext item.";
	USAGE << "";
	USAGE << "SEARCH EDIT:";
	USAGE << "    Enter\tFind the items accordingly.";
	USAGE << "SEARCH Syntax:";
	USAGE << "    Signal:\t sig:\<signal_name\>";
	USAGE << "    Column:\t \<column_name\>=\<value\>";
	USAGE << "    All conditions connected with ;";
	USAGE << "Examples:";
	USAGE << "    sig:IEvtDiscardRate;sig:IBatTem";
	USAGE << "    ID=504;Dir=tx";
}

void QtWidgetsApplication1::updateComoboPage()
{
	ui.comboBox_Page->clear();
	if (paused_instant_index < 0) return;
	max_page_count = paused_instant_index / count_per_page + 1;
	int fs = filtered_size();
	if (fs) max_page_count = fs / count_per_page + 1;
	for (int i = 1; i <= max_page_count; i++) {
		ui.comboBox_Page->addItem(QString::number(i));
	}
}

void QtWidgetsApplication1::ButtonFirstClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int current_page = ui.label_Current->text().toInt(&ok);
	int target_page = 1;
	qDebug() << "TARGET_PAGE -> " << target_page;
	if (current_page == target_page) return;
	show_fullpage_with_index(target_page);
	ui.label_Current->setText(QString::number(target_page));
}

void QtWidgetsApplication1::ButtonPreviousClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int current_page = ui.label_Current->text().toInt(&ok);
	int target_page = (current_page - 1) < 1 ? 1 : current_page - 1;
	qDebug() << "TARGET_PAGE -> " << target_page;
	if (current_page == target_page) return;
	show_fullpage_with_index(target_page);
	ui.label_Current->setText(QString::number(target_page));
}

void QtWidgetsApplication1::ButtonNextClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int current_page = ui.label_Current->text().toInt(&ok);
	int target_page = (current_page + 1) > max_page_count ? max_page_count : current_page + 1;
	if (target_page > 0) {
		qDebug() << "TARGET_PAGE -> " << target_page;
		if (current_page == target_page) return;
		show_fullpage_with_index(target_page);
		ui.label_Current->setText(QString::number(target_page));
	}
}

void QtWidgetsApplication1::ButtonLastClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int current_page = ui.label_Current->text().toInt(&ok);
	int target_page = max_page_count;
	if (current_page == target_page) return;
	if (target_page > 0) {
		qDebug() << "TARGET_PAGE -> " << target_page;
		if (current_page == target_page) return;
		show_fullpage_with_index(target_page);
		ui.label_Current->setText(QString::number(target_page));
	}
}

void QtWidgetsApplication1::ButtonGotoClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int target_page = ui.comboBox_Page->currentText().toInt(&ok);
	int current_page = ui.label_Current->text().toInt(&ok);
	qDebug() << "TARGET_PAGE -> " << target_page;
	if (current_page == target_page) return;
	show_fullpage_with_index(target_page);
	ui.label_Current->setText(QString::number(target_page));
}

void QtWidgetsApplication1::form_conditions_simple(QString findstr)
{
	last_findstring = findstr;
	QStringList full_items = findstr.split(QRegExp("[,;|]"), QString::SkipEmptyParts);
	list_map.clear();
	full_search = false;
	for (const QString& item : full_items) {
		qDebug() << item.trimmed();
		QSet<QString> stringSet;
		stringSet << "sig" << "signal";
		// column mode
		if (item.contains("=")) {
			QStringList item_map = item.split("=", QString::SkipEmptyParts);
			qDebug() << "ITEM MAP SIZE ->" << item_map.size();
			if (item_map.size() != 2)
				continue;
			QString k = item_map[0].trimmed();
			QString v = item_map[1].trimmed().toLower();
			int nk = -1;
			for (int i = 0; i < CurrentHeader.size(); ++i) {
				if (CurrentHeader[i].toLower() == k.toLower()) {
					list_map[i] << v;
					condition_type = condition_type > 1 ? 3 : 1;
					break;
				}
			}
			if (k.contains(":")) {
				// signal - "sig:signal_name=value"
				QStringList sig_list = k.split(":", QString::SkipEmptyParts);
				if (sig_list.size() != 2)
					continue;
				QString s_flag = sig_list[0].trimmed().toLower(); // sig or signal
				QString s_name = sig_list[1].trimmed().toLower(); // signal name
				if (stringSet.contains(s_flag)) {
					list_map[0] << s_name;
					full_search = true;
					condition_type = (condition_type % 2) ? 3 : 2;
				}
			}
		}
		else if (item.contains(":")) {
			// signal mode
			QStringList sig_list = item.split(":", QString::SkipEmptyParts);
			if (sig_list.size() != 2)
				continue;
			QString s_flag = sig_list[0].trimmed().toLower(); // sig or signal
			QString s_name = sig_list[1].trimmed().toLower(); // signal name
			if (stringSet.contains(s_flag)) {
				list_map[0] << s_name;
				full_search = true;
			}
		}
		else { 
			// no equal signs or no signals
			for (int i = 0; i < CurrentHeader.size(); ++i) {
				list_map[i] << item.trimmed().toLower();
				condition_type = 99; // magic code full search.
			}
		}
	}
}

#include "lexer.h"

bool QtWidgetsApplication1::parser2condition(QList<Token> token_list)
{
	bool bok = false;
	return bok;
}

void QtWidgetsApplication1::form_conditions_compiler(QString findstr)
{
	QByteArray byteArray = findstr.toUtf8();
	const char* cStr = byteArray.constData();
	Lexer lex(cStr);
	QList<Token> token_list;
	for (auto token = lex.next();
		!token.is_one_of(Token::Kind::End, Token::Kind::Unexpected);
		token = lex.next())
	{
		token_list.append(token);
	}
	for (int i = 0; i < token_list.size(); i++)
	{
		QString s;
		Token token = token_list.at(i);
		s += QString::fromStdString(token.lexeme()) + "|";
		s += QString::fromStdString(token.kind_name()) + "|\n";
		qDebug() << "Lexer OUTPUT -> " << s;
	}
	parser2condition(token_list);
}

bool QtWidgetsApplication1::findItem(QTreeWidgetItem* item, int col,
	const QStringList& target, QSet<QTreeWidgetItem*>& hiddens,
	QQueue<Found_Item*>& founds, int level)
{
	int child = item->childCount();
	QString s = item->text(col).toLower();
	if (!child && level)
	{
		qDebug() << "Hidden ->" << s;
		hiddens.insert(item);
		QList<QTreeWidgetItem*> siblings;
		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem) {
			for (int i = 0; i < parentItem->childCount(); ++i) {
				QTreeWidgetItem* sibling = parentItem->child(i);
				if (sibling != item) {
					siblings.append(sibling);
					hiddens.insert(sibling);
				}
			}
		}
		if (!siblings.isEmpty()) {
			for (QTreeWidgetItem* sib : siblings) {
				for (QString targetText : target) {
					if (sib->text(col).toLower() == targetText)
					{
						bool contained = false;
						for (Found_Item* it : founds) {
							if (it->item == sib) { contained = true; break; }
						}
						if (!contained) {
							Found_Item* found = new Found_Item();
							found->item = sib;
							found->col = col;
							founds.enqueue(found);
						}
						if (hiddens.contains(sib))
							hiddens.remove(sib);
					}
				}
			}
		}
	}
	for (QString targetText : target) {
		if (item->text(col).toLower() == targetText)
		{
			//if (!founds.contains(item)) {
			//  founds.enqueue(item);
			//}
			for (Found_Item* t : founds)
			{
				if (t->item->text(col).toLower() == targetText) {}
			}
			Found_Item* x = new Found_Item();
			x->item = item;
			x->col = col;
			founds.enqueue(x);
			if (hiddens.contains(item))
				hiddens.remove(item);
			return true;
		}
	}
	for (int i = 0; i < child; ++i) {
		if (findItem(item->child(i), col, target, hiddens, founds, level + 1)) {
			return true;
		}
	}
	return false;
}


QQueue<Found_Item*> QtWidgetsApplication1::remove_duplicates(QQueue<Found_Item*> queue) {
	QQueue<Found_Item*> result;

	while (!queue.isEmpty()) {
		Found_Item* value = queue.dequeue();
		bool omit = false;
		for (Found_Item* t : result) {
			if (t->item == value->item) {
				omit = true;
				break;
			}
		}
		if (!omit && !result.contains(value)) {
			result.enqueue(value);
		}
	}
	return result;
}
void QtWidgetsApplication1::show_fullpage_with_findings()
{
	QString search_str = ui.mysearch->toPlainText().trimmed();

	// todo: refactoring the simple algorithm to compiler constrction.
	// 2024-11-01 15:41:19
	// form_conditions_compiler(search_str);
	if (search_str.length() <= 0)
	{
		reset_find_status();
	}
	if (current_find_status() && foundcount() && search_str == last_findstring) {
		// todo: if the current find status is "in find" already. 
		// find the current_find and hilight it.
		if (current_find == nullptr && !found_queue.isEmpty())
		{
			int i = 0;
			prev_find = current_find;
			current_find = found_queue.at(i);
		}
		else if (current_find != nullptr && !found_queue.isEmpty())
		{
			int i = found_queue.indexOf(current_find);
			if (find_direction) // up
			{
				i = (i <= 0) ? found_queue.size() - 1 : i - 1;
			}
			else // down
			{
				i = (i >= found_queue.size() - 1) ? 0 : i + 1;
			}
			prev_find = current_find;
			current_find = found_queue.at(i);
		}
		if (current_find) {
			current_find->highlight(ui.treetrace);
		}
		if (prev_find)
			prev_find->dehighlight();
		return;
	}
	if (found_queue.isEmpty() || search_str != last_findstring)
		reset_find_status();
	enter_find_status();
	form_conditions_simple(search_str);
	resetInvisibles();

	QTreeWidgetItem* invisible_root_item = ui.treetrace->invisibleRootItem();
	int child_count = invisible_root_item->childCount();
	full_search = 1; // the global switch, always full_search
	for (int i = 0; i < child_count; ++i) {
		QTreeWidgetItem* item = invisible_root_item->child(i);
		bool hidden = false;
		for (const int& key : list_map.keys()) {
			QStringList values = list_map.value(key);
			if (full_search) { 
				// findItem is the very key function, should be refactored.
				bool matched = false;
				matched = findItem(item, key, values, invisibles, found_queue);
				hidden = !matched;
			}
			else {
				if (!values.contains(item->text(key).toLower())) {
					hidden = true;
					continue;
				}
				else {
					bool contained = false;
					for (Found_Item* it : found_queue)
					{
						if (it->item == item) { contained = true; break; }
					}
					if (!contained) {
						Found_Item* found = new Found_Item();
						found->item = item;
						found->col = key;
						found_queue.enqueue(found);
					}
				}
			}
		}
		//item->setHidden(hidden);
	}
	//for (QTreeWidgetItem* h : invisibles) {
	//	h->setHidden(true);
	//}
	//for (QTreeWidgetItem* x : found_queue) {
	//	qDebug() << "Found Item ->" << x->text(0);
	//}
	if (!found_queue.isEmpty()) {
		found_queue = remove_duplicates(found_queue);
		prev_find = current_find;
		current_find = found_queue.at(0);
		//ui.treetrace->setCurrentItem(current_find->item);
		current_find->highlight(ui.treetrace);
		if (prev_find)
			prev_find->dehighlight();
	}
	updateContinuousProgress();
}

void QtWidgetsApplication1::resetInvisibles()
{
	for (QTreeWidgetItem* h : invisibles) {
		h->setHidden(false);
	}
	invisibles.clear();
}

void QtWidgetsApplication1::show_fullpage_with_index(int index)
{
	// index is the page index  starting from 1
	if (timer_isRunning) return;
	timer_isRunning = true;
	qDebug() << "SHOW_FULLPAGE_WITH_INDEX";
	int page_index = index - 1;
	ui.treetrace->clear();
	passed_uuid_set.clear();
	safe_clear_trace();

	QQueue<TraceTreeWidgetItem*>* search_queue;
	if (filtered_queue.isEmpty())
		search_queue = &full_queue_stream;
	else
		search_queue = &filtered_queue;

	if (search_queue->isEmpty()) return;

	QQueue<QTreeWidgetItem*> baseQueue;
	int pi = (page_index < 0) ? 0 : page_index;
	int i = count_per_page * pi;
	int fs = filtered_size();
	int max_index = 0;
	if (fs)
		max_index = std::min(paused_instant_index, fs);
	else
	{
		max_index = paused_instant_index;
	}
	int stop = (count_per_page * index > max_index) ? max_index : i + count_per_page;
	if (fs && fs < count_per_page) stop = fs;
	qDebug() << "INDEX FROM ->" << i << "TO ->" << stop;
	for (; i < stop; i++)
	{
		TraceTreeWidgetItem* traceItem = search_queue->at(i)->clone();
		baseQueue.enqueue(traceItem);
	}
	qDebug() << "BASEQUEUE SIZE ->" << baseQueue.size();
	ui.treetrace->addTopLevelItems(baseQueue);
	ui.treetrace->scrollToBottom();
	this->updateContinuousProgress();
	timer_isRunning = false;
}

void QtWidgetsApplication1::highlight_previous_item(QTreeWidgetItem* item)
{
	QTreeWidgetItem* parent = item->parent();
	QTreeWidgetItem* prevSibling;
	if (parent) {
		prevSibling = parent->child(parent->indexOfChild(item) - 1);
	}
	else {
		QTreeWidget* treeWidget = item->treeWidget();
		prevSibling = treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(item) - 1);
		if (prevSibling) {
			while (prevSibling && prevSibling->isHidden()) {
				prevSibling = treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(prevSibling) - 1);
			}
			if (prevSibling) {
				ui.treetrace->setCurrentItem(prevSibling);
			}
		}
	}
}

void QtWidgetsApplication1::highlight_next_item(QTreeWidgetItem* item)
{
	QTreeWidgetItem* parent = item->parent();
	QTreeWidgetItem* nextSibling;
	if (parent) {
		nextSibling = parent->child(parent->indexOfChild(item) + 1);
	}
	else {
		QTreeWidget* treeWidget = item->treeWidget();
		nextSibling = treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(item) + 1);
		if (nextSibling) {
			while (nextSibling && nextSibling->isHidden()) {
				nextSibling = treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(nextSibling) + 1);
			}
			if (nextSibling) {
				ui.treetrace->setCurrentItem(nextSibling);
			}
		}
	}
}

void QtWidgetsApplication1::extract_item(QTreeWidgetItem* item)
{
	item->setExpanded(true);
	for (int i = 0; i < item->childCount(); i++) {
		extract_item(item->child(i));
	}
}

void QtWidgetsApplication1::collapse_item(QTreeWidgetItem* item)
{
	item->setExpanded(false);
	for (int i = 0; i < item->childCount(); i++) {
		collapse_item(item->child(i));
	}
}

void QtWidgetsApplication1::updateHeaderStyle(QTreeWidgetItem* item) {
	//ui.treetrace->setStyleSheet(treewidget_header_style);
}
