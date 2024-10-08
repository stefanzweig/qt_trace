#include "multiThread.h"
#include <QDebug>
#include <QTreeWidgetItem>
#include "TraceTreeWidgetItem.h"

multiThread::multiThread()
    :is_stop(true),is_paused(false)
{
}

multiThread::~multiThread() {}

void multiThread::stopThread() {
    is_stop = true;
    is_paused = false;

    if (mysub_can_frames != nullptr) {
        delete mysub_can_frames;
        mysub_can_frames = nullptr;
        bconnected_cf = false;
    }
    if (mysub_can_parser != nullptr) {
        delete mysub_can_parser;
        mysub_can_parser = nullptr;
        bconnected_cp = false;
    }
    if (mysub_lin_frames != nullptr) {
        delete mysub_lin_frames;
        mysub_lin_frames = nullptr;
        bconnected_lf = false;
    }
    if (mysub_lin_parser != nullptr) {
        delete mysub_lin_parser;
        mysub_lin_parser = nullptr;
        bconnected_lp = false;
    }
}

void multiThread::stopFlag()
{
    is_stop = true;
    is_paused = false;
}

void multiThread::pauseThread() {
    if (!is_stop) {
        is_paused = true;
    }
}

void multiThread::restartThread() {
    is_stop = false;
    is_paused = false;
}

void multiThread::run() {
    // key function
    if (monitor_modules.contains("can")) {
        if (mysub_can_frames != nullptr) {
            if (mysub_can_frames->init()) {
            }
        }
    }

    if (monitor_modules.contains("canpdu")) {
        if (mysub_can_parser != nullptr) {
            if (mysub_can_parser->init()) {
            }
        }
    }
    if (monitor_modules.contains("lin")) {
        if (mysub_lin_frames != nullptr) {
            if (mysub_lin_frames->init()) {
            }
        }
    }
    if (monitor_modules.contains("linpdu")) {
        if (mysub_lin_parser != nullptr) {
            if (mysub_lin_parser->init()) {
            }
        }
    }
    while (true) {
        if (!is_stop) {
            msleep(100); // every 1 second
        }
        else {
            break;
        }
    }
}

std::time_t multiThread::get_timestamp()
{
    std::chrono::time_point<std::chrono::system_clock,
        std::chrono::microseconds> tp =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::time_t timestamp = tp.time_since_epoch().count() * 1000;
    return timestamp;
}

void multiThread::bindDataToTraceTree()
{

    try {
        m_from_id = to_id;
    }
    catch (...) {
        //qDebug() << "mongodb problem from " << from_t << " to " << to_t;
    }

    try {
            m_from_id = to_id;
            m_lock.unlock();
            msleep(20);
    }
    catch (...) {
        //qDebug() << "handling result problem.";
    }
}

void multiThread::setQueryString(QString str)
{
    this->query_string = str;
}

void multiThread::setCanSubscriber(ZoneMasterCanMessageDataSubscriber* subscriber, int samples, QTreeView* treeview)
{
    if (!bconnected_cf) {
        mysub_can_frames = subscriber;
        samples_ = samples;
        subscriber->setOuterThread(this, treeview);
        QObject::connect(&mysub_can_frames->listener_, &SubListener::traceItemUpdate_internal_cf, this, &multiThread::formatRow_canframe_thread); 
        bconnected_cf = true;
    }
    
}

void multiThread::setCanParserSubscriber(ZoneMasterCanParserSubscriber* subscriber, int samples, QTreeView* treeview)
{
    if (!bconnected_cp) {
        mysub_can_parser = subscriber;
        samples_ = samples;
        subscriber->setOuterThread(this, treeview);
        QObject::connect(&mysub_can_parser->listener_, &CanParserListener::traceItemUpdate_internal_canparser, this, &multiThread::formatRow_canparser_thread);
        bconnected_cp = true;
    }
}

void multiThread::setLinSubscriber(ZoneMasterLinMessageDataSubscriber* subscriber, int samples, QTreeView* treeview)
{
    if (!bconnected_lf) {
        mysub_lin_frames = subscriber;
        samples_ = samples;
        subscriber->setOuterThread(this, treeview);
        //QObject::connect(&mysub_lin_frames->listener_, &LinSubListener::traceItemUpdate_internal_lin_frame, this, &multiThread::formatRow_linframe_thread);
        bconnected_lf = true;
    }
}

void multiThread::setLinParserSubscriber(ZoneMasterLinParserSubscriber* subscriber, int samples, QTreeView* treeview)
{
    if (!bconnected_lp) {
        mysub_lin_parser = subscriber;
        samples_ = samples;
        subscriber->setOuterThread(this, treeview);
        //QObject::connect(&mysub_lin_parser->listener_, &LinParserListener::traceItemUpdate_internal_linparser, this, &multiThread::formatRow_linparser_thread);
        bconnected_lp = true;
    }
}


