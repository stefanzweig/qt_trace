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

#include "TraceTreeWidgetItem.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "my_spdlog.h"
#include "multiThread.h"
#include "columnfilter.h"
#include "LineEditFilter.h"

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

	state_manager.changeState(State::COMPLETE);
	state_manager.printHistory();

	print_item_queue(shown_queue);
	print_item_queue(full_queue_stream);

	clear_queue(shown_queue);
	clear_queue(full_queue_stream);
	clear_queue(current_page_queue);

	print_item_queue(shown_queue);
	print_item_queue(full_queue_stream);

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

	QList<QVariant> modules = settings.value("app/modules").toList();
	qDebug() << "Modules -> " << modules;
	for (QVariant m : modules) {
		monitor_modules.append(m.toString());
	}
}

void QtWidgetsApplication1::updateProgressRunStatus()
{
	QString runStatus = "";
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
		//+ ". CAN PDUs: " + QString::number(canparser_count)
		+". LIN Messages: " + QString::number(lin_messages_count)
		+ ". Total: " + QString::number(can_messages_count
			//+ canparser_count 
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
		page_index = paused_instant_index / count_per_page + 1;
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

	datachoice = ui.comboBox;
	datachoice->setEnabled(false);
	connect(datachoice, &QComboBox::currentTextChanged, this, &QtWidgetsApplication1::ChangeHeader);
	connect(ui.treetrace->verticalScrollBar(), &QScrollBar::valueChanged, this, &QtWidgetsApplication1::trace_scroll_changed);
	setWindowIcon(QIcon(":/QtWidgetsApplication1/res/spreadsheet.png"));
	updateToolbar();

	// mysearch
	LineEditFilter* lineeditfilter = new LineEditFilter();
	ui.mysearch->installEventFilter(lineeditfilter);
}

void QtWidgetsApplication1::resetStatusBar()
{
	QString strLeftTemplate = "Count: 0";
	QString strRightTemplate = "00:00:00";
	leftLabel = new QLabel(strLeftTemplate, this);
	leftWidget = new QWidget(this);
	leftLayout = new QHBoxLayout(leftWidget);
	leftLayout->addWidget(leftLabel);
	leftLayout->addStretch();

	rightLabel = new QLabel(strRightTemplate, this);
	rightWidget = new QWidget(this);
	rightLayout = new QHBoxLayout(rightWidget);
	rightLayout->addStretch();
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
	// no use so far
}

void QtWidgetsApplication1::onActionTriggered()
{
	qDebug() << tr("�˵����Ѵ�����");
}


void QtWidgetsApplication1::clearance()
{
	calc_thread->full_count_canframes = 0;
	calc_thread->full_count_canparser = 0;

	// todo: how to delete the items in the queue safely?
	// 2024��10��3�� 11:23
	safe_clear_trace();
}

void QtWidgetsApplication1::initialize_new_session()
{
	calc_thread->full_count_canframes = 0;
	calc_thread->full_count_canparser = 0;
	paused_instant_index = -1;
	clear_queue(shown_queue);
	clear_queue(full_queue_stream);
	clear_queue(current_page_queue);
}

bool QtWidgetsApplication1::new_session()
{
	int ncount = ui.treetrace->topLevelItemCount();
	if (!ncount) return true;

	if (showNewSession()) {
		ui.treetrace->clear();
		initialize_new_session();
	}
	return true;
}

void QtWidgetsApplication1::startTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== START TRACE CLICKED ====");
	state_manager.changeState(State::START);

	if (!new_session())
		return;
	start_time = QDateTime::currentDateTime();
	LOGGER_INFO(log_, "==== BEFORE RESUME TRACE ====");
	resumeTrace();
	last_status = "STARTED";
	LOGGER_INFO(log_, "==== AFTER RESUME TRACE ====");
	LOGGER_INFO(log_, "\n");
}

