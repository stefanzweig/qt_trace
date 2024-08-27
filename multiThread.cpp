#include "multiThread.h"
#include <QDebug>
#include <QTreeWidgetItem>

multiThread::multiThread()
    :is_stop(false),is_paused(false)
{
}

multiThread::~multiThread() {}

void multiThread::stopThread() {
    is_stop = true;
    is_paused = false;

    if (mysub_can_frames != nullptr) {
        delete mysub_can_frames;
        mysub_can_frames = nullptr;
    }
    if (mysub_can_parser != nullptr) {
        delete mysub_can_parser;
        mysub_can_parser = nullptr;
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
    
    // from_t = get_timestamp();
    if (mysub_can_frames != nullptr) {
        qDebug() << "mysub is not nullptr";
            if (mysub_can_frames->init()) {
                qDebug() << "mysub before run";
                mysub_can_frames->run(samples_);
                qDebug() << "mysub after run";
                qDebug() << "debug: inited";
            }
    }


    if (mysub_can_parser != nullptr) {
        qDebug() << "mysubparser is not nullptr";
        if (mysub_can_parser->init()) {
            qDebug() << "mysubparser before run";
            mysub_can_parser->run(samples_);
            qDebug() << "mysubparser after run";
            qDebug() << "debug: mysubparser inited";
        }
    }

    while (true) {
        if (!is_stop) {
            qDebug() << "running...";
            msleep(1000); // every 1 second
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
        qDebug() << "mongodb problem from " << from_t << " to " << to_t;
    }

    try {
            m_from_id = to_id;

            m_lock.unlock();
            msleep(20);
    }
    catch (...) {
        qDebug() << "handling result problem.";
    }
}

void multiThread::setQueryString(QString str)
{
    this->query_string = str;
}

void multiThread::setSubscriber(ZoneMasterCanMessageDataSubscriber* subscriber, int samples, QTreeView* treeview)
{
    mysub_can_frames = subscriber;
    samples_ = samples;
    subscriber->setOuterThread(this, treeview);
    QObject::connect(&mysub_can_frames->listener_, &SubListener::traceItemUpdate_internal_cf, this, &multiThread::formatRow_canframe_thread);
}

void multiThread::setCanParserSubscriber(ZoneMasterCanParserSubscriber* subscriber, int samples, QTreeView* treeview)
{
    mysub_can_parser = subscriber;
    samples_ = samples;
    subscriber->setOuterThread(this, treeview);
    QObject::connect(&mysub_can_parser->listener_, &CanParserListener::traceItemUpdate_internal_canparser, this, &multiThread::formatRow_canparser_thread);
}

void multiThread::formatRow_canparser_thread(canframe frame)
{
    full_canparserdata.append(frame);
    full_count_canparser++;
    last_timestamp_canparser = frame.timeStamp();

    QStringList str_parser = {};
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.timeStamp() / 1000000);
    str_parser.append(timestamp.toString("hh:mm:ss.zzz"));
    str_parser.append("");
    str_parser.append("");
    str_parser.append(QString::fromStdString(frame.name()));
    QTreeWidgetItem* Item = new QTreeWidgetItem(str_parser);

    std::vector<canpdu> pdus = frame.pdus();
    std::vector<canpdu> containspdus = frame.containPdus();
        
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
            QTreeWidgetItem* item_signal = new QTreeWidgetItem(str_signal);
            item_pdu->addChild(item_signal);
        }
        Item->addChild(item_pdu);
    }

    if (containspdus.size() == 0)
    {
        emit popToRoot(Item);
    }

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

void multiThread::formatRow_canframe_thread(can_frame frame)
{
    full_canframes.append(frame);
    full_count_canframes++; // message counts

    QStringList str = {};
    QStringList filter_value = {};
    bool bFilter = false;
    last_timestamp_canframe = frame.Timestamp;
    if (filter_items.size() > 0) {
        bFilter = true;
        for (int k = 0; k < filter_items.size(); k++) {
            for (int i = 0; i < 1; i++) {
                filter_value.append(filter_items[k][i]);
            }
        }
    }

    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.Timestamp / 1000000);
    str.append(timestamp.toString("hh:mm:ss.zzz"));
    str.append(QString::number(frame.Chn));
    str.append("0x" + QString::number(frame.ID, 16));
    str.append(frame.Name);
    str.append(frame.Dir);
    str.append(QString::number(frame.DLC));
    QString myData = frame.Data_Str;
    str.append(myData);
    str.append(frame.EventType);
    str.append(QString::number(frame.DataLength));
    str.append(frame.BusType);
    QTreeWidgetItem* Item = new QTreeWidgetItem(str);

    if (bFilter) {
        if (filter_colName == "Chn")
        {
            if (filter_value.contains(QString::number(frame.Chn))) {
                emit(popToRoot(Item));
            }                
        }
        if (filter_colName == "ID")
        {
            QString hexv = "0x" + QString::number(frame.ID, 16);
            if (filter_value.contains(hexv)) {
                emit(popToRoot(Item));
            }
        }


        if (filter_colName == "Dir")
        {
            if (filter_value.contains(frame.Dir)) {
                emit(popToRoot(Item));
            }
        }

        if (filter_colName == "DLC")
        {
            QString hexv = QString::number(frame.DLC);
            if (filter_value.contains(hexv)) {
                emit(popToRoot(Item));
            }
        }
        if (filter_colName == "EventType")
        {
            if (filter_value.contains(frame.EventType)) {
                emit(popToRoot(Item));
            }
        }
        if (filter_colName == "DataLength")
        {
            QString hexv = QString::number(frame.DataLength);
            if (filter_value.contains(hexv)) {
                emit(popToRoot(Item));
            }
        }
        if (filter_colName == "BusType")
        {
            if (filter_value.contains(frame.BusType)) {
                emit(popToRoot(Item));
            }
        }
    }
    else {
        emit(popToRoot(Item));
    }
}

void multiThread::setFilterOption(QString colName, QList<QList<QString>> items)
{
    filter_colName = colName;
    filter_items = items;
}