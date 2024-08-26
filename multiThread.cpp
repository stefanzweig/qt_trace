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
    qDebug() << "PDU ID -> "  << QString::number(frame.id());
    full_canparserdata.append(frame);


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
        //QTreeWidget* t = ui.treetrace;
        //t->addTopLevelItem(Item);
        //t->setIndentation(20);
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
                QTreeWidgetItem* item_signal = new QTreeWidgetItem(str_signal);
                item_pdu->addChild(item_signal);
            }
            Item->addChild(item_pdu);
        }

        if (containspdus.size() == 0)
        {
            emit popToRoot(Item);
            continue;
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
}

void multiThread::formatRow_canframe_thread(can_frame frame)
{
    qDebug() << "FRAME ID -> " << QString::number(frame.ID);

    full_canframes.append(frame);
    QStringList str = {};
    for (int i = 0; i < full_canframes.count(); i++)
    {
        if (full_canframes[i].Timestamp <= last_timestamp_canframe) continue;

        full_count_canframes++; // message counts
        //int count_in_page = full_count_canframes % count_per_page;
        //int last_page_index = full_count_canframes / count_per_page;
        //if (last_page_index > 0) {
        //    if (last_page_index != current_page)
        //        if (count_in_page != 0) {
        //            QTreeWidget* t = ui.treetrace;
        //            t->clear();
        //            current_page = last_page_index;
        //            qDebug() << full_count_canframes << endl;
        //        }
        //}
        last_timestamp_canframe = full_canframes[i].Timestamp;
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(full_canframes[i].Timestamp / 1000000);
        str.append(timestamp.toString("hh:mm:ss.zzz"));
        str.append(QString::number(full_canframes[i].Chn));
        str.append("0x" + QString::number(full_canframes[i].ID, 16));
        str.append(full_canframes[i].Name);
        str.append(full_canframes[i].Dir);
        str.append(QString::number(full_canframes[i].DLC));
        QString myData = full_canframes[i].Data_Str;
        str.append(myData);
        str.append(full_canframes[i].EventType);
        str.append(QString::number(full_canframes[i].DataLength));
        str.append(full_canframes[i].BusType);
        QTreeWidgetItem* Item = new QTreeWidgetItem(str);
        //QTreeWidget* t = ui.treetrace;
        //t->addTopLevelItem(Item);
        //t->setIndentation(20);
        emit(popToRoot(Item));
    }
}