void QtWidgetsApplication1::resumeTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== RESUME TRACE PROCESS ====");

	uint32_t samples = 1000;
	//LOGGER_INFO(log_, "==== mysub_can_frames IS NULL? {} ====", mysub_can_frames == nullptr);
	//LOGGER_INFO(log_, "==== mysub_can_parser IS NULL? {} ====", mysub_can_parser == nullptr);

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
		qRegisterMetaType <linFrame>("linFrame");
	}

	calc_thread->restartThread();
	LOGGER_INFO(log_, "==== CAN SUB SET ====");
	calc_thread->setCanSubscriber(mysub_can_frames, samples, ui.treetrace);
	calc_thread->setCanParserSubscriber(mysub_can_parser, samples, ui.treetrace);
	calc_thread->setLinSubscriber(mysub_lin_frames, samples, ui.treetrace);
	calc_thread->setLinParserSubscriber(mysub_lin_parser, samples, ui.treetrace);
	LOGGER_INFO(log_, "==== THREAD RESTARTED ====");
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
	LOGGER_INFO(log_, "==== END OF STOPPING TRACE ====");
	LOGGER_INFO(log_, "\n");

}

void QtWidgetsApplication1::replayTrace()
{
	// 2024��10��11�� 17:46 �طŹ���
	qDebug() << "REPLAY...";
}

void QtWidgetsApplication1::resume_from_pause_trace()
{
	auto log_ = GETLOG("WORKFLOW");
	if (last_status == "STOPPED" || last_status == "READY") return;
	if (calc_thread->isPAUSED()) {
		LOGGER_INFO(log_, "==== CONTINUE AFTER PAUSE ====");
		frozen = false;
		initial_trace = true;
		state_manager.changeState(State::RESUMED);
		LOGGER_INFO(log_, "==== FROZEN STATUS {} ====", frozen);
		LOGGER_INFO(log_, "==== BEFORE RESUME TRACE ====");
		LOGGER_INFO(log_, "==== QUEUE SIZE BEFORE RESUMING {} ====", shown_queue.size() - padding);
		resumeTrace();
		LOGGER_INFO(log_, "==== AFTER RESUME TRACE ====");
		LOGGER_INFO(log_, "\n");
		return;
	}
}

void QtWidgetsApplication1::pauseTrace()
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== PAUSE BUTTON CLICKED ====");
	if (last_status == "STOPPED" || last_status == "READY") return;

	int backup_count = full_queue_stream.size();
	qDebug() << "FULL_QUEUE_BACKUP SIZE ->" << backup_count;
	if (calc_thread->isPAUSED()) {
		ui.treetrace->clear();
		restore_full_queue();
		resume_from_pause_trace();
		return;
	}
	LOGGER_INFO(log_, "==== PAUSE TRACE ====");
	calc_thread->pauseThread();
	timer->stop();
	qDebug() << "INSTANT INDEX ->" << paused_instant_index;
	LOGGER_INFO(log_, "==== QUEUE SIZE WHEN PAUSE {} ====", shown_queue.size() - padding);
	show_fullpage();
	last_status = "PAUSED";
	state_manager.changeState(State::PAUSE);
	updateToolbar();
	updateContinuousProgress();
	LOGGER_INFO(log_, "==== END OF PAUSE BUTTON CLICKED ====");
	LOGGER_INFO(log_, "\n");
}

void QtWidgetsApplication1::setupTreeTrace()
{
	QTreeWidget* t = ui.treetrace;
	QHeaderView* header = t->header();
	t->setSelectionBehavior(QTreeView::SelectRows);
	t->setFocusPolicy(Qt::WheelFocus);
	t->header()->setHighlightSections(true);
	t->header()->setStretchLastSection(true);
	t->header()->setSortIndicator(0, Qt::AscendingOrder);
	t->setWindowTitle(QObject::tr("CAN Frames"));
	t->setAttribute(Qt::WA_OpaquePaintEvent);
	t->setAttribute(Qt::WA_NoSystemBackground);
	t->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	t->setHeaderLabels(initialHeader);
	t->setSortingEnabled(true);
	t->setColumnWidth(0, 150);
	t->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	t->invisibleRootItem()->setHidden(true);
	t->installEventFilter(this);
	header->setDefaultSectionSize(150);
	header->setSectionResizeMode(QHeaderView::Interactive);
	QFont font("SimSun", 8);
	t->setFont(font);
	initialHeaders();
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
}