void multiThread::formatRow_canparser_thread(canframe frame)
{
    full_count_canparser++;
    if (is_paused) { return; }

    last_timestamp_canparser = frame.timeStamp();

    QStringList str_parser = {};
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.timeStamp() / 1000000);
    str_parser.append(timestamp.toString("hh:mm:ss.zzz"));
    str_parser.append("");
    str_parser.append("");
    str_parser.append(QString::fromStdString(frame.name()));
    TraceTreeWidgetItem* Item = new TraceTreeWidgetItem(str_parser);
    Item->setSource("can_parser");
    UUIDv4::UUID uuid = uuidGenerator.getUUID();
    std::string s = uuid.str();
    QString uuid_str = QString::fromStdString(s);
    Item->setUUID(uuid_str);
    if (Item == nullptr)
        return;

    std::vector<canpdu> pdus = frame.pdus();
    std::vector<canpdu> containspdus = frame.containPdus();
        
    for (int k = 0; k < pdus.size(); k++) {
        canpdu current_pdu = pdus[k];
        QString pdu_name = QString::fromStdString(current_pdu.name());
        QString pdu_size = QString::number(current_pdu.zone_signals().size());
        QStringList str_pdu = {};
        str_pdu.append(pdu_name);
        str_pdu.append(pdu_size);
        TraceTreeWidgetItem* item_pdu = new TraceTreeWidgetItem(str_pdu);
        item_pdu->setSource("can_pdu");
        
        for (int m = 0; m < current_pdu.zone_signals().size(); m++) {
            QStringList str_signal = {};
            cansignal current_signal = current_pdu.zone_signals()[m];
            str_signal.append(QString::fromStdString(current_signal.name()));
            str_signal.append(QString::number(current_signal.raw_value()));
            str_signal.append(QString::fromStdString(current_signal.phy_value()));
            //QTreeWidgetItem* item_signal = new QTreeWidgetItem(str_signal);
            TraceTreeWidgetItem* item_signal = new TraceTreeWidgetItem(str_pdu);
            item_signal->setSource("can_pdu_signal");
            item_pdu->addChild(item_signal);
        }
        Item->addChild(item_pdu);
    }

    if (containspdus.size() == 0)
    {
        emit popToRoot(Item);
    }
    else {
        for (int k = 0; k < containspdus.size(); k++) {
            canpdu current_pdu = containspdus[k];
            QString pdu_name = QString::fromStdString(current_pdu.name());
            QString pdu_size = QString::number(current_pdu.zone_signals().size());
            QStringList str_pdu = {};
            str_pdu.append(pdu_name);
            str_pdu.append(pdu_size);
            TraceTreeWidgetItem* item_pdu = new TraceTreeWidgetItem(str_pdu);
            item_pdu->setSource("can_container_pdu");
            for (int m = 0; m < current_pdu.zone_signals().size(); m++) {
                QStringList str_signal = {};
                cansignal current_signal = current_pdu.zone_signals()[m];
                str_signal.append(QString::fromStdString(current_signal.name()));
                str_signal.append(QString::number(current_signal.raw_value()));
                str_signal.append(QString::fromStdString(current_signal.phy_value()));
                TraceTreeWidgetItem* item_signal = new TraceTreeWidgetItem(str_pdu);
                item_signal->setSource("can_container_pdu_signal");
                item_pdu->addChild(item_signal);
            }
            Item->addChild(item_pdu);
        }
        emit popToRoot(Item);
    }
}

void multiThread::formatRow_canframe_thread(can_frame frame)
{
    //full_canframes.append(frame);
    full_count_canframes++; // message counts
    //return;

    if (is_paused) { return; }

    QStringList can_list = {};
    QStringList filter_value = {};
    bool bFilter = false;
    last_timestamp_canframe = frame.Timestamp;

    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.Timestamp / 1000000);
    can_list.append(timestamp.toString("hh:mm:ss.zzz"));
    can_list.append("CAN " +QString::number(frame.Chn));
    can_list.append(QString::number(frame.ID, 16).toUpper());
    can_list.append("CAN Frame");

    QString sDir = "Rx";
    if (frame.Dir == "1") sDir = "Tx";
    can_list.append(sDir);
    
    can_list.append(QString::number(frame.DLC));
    can_list.append(frame.EventType);
    can_list.append(QString::number(frame.DataLength));
    can_list.append(frame.BusType);
    
    QString myData = frame.Data_Str;
    can_list.append(myData);
    
    TraceTreeWidgetItem* Item = new TraceTreeWidgetItem(can_list);
    if (Item != nullptr) {
        UUIDv4::UUID uuid = uuidGenerator.getUUID();
        std::string s = uuid.str();
        QString uuid_str = QString::fromStdString(s);
        Item->setUUID(uuid_str);
        emit(popToRoot(Item));
    }
        
}

void multiThread::formatRow_linframe_thread(linFrame frame)
{
}

void multiThread::formatRow_linparser_thread(linFrame frame)
{
}

void multiThread::setFilterOption(QString colName, QList<QList<QString>> items)
{
    filter_colName = colName;
    filter_items = items;
}