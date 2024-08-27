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
    
    // 定义动作，菜单和工具栏
    createActions();

    // 定义定时器
    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &QtWidgetsApplication1::updateState);

    calc_thread = new multiThread();
    connect(calc_thread, &multiThread::traceItemUpdate_internal, [=]() {qDebug() << "lambda"; });
    connect(calc_thread, &multiThread::popToRoot, this, &QtWidgetsApplication1::on_pop_to_root);


    // 重置布局 todo
    resetLayout();

    // 更新状态 todo
    _updateCurrentState();
    setupTreeTrace();

    filter = new columnFilterDialog(this);

    showMaximized();
}

void QtWidgetsApplication1::_updateCurrentState()
{
    QString status_string = "CAN Frames: " + QString::number(this->calc_thread->full_count_canframes)
        + ". PDUs: " + QString::number(this->calc_thread->full_count_canparser);
    ui.statusBar->showMessage(status_string);
}

void QtWidgetsApplication1::updateState()
{
    _updateCurrentState();
}

void QtWidgetsApplication1::createActions()
{
    qDebug() << "createActions...";
    connect(ui.actionstart, &QAction::triggered, this, &QtWidgetsApplication1::startTrace);
    connect(ui.actionstop, &QAction::triggered, this, &QtWidgetsApplication1::stopTrace);
    connect(ui.actionpause, &QAction::triggered, this, &QtWidgetsApplication1::pauseTrace);
}

void QtWidgetsApplication1::resetLayout()
{
    qDebug() << "resetLayout...";

    // will be removed later.
    QAction* action = new QAction("Temp Action");
    ui.menu->addAction(action);
    connect(action, &QAction::triggered, this, &QtWidgetsApplication1::onActionTriggered);

    // icons
    ui.menu->setIcon(QIcon(":/QtWidgetsApplication1/res/sqlitestudio.ico"));

    ui.actionpause->setIcon(QIcon(":/QtWidgetsApplication1/res/pauseTrace.png"));
    ui.actionstart->setIcon(QIcon(":/QtWidgetsApplication1/res/startTrace.png"));
    ui.actionstop->setIcon(QIcon(":/QtWidgetsApplication1/res/stopTrace.png"));

    // toolbar actions
    ui.toolbar->addAction(ui.actionstart);
    ui.toolbar->addAction(ui.actionstop);
    ui.toolbar->addAction(ui.actionpause);
    initialHeaders();

    ui.treetrace->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treetrace, &QTreeWidget::customContextMenuRequested, this, &QtWidgetsApplication1::prepareMenu);
}

void QtWidgetsApplication1::prepareMenu(const QPoint& pos)
{
    QTreeWidget* tree = ui.treetrace;
    QTreeWidgetItem* nd = tree->itemAt(pos);
    QAction* newAct = new QAction(QIcon(":/QtWidgetsApplication1/res/funnel-icon.ico"), tr("&TBD"), this);
    newAct->setStatusTip(tr("TBD"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newDev()));
    QMenu menu(this);
    menu.addAction(newAct);
    QPoint pt(pos);
    menu.exec(tree->mapToGlobal(pos));
}

void QtWidgetsApplication1::newDev()
{
    qDebug() << tr("newDev");
}

void QtWidgetsApplication1::onActionTriggered()
{
    qDebug() << tr("菜单项已触发！");
}

void QtWidgetsApplication1::startTrace()
{
    qDebug() << "startTrace...";
    std::cout << "Starting subscriber." << std::endl;

    uint32_t samples = 10;

    if (mysub_can_frames == nullptr) {
        mysub_can_frames = new ZoneMasterCanMessageDataSubscriber();
        qRegisterMetaType <can_frame>("can_frame");
        //connect(&mysub_can_frames->listener_, &SubListener::traceItemUpdate_internal_cf, this, &QtWidgetsApplication1::formatRow_canframe);
    }

    if (mysub_can_parser == nullptr) {
        mysub_can_parser = new ZoneMasterCanParserSubscriber();
        qRegisterMetaType <canframe>("canframe");
        //connect(&mysub_can_parser->listener_, &CanParserListener::traceItemUpdate_internal, this, &QtWidgetsApplication1::formatRow_canparser);
        //connect(&mysub_can_parser->listener_, &CanParserListener::traceItemUpdate_internal_canparser, this, &QtWidgetsApplication1::internal_canparser);
    }
    calc_thread->restartThread();
    calc_thread->setSubscriber(mysub_can_frames, samples, ui.treetrace); // nonsense
    calc_thread->setCanParserSubscriber(mysub_can_parser, samples, ui.treetrace);
    calc_thread->start();
    timer->start(1000);
}

void QtWidgetsApplication1::stopTrace()
{
    qDebug() << "stopTrace...";
    calc_thread->stopThread();
    mysub_can_frames = nullptr;
    mysub_can_parser = nullptr;
    timer->stop();
}

void QtWidgetsApplication1::pauseTrace()
{
    qDebug() << "pauseTrace...";
    calc_thread->pauseThread();
    timer->stop();
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
    full_canframes.append(cf);
    setupdatamodel();
}