void QtWidgetsApplication1::on_pop_to_root(TraceTreeWidgetItem* item)
{
	auto log_ = GETLOG("WORKFLOW");
	timer_isRunning = true;
	if (item != NULL) {
		//m_mutex.lock();
		if (calc_thread->isPAUSED()) {
			TraceTreeWidgetItem* item_backup = (TraceTreeWidgetItem*)item->clone();
			full_queue_stream.enqueue(item_backup);
			//m_mutex.unlock();
			QString source = ((TraceTreeWidgetItem*)item_backup)->getSource();
			QString uuid = ((TraceTreeWidgetItem*)item_backup)->getUUID();
			LOGGER_INFO(log_, "ENQUEUING -> {}, UUID -> {}", item->text(0).toStdString(), uuid.toStdString());
		}
		else {
			shown_queue.enqueue(item);
			// limit the shown_queue size
			if (shown_queue.size() > count_per_page) {
				TraceTreeWidgetItem* it_remove = shown_queue.dequeue();
				delete it_remove;
			}

			TraceTreeWidgetItem* item_backup = (TraceTreeWidgetItem*)item->clone();
			full_queue_stream.enqueue(item_backup);
			//m_mutex.unlock();
			QString source = ((TraceTreeWidgetItem*)item_backup)->getSource();
			QString uuid = ((TraceTreeWidgetItem*)item_backup)->getUUID();
			LOGGER_INFO(log_, "ENQUEUING -> {}, UUID -> {}", item->text(0).toStdString(), uuid.toStdString());
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
	if (current_state == State::START || current_state == State::RESUMED) {
		calc_thread->pauseThread();
		timer->stop();
		last_status = "PAUSED";
		state_manager.changeState(State::PAUSE);
	}
	show_fullpage_with_findings();
	updateToolbar();
	updateContinuousProgress();

}

void QtWidgetsApplication1::hide_filtered_items(int column_idx, QList<QList<QString>> items)
{
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
	updateProgressRunStatus();
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
	LOGGER_INFO(log_, "======= UPDATE_TRACEWINDOW =======");
	LOGGER_INFO(log_, "LAST STATUS -> {}", last_status.toStdString());
	//m_mutex.lock();
	int queue_size = shown_queue.size() - padding;
	int queue_backup_size = full_queue_stream.size() - padding;
	qDebug() << "FULL -> " << queue_size << "BACKUP -> " << queue_backup_size;
	LOGGER_INFO(log_, "FULLSIZE -> {}, BACKUP_SIZE -> {}", queue_size, queue_backup_size);

	QTreeWidget* tree = ui.treetrace;
	QTreeWidgetItem* invisible_root_item = tree->invisibleRootItem();

	QSize itemSize;
	itemSize = getItemSize(invisible_root_item, 0, ui.treetrace->font());
	int v_height = ui.treetrace->viewport()->height();
	int window_capacity = visible_height / itemSize.height() - 1;

	visible_height = v_height;
	item_height = itemSize.height();
	item_width = itemSize.width();
	LOGGER_INFO(log_, "UPDATE_TRACEWINDOW CAPACITY -> {}", window_capacity);
	LOGGER_INFO(log_, "PAGE CAPACITY -> {}", page_capacity);

	int tree_count = ui.treetrace->topLevelItemCount();
	LOGGER_INFO(log_, "TREE COUNT -> {}", tree_count);

	/*
	һ����������ܹ�����60�У���ôwindow_capacity����60��
	 1.���Ŀǰ������Ϊ�գ������һ�С�
	 2.���Ŀǰ�����������ݵ��ǲ��㵱ҳ����������Ϊpage_capacity)����������page_capacity�С�
	 3.�����������page_capacity�У����滻�������ݡ�
	 */

	if (tree_count == 0) {
		LOGGER_INFO(log_, "==== EMPTY TREE ====");
		fill_empty_tree(window_capacity); // ���һ��
	}
	else if (tree_count > 0 && tree_count < page_capacity) {
		LOGGER_INFO(log_, "==== HALF TREE ====");
		fill_partial_tree(page_capacity); // ׷��
	}
	else {
		LOGGER_INFO(log_, "==== REDRAW TREE ====");
		refresh_full_tree(window_capacity);
	}
	//m_mutex.unlock();
	ui.treetrace->scrollToBottom();
	LOGGER_INFO(log_, "======= END of update_tracewindow =======");
	LOGGER_INFO(log_, "\n");
}

void QtWidgetsApplication1::refresh_full_tree(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "==== REFRESH_FULL_TREE ====");
	draw_trace_window(capacity);
	LOGGER_INFO(log_, "==== END OF REFRESH_FULL_TREE ====");
}

void QtWidgetsApplication1::draw_trace_window(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");
	LOGGER_INFO(log_, "DRAW TRACE WINDOW -> {}", capacity);

	int queue_size = shown_queue.size() - padding;
	LOGGER_INFO(log_, "DRAW QUEUE SIZE -> {}", queue_size);
	LOGGER_INFO(log_, "PADDING -> {}", padding);

	if (queue_size <= 0) return;

	QTreeWidget* tree = ui.treetrace;
	TraceTreeWidgetItem* it = nullptr;
	int changes = std::min(queue_size, capacity);
	LOGGER_INFO(log_, "DRAW CHANGES -> {}", changes);
	if (changes) {
		int count = tree->topLevelItemCount();
		LOGGER_INFO(log_, "TREE COUNT -> {}", count);
		for (int i = 0; i < changes; i++) {
			int idx = queue_size - changes + i;
			LOGGER_INFO(log_, "DRAW ITEM -> {}", idx);
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
						LOGGER_INFO(log_, "REF COUNTER -> {}", ref);
						qDebug() << "REF COUNTER" << ref;
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
	LOGGER_INFO(log_, "GAP -> {}", gap);
	if (gap <= 0) {
		//m_mutex.unlock();
		return;
	}
	int changes = std::min(queue_size, gap);
	LOGGER_INFO(log_, "PARTIAL CHANGES -> {}", changes);

	if (changes > 0) {
		int queue_original_size = shown_queue.size();
		qDebug() << "ORIGINAL QUEUE SIZE -> " << queue_original_size;
		QList<QTreeWidgetItem*> item_list;
		for (int i = 0; i < changes; i++) {
			int idx = queue_size - changes + i;
			LOGGER_INFO(log_, "PARTIAL INDEX -> {}", idx);
			if (idx >= 0) {
				TraceTreeWidgetItem* it0 = read_item_from_queue(idx);
				int it0colcount = it0->columnCount();
				if (filter_pass_item(it0)) {
					ui.treetrace->addTopLevelItem(it0);
				}
			}
		}
	}
	//m_mutex.unlock();
	LOGGER_INFO(log_, "END OF PARTIAL FILLING");
	LOGGER_INFO(log_, "====================");
}

void QtWidgetsApplication1::fill_empty_tree(int capacity)
{
	auto log_ = GETLOG("WORKFLOW");

	int queue_size = shown_queue.size() - padding;
	LOGGER_INFO(log_, "FULL QUEUE SIZE IN EMPTY FUNC -> {}", queue_size);
	if (queue_size <= 0) return;

	int changes = std::min(queue_size, capacity);
	if (changes > 0) {
		changes = 1;
		LOGGER_INFO(log_, "EMPTY CHANGES -> {}", changes);
		for (int i = 0; i < changes; i++) {
			TraceTreeWidgetItem* it = nullptr;
			int idx = queue_size - changes + i;
			it = this->read_item_from_queue(idx);
			LOGGER_INFO(log_, "READY INDEX -> {}", idx);
			if (it != nullptr) {
				LOGGER_INFO(log_, "ITEM INDEX -> {}", idx);
				if (filter_pass_item(it)) {
					ui.treetrace->addTopLevelItem(it);
				}
			}
		}
	}
	LOGGER_INFO(log_, "EXITING EMPTY TREE.");
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
	LOGGER_INFO(log_, "SIZE -> {}", size);
	if (size <= 0) return;
	LOGGER_INFO(log_, "TARGET -> {}", ncount);
	ui.treetrace->clear();
	int total = std::min(size, ncount) - 5;
	LOGGER_INFO(log_, "TOTAL -> {}", total);
	QTreeWidgetItem* it = nullptr;
	QList<QTreeWidgetItem*> item_list;
	item_list.clear();
	for (int i = 0; i < total; i++)
	{
		int idx = size - i;
		LOGGER_INFO(log_, "INDEX -> {}", idx);
		it = shown_queue.at(idx);
		if (it != nullptr && filter_pass_item(it)) {
			item_list.append(it);
		}
	}
	if (item_list.size())
	{
		ui.treetrace->addTopLevelItems(item_list);
		int frozen_size = item_list.size();
		LOGGER_INFO(log_, "FRONZEN -> {}", frozen_size);
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
}

bool QtWidgetsApplication1::filter_pass_item(QTreeWidgetItem* it)
{
	if (it == NULL) return false;

	bool matched = true;

	// running, only root items passed. 2024��10��3�� 9:39
	if (calc_thread->isRUN() && it) {
		matched = filter_run_pass_item_without_children(it);
		if (!matched) return false;
	}

	// filters. 2024��10��3�� 9:39
	if (new_filters.isEmpty())
		return true;
	for (const auto& key : new_filters.keys()) {
		QList<QVariant> vlist = new_filters.value(key);
		int col_index = vlist[0].toInt();
		int length = vlist.length();
		QList<QVariant> slice = vlist.mid(1, length);
		if (!slice.contains(it->text(col_index)))
		{ 
			for (const QVariant& value : slice) {
				qDebug() << "COL ->" << col_index << "Text ->" << it->text(col_index) << "Not Matched ->" << value;
			}
			matched = false;
		}
			
	}
	return matched;
}

bool QtWidgetsApplication1::filter_run_pass_item_without_children(QTreeWidgetItem* it)
{
	if (calc_thread->isRUN() && it) {
		try
		{
			int children_count = it->childCount();
			if (children_count > 0) {
				return false;
			}
			else return true;
		}
		catch (...)
		{
		}
	}
	else return true;
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
	LOGGER_INFO(log_, "ITEM CONTENT -> {}", item->text(0).toStdString());
	return item;
}

TraceTreeWidgetItem* QtWidgetsApplication1::read_item_from_dumb(int index)
{
	/* This function is a mock one to generate 10 rows data
	*  to the tracewidget to show. 2024��10��2�� 22:14
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
	*  2024��10��2�� 22:06
	*/
	current_page_queue.enqueue(item->clone());
	if (current_page_queue.size() > count_per_page)
		current_page_queue.dequeue();
}

void QtWidgetsApplication1::show_fullpage()
{
	if (timer_isRunning) return;
	timer_isRunning = true;

	qDebug() << "SHOW_FULLPAGE()";

	ui.treetrace->clear();
	safe_clear_trace();
	updateComoboPage();

	QQueue<QTreeWidgetItem*> baseQueue;
	int i = (paused_instant_index > count_per_page) ? paused_instant_index - count_per_page : 0;
	qDebug() << "PAUSED_INSTANT_INDEX ->" << paused_instant_index;
	for (;i < paused_instant_index;i++)
	{
		TraceTreeWidgetItem* traceItem = full_queue_stream.at(i)->clone();
		if (filter_pass_item(traceItem)) baseQueue.enqueue(traceItem);
	}
	qDebug() << "BASEQUEUE SIZE ->" << baseQueue.size();
	ui.treetrace->addTopLevelItems(baseQueue);
	timer_isRunning = false;
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
	//qDebug() << "PRINT ITEM -> " << "SIZE -> " << queue.size();
}

void QtWidgetsApplication1::update_latest_index(uint64_t index)
{
	paused_instant_index = index;
	qDebug() << "LATEST INDEX ->" << index;
	current_page_index = paused_instant_index / count_per_page;
	current_item_index = paused_instant_index % count_per_page;
	max_page_count = current_page_index + 1;
	//qDebug() << "CURRENT PAGE INDEX:" << current_item_index;
	//qDebug() << "CURRENT ITEM INDEX:" << current_page_index;
	qDebug() << "MAX PAGE COUNT -> " << max_page_count;
}

bool QtWidgetsApplication1::eventFilter(QObject* obj, QEvent* event) {
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
		else if (keyEvent->key() == Qt::Key_Down) {
			qDebug() << "DOWN KEY PRESSED";
			QTreeWidgetItem* current = ui.treetrace->currentItem();
			if (current) {
				highlight_next_item(current);
			}
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Tab) {
			QTreeWidgetItem* current = ui.treetrace->currentItem();
			if (current) {
				if (current->isExpanded())
					collapse_item(current);
				else
					extract_item(current);
			}
			return true;
		}
	}
	return QWidget::eventFilter(obj, event);
}

void QtWidgetsApplication1::updateComoboPage()
{
	ui.comboBox_Page->clear();
	int ncount = paused_instant_index / count_per_page + 1;
	for (int i = 1; i <= ncount; i++) {
		ui.comboBox_Page->addItem(QString::number(i));
	}
}

void QtWidgetsApplication1::ButtonFirstClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	int target_page = 1;
	qDebug() << "TARGET_PAGE -> " << target_page;
	show_fullpage_with_index(target_page);
	ui.label_Current->setText(QString::number(target_page));
}

void QtWidgetsApplication1::ButtonPreviousClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int current_page = ui.label_Current->text().toInt(&ok);
	int target_page = (current_page -1) < 1 ? 1: current_page - 1;
	qDebug() << "TARGET_PAGE -> " << target_page;
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
		show_fullpage_with_index(target_page);
		ui.label_Current->setText(QString::number(target_page));
	}
}

void QtWidgetsApplication1::ButtonLastClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	int target_page = max_page_count;
	if (target_page > 0) {
		qDebug() << "TARGET_PAGE -> " << target_page;
		show_fullpage_with_index(target_page);
		ui.label_Current->setText(QString::number(target_page));
	}
}

void QtWidgetsApplication1::ButtonGotoClicked()
{
	qDebug() << "MAX_PAGE_COUNT -> " << max_page_count;
	bool ok;
	int target_page = ui.comboBox_Page->currentText().toInt(&ok);
	qDebug() << "TARGET_PAGE -> " << target_page;
	show_fullpage_with_index(target_page);
	ui.label_Current->setText(QString::number(target_page));
}

void QtWidgetsApplication1::show_fullpage_with_findings()
{
	QString search_str = ui.mysearch->toPlainText().trimmed();
	QStringList items = search_str.split(QRegExp("[,;|]"), QString::SkipEmptyParts);
	//QMap<QString, QString> map;
	//QMap<int, QString> raw_map;
	QMap<int, QStringList> list_map;
	for (const QString& item : items) {
		qDebug() << item.trimmed();
		if (item.contains("=")) {
			QStringList item_map = item.split("=", QString::SkipEmptyParts);
			qDebug() << "ITEM MAP SIZE ->" << item_map.size();
			QString k = item_map[0].trimmed();
			QString v = item_map[1].trimmed();
			int nk = -1;
			for (int i = 0; i < CurrentHeader.size(); ++i) {
				if (CurrentHeader[i] == k) {
					//raw_map.insert(i, v);
					list_map[i] << v;
					break;
				}
			}
			//map.insert(k, v);
		}
	}

	for (int key : list_map.keys()) {
		qDebug() << "Key:" << key;
		const QStringList& list = list_map[key];
		for (const QString& item : list) {
			qDebug() << "  Value:" << item;
		}
	}

	QTreeWidgetItem* invisible_root_item = ui.treetrace->invisibleRootItem();
	int child_count = invisible_root_item->childCount();
	for (int i = 0; i < child_count; ++i) {
		QTreeWidgetItem* item = invisible_root_item->child(i);
		bool hidden = false;
		//for (const int& key : raw_map.keys()) {
		//	if (item->text(key) != raw_map.value(key)) {
		//		hidden = true;
		//		continue;
		//	}
		//}

		for (const int& key : list_map.keys()) {
			QStringList values = list_map.value(key);
			if (!values.contains(item->text(key))) {
				hidden = true;
				continue;
			}
		}
		item->setHidden(hidden);
	}
	updateContinuousProgress();
}