void QtWidgetsApplication1::formatRow_canparser(unsigned long long i)
{
    qDebug() << "canparser id ->" << i;
    for (int k = 0; k < full_canframes.size(); k++) {
        if (full_canframes[k].Timestamp == i) {
            qDebug() << "MATCHED ->" << full_canframes[k].ID << "INDEX ->" << k;
        }
    }
}
void QtWidgetsApplication1::internal_canparser(canframe frame)
{
    //qDebug() << "canframe ->" << QString::fromStdString(frame.name());
    full_canparserdata.append(frame);
    setupdatamodel_canparser();
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
void QtWidgetsApplication1::setupdatamodel_canparser()
{
    QStringList str = {};
    for (int i = 0; i < this->full_canparserdata.count(); i++)
    {
        if (full_canparserdata[i].timeStamp() <= last_timestamp_canparser) continue;
        canframe cf = full_canparserdata[i];
        last_timestamp_canparser = cf.timeStamp();
        full_count_canparser++;
        QStringList str_parser = {};
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(full_canparserdata[i].timeStamp() / 1000000);
        str_parser.append(timestamp.toString("hh:mm:ss.zzz"));
        str_parser.append("");
        str_parser.append("");
        str_parser.append(QString::fromStdString(full_canparserdata[i].name()));
        QTreeWidgetItem* Item = new QTreeWidgetItem(str_parser);
        QTreeWidget* t = ui.treetrace;
        t->addTopLevelItem(Item);
        t->setIndentation(20);
        std::vector<canpdu> pdus = cf.pdus();
        std::vector<canpdu> containspdus = cf.containPdus();
        for (int k = 0; k < pdus.size(); k++) {
            canpdu current_pdu = pdus[k];
            QString pdu_name = QString::fromStdString(current_pdu.name());
            QString pdu_size = QString::number(current_pdu.zone_signals().size());
            QStringList str_pdu = {};
            str_pdu.append(pdu_name);
            str_pdu.append(pdu_size);
            QTreeWidgetItem* item_pdu = new QTreeWidgetItem(str_pdu);
            for (int m = 0; m < current_pdu.zone_signals().size(); m++) {
                QStringList str_signal = {};
                cansignal current_signal = current_pdu.zone_signals()[m];
                str_signal.append(QString::fromStdString(current_signal.name()));
                str_signal.append(QString::number(current_signal.raw_value()));
                str_signal.append(QString::fromStdString(current_signal.phy_value()));
                QTreeWidgetItem *item_signal = new QTreeWidgetItem(str_signal);
                item_pdu->addChild(item_signal);
            }            
            Item->addChild(item_pdu);
        }

        if (containspdus.size() == 0) continue;

        for (int k = 0; k < containspdus.size(); k++) {
            canpdu current_pdu = containspdus[k];
            QString pdu_name = QString::fromStdString(current_pdu.name());
            QString pdu_size = QString::number(current_pdu.zone_signals().size());
            QStringList str_pdu = {};
            str_pdu.append(pdu_name);
            str_pdu.append(pdu_size);
            QTreeWidgetItem* item_pdu = new QTreeWidgetItem(str_pdu);
            for (int m = 0; m < current_pdu.zone_signals().size(); m++) {
                QStringList str_signal = {};
                cansignal current_signal = current_pdu.zone_signals()[m];
                str_signal.append(QString::fromStdString(current_signal.name()));
                str_signal.append(QString::number(current_signal.raw_value()));
                str_signal.append(QString::fromStdString(current_signal.phy_value()));
                QTreeWidgetItem* item_signal = new QTreeWidgetItem(str_signal);
                item_pdu->addChild(item_signal);
            }
            Item->addChild(item_pdu);
        }
    }
}

void QtWidgetsApplication1::setupdatamodel()
{
    QStringList str = {};
    for (int i =0; i< full_canframes.count(); i++)
    {
        if (full_canframes[i].Timestamp <= last_timestamp) continue;

        full_count_canframes++; // message counts
        int count_in_page = full_count_canframes % count_per_page;
        int last_page_index = full_count_canframes / count_per_page;
        if (last_page_index > 0) {
            if (last_page_index != current_page)
                if (count_in_page != 0) {
                    QTreeWidget* t = ui.treetrace;
                    t->clear();
                    current_page = last_page_index;
                    qDebug() << full_count_canframes << endl;
                }
        }
        
        last_timestamp = full_canframes[i].Timestamp;
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(full_canframes[i].Timestamp/1000000);
        str.append(timestamp.toString("hh:mm:ss.zzz"));
        str.append(QString::number(full_canframes[i].Chn));
        str.append("0x" +QString::number(full_canframes[i].ID, 16));
        str.append(full_canframes[i].Name);
        str.append(full_canframes[i].Dir);
        str.append(QString::number(full_canframes[i].DLC));
        QString myData = full_canframes[i].Data_Str;
        str.append(myData);
        str.append(full_canframes[i].EventType);
        str.append(QString::number(full_canframes[i].DataLength));
        str.append(full_canframes[i].BusType);
        QTreeWidgetItem* Item = new QTreeWidgetItem(str);
        QTreeWidget* t = ui.treetrace;
        t->addTopLevelItem(Item);
        t->setIndentation(20);
        
    }
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
    qDebug() << "headerButtonClicked" << endl;

    QPushButton* filterButton = qobject_cast<QPushButton*>(sender());
    int columnButton = headerButtonList.indexOf(filterButton);
    QList<QString> filterConfig;

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
    qDebug() << "COLUMN -> " << colName;
    if (items.size() == 0) colName = "";
    for (int k = 0; k < items.size(); k++) {
        for (int i = 0; i < 1; i++) {
            QString s = items[k][i];
            qDebug() << "FILTER -> " << s;
        }
    }
    this->calc_thread->setFilterOption(colName, items);
}


void QtWidgetsApplication1::on_pop_to_root(QTreeWidgetItem* item)
{
    ui.treetrace->addTopLevelItem(item);
}