void QtWidgetsApplication1::show_fullpage_with_index(int index)
{
	// index is the page index  starting from 1
	if (timer_isRunning) return;
	timer_isRunning = true;
	qDebug() << "SHOW_FULLPAGE_WITH_INDEX";
	int page_index = index - 1;
	ui.treetrace->clear();
	safe_clear_trace();
	if (full_queue_stream.isEmpty()) return;
	QQueue<QTreeWidgetItem*> baseQueue;
	int pi = (page_index<0)?0:page_index;
	int i = count_per_page*pi;
	int stop = (count_per_page * index > paused_instant_index)? paused_instant_index : i+count_per_page;
	qDebug() << "INDEX FROM ->" << i << "TO ->" << stop;
	for (; i < stop; i++)
	{
		TraceTreeWidgetItem* traceItem = full_queue_stream.at(i)->clone();
		baseQueue.enqueue(traceItem);
	}
	qDebug() << "BASEQUEUE SIZE ->" << baseQueue.size();
	ui.treetrace->addTopLevelItems(baseQueue);
	ui.treetrace->scrollToBottom();
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
	}
	if (prevSibling) {
		ui.treetrace->setCurrentItem(prevSibling);
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
	}
	if (nextSibling) {
		ui.treetrace->setCurrentItem(nextSibling);
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
