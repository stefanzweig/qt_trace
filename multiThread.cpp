#include "multiThread.h"
#include <QDebug>
#include <QTreeWidgetItem>
#include "TraceTreeWidgetItem.h"

multiThread::multiThread()
    :is_stop(true),
    is_paused(false),
    queue_(nullptr),
    tree_(nullptr)
{
}

multiThread::~multiThread() {
    clear_items_queue();
}

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
    if (mysub_someip != nullptr) {
        delete mysub_someip;
        mysub_someip = nullptr;
        bconnected_someip = false;
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
    if (monitor_modules.contains("canpdu")) {
        if (mysub_can_parser != nullptr) {
            if (mysub_can_parser->init()) {
            }
        }
    }

    if (monitor_modules.contains("can")) {
        if (mysub_can_frames != nullptr) {
            if (mysub_can_frames->init()) {
            }
        }
    }

    if (monitor_modules.contains("linpdu")) {
        if (mysub_lin_parser != nullptr) {
            if (mysub_lin_parser->init()) {
            }
        }
    }

    if (monitor_modules.contains("lin")) {
        if (mysub_lin_frames != nullptr) {
            if (mysub_lin_frames->init()) {
            }
        }
    }

    if (monitor_modules.contains("someip")) {
        if (mysub_someip != nullptr) {
            if (mysub_someip->init()) {
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
        QObject::connect(&mysub_lin_frames->listener_, &LinSubListener::ItemUpdate_internal_lin_frame, this, &multiThread::formatRow_linframe_thread);
        bconnected_lf = true;
    }
}

void multiThread::setLinParserSubscriber(ZoneMasterLinParserSubscriber* subscriber, int samples, QTreeView* treeview)
{
    if (!bconnected_lp) {
        mysub_lin_parser = subscriber;
        samples_ = samples;
        subscriber->setOuterThread(this, treeview);
        QObject::connect(&mysub_lin_parser->listener_, &LinParserListener::traceItemUpdate_internal_linparser, this, &multiThread::formatRow_linparser_thread);
        bconnected_lp = true;
    }
}
void multiThread::setSomeipSubscriber(ZoneMasterSomeipSubscriber* subscriber, int samples, QTreeView* treeview)
{
    if (!bconnected_someip) {
        mysub_someip = subscriber;
        samples_ = samples;
        subscriber->setOuterThread(this, treeview);
        QObject::connect(&mysub_someip->listener_package, &SomeipPackageListener::ItemUpdate_internal_someip_package, this, &multiThread::formatRow_someip_package_thread);
        QObject::connect(&mysub_someip->listener_calling, &SomeipCallingListener::ItemUpdate_internal_someip_calling, this, &multiThread::formatRow_someip_calling_thread);
        //QObject::connect(&mysub_someip->listener_state, &SomeipStateListener::ItemUpdate_internal_someip_state, this, &multiThread::formatRow_someip_state_thread);
        //QObject::connect(&mysub_someip->listener_eth_frame, &SomeipEthFrameListener::ItemUpdate_internal_eth_frame, this, &multiThread::formatRow_someip_eth_frame_thread);
        //QObject::connect(&mysub_someip->listener_sd, &SomeipSDListener::ItemUpdate_internal_someip_sd, this, &multiThread::formatRow_someip_sd_thread);
        bconnected_someip = true;
    }
}

void multiThread::formatRow_canparser_thread(canframe frame)
{
    full_count_canparser++;

    last_timestamp_canparser = frame.timeStamp();
    QStringList str_parser = {};
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.timeStamp() / 1000000);
    str_parser.append(timestamp.toString("hh:mm:ss.zzz"));
    str_parser.append("CAN " + QString::number(frame.channel()));
    str_parser.append(QString::number(frame.id(), 16).toUpper());
    str_parser.append(QString::fromStdString(frame.name()));
    str_parser.append("");
    str_parser.append(QString::number(frame.dlc()));
    int isfd = frame.isFd();
    QString EventType = (isfd) ? "CANFD" : "CAN";
    str_parser.append(EventType);
    str_parser.append(QString::number(frame.dataLen()));
    str_parser.append("CAN Frame");

    TraceTreeWidgetItem* Item = new TraceTreeWidgetItem(str_parser);
    Item->setSource("can_parser");
    UUIDv4::UUID uuid = uuidGenerator.getUUID();
    std::string s = uuid.str();
    QString uuid_str = QString::fromStdString(s);
    Item->setUUID(uuid_str);
    Item->setData(0, Qt::UserRole, uuid_str);
    if (Item == nullptr)
        return;

    std::vector<canpdu> pdus = frame.pdus();
    std::vector<canpdu> containspdus = frame.containPdus();

    for (int k = 0; k < pdus.size(); k++)
    {
        canpdu current_pdu = pdus[k];
        QString pdu_name = QString::fromStdString(current_pdu.name());
        QString pdu_size = QString::number(current_pdu.zone_signals().size());
        QStringList str_pdu = {};
        str_pdu.append(pdu_name);
        str_pdu.append(pdu_size);
        str_pdu.append("");
        str_pdu.append("");
        str_pdu.append("");
        str_pdu.append("");
        str_pdu.append("");
        str_pdu.append("");
        str_pdu.append("");
        QByteArray output;
        std::copy(current_pdu.data().begin(), current_pdu.data().end(), std::back_inserter(output));
        QString myData = output.toHex(' ');
        str_pdu.append(myData);
        TraceTreeWidgetItem* item_pdu = new TraceTreeWidgetItem(str_pdu);
        item_pdu->setSource("can_pdu");

        for (int m = 0; m < current_pdu.zone_signals().size(); m++) {
            QStringList str_signal = {};
            QStringList str_signal_tooltips = {};
            cansignal current_signal = current_pdu.zone_signals()[m];

            str_signal.append(QString::fromStdString(current_signal.name()));
            str_signal.append(QString::number(current_signal.raw_value()));
            str_signal.append(QString::fromStdString(current_signal.phy_value()));
            str_signal_tooltips << "name: " + QString::fromStdString(current_signal.name())
                << "raw_value: " + QString::number(current_signal.raw_value())
                << "phy_value: " + QString::fromStdString(current_signal.phy_value());

            TraceTreeWidgetItem* item_signal = new TraceTreeWidgetItem(str_signal);
            item_signal->setSource("can_pdu_signal");
            item_signal->setToolTip(0, str_signal_tooltips.join("<br>"));
            item_pdu->addChild(item_signal);
        }
        Item->addChild(item_pdu);
    }

    if (containspdus.size() == 0)
    {
        emit popToRoot(Item);
        list_items_queue.enqueue(Item);
    }
    else
    {
        for (int k = 0; k < containspdus.size(); k++)
        {
            canpdu current_pdu = containspdus[k];
            QString pdu_name = QString::fromStdString(current_pdu.name());
            QString pdu_size = QString::number(current_pdu.zone_signals().size());
            QStringList str_pdu = {};
            str_pdu.append(pdu_name);
            str_pdu.append(pdu_size);
            str_pdu.append("");
            str_pdu.append("");
            str_pdu.append("");
            str_pdu.append("");
            str_pdu.append("");
            str_pdu.append("");
            str_pdu.append("");
            QByteArray output;
            std::copy(current_pdu.data().begin(), current_pdu.data().end(), std::back_inserter(output));
            QString myData = output.toHex(' ');
            str_pdu.append(myData); //data

            TraceTreeWidgetItem* item_pdu = new TraceTreeWidgetItem(str_pdu);
            item_pdu->setSource("can_container_pdu");
            for (int m = 0; m < current_pdu.zone_signals().size(); m++) {
                QStringList str_signal = {};
                QStringList str_signal_tooltips = {};
                cansignal current_signal = current_pdu.zone_signals()[m];

                str_signal.append(QString::fromStdString(current_signal.name()));
                str_signal.append(QString::number(current_signal.raw_value()));
                str_signal.append(QString::fromStdString(current_signal.phy_value()));

                str_signal_tooltips << "name: " + QString::fromStdString(current_signal.name())
                    << "raw_value: " + QString::number(current_signal.raw_value())
                    << "phy_value: " + QString::fromStdString(current_signal.phy_value());

                TraceTreeWidgetItem* item_signal = new TraceTreeWidgetItem(str_signal);
                item_signal->setSource("can_container_pdu_signal");
                item_signal->setToolTip(0, str_signal_tooltips.join("<br>"));
                item_pdu->addChild(item_signal);
            }
            Item->addChild(item_pdu);
        }
        emit popToRoot(Item);
        list_items_queue.enqueue(Item);
    }
}

void multiThread::formatRow_canframe_thread(can_frame frame)
{
    full_count_canframes++;
    QStringList can_list = {};
    QStringList filter_value = {};
    bool bFilter = false;
    last_timestamp_canframe = frame.Timestamp;

    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.Timestamp / 1000000);
    can_list.append(timestamp.toString("hh:mm:ss.zzz"));
    can_list.append("CAN " +QString::number(frame.Chn));
    can_list.append(QString::number(frame.ID, 16).toUpper());
    can_list.append("");

    QString sDir = "Rx";
    if (frame.Dir == "1") sDir = "Tx";
    can_list.append(sDir);

    can_list.append(QString::number(frame.DLC));
    can_list.append(frame.EventType);
    can_list.append(QString::number(frame.DataLength));
    can_list.append("CAN Message");

    QString myData = frame.Data_Str;
    can_list.append(myData);

    TraceTreeWidgetItem* Item = new TraceTreeWidgetItem(can_list);
    if (Item != nullptr) {
        UUIDv4::UUID uuid = uuidGenerator.getUUID();
        std::string s = uuid.str();
        QString uuid_str = QString::fromStdString(s);
        Item->setUUID(uuid_str);
        Item->setData(0, Qt::UserRole, uuid_str);
        emit(popToRoot(Item));
        list_items_queue.enqueue(Item);
    }
}

void multiThread::formatRow_linframe_thread(linMessage frame)
{
    /*
    eProsima_user_DllExport uint32_t channel() const;
    eProsima_user_DllExport uint32_t id() const;
    eProsima_user_DllExport uint8_t dlc() const;
    eProsima_user_DllExport const std::vector<uint8_t>& data() const;
    eProsima_user_DllExport const std::vector<uint8_t>& dataParser() const;
    eProsima_user_DllExport uint16_t flags() const;
    eProsima_user_DllExport uint8_t rxtx() const;
    eProsima_user_DllExport uint8_t errorCode() const;
    eProsima_user_DllExport uint8_t NMstate() const;
    eProsima_user_DllExport uint8_t isMasterFrame() const;
    */

    // { "Time[ms]", "Chn", "ID", "Name", "Dir", "DLC",  "EventType", "DataLength", "BusType", "Data" };
    full_count_linframes++;
    QStringList str_lm = {};
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(frame.timeStamp() / 1000000);
    str_lm.append(timestamp.toString("hh:mm:ss.zzz"));		// Timestamp
    str_lm.append("LIN " + QString::number(frame.channel()));	// Chn
    str_lm.append(QString::number(frame.id(), 16).toUpper());	// ID
    str_lm.append("");					// Name

    QString sDir = "Rx";
    if (frame.rxtx() == 1) sDir = "Tx";
    str_lm.append(sDir);				// Dir
    str_lm.append(QString::number(frame.dlc()));	// DLC
    str_lm.append("LIN Message");	// EventType
    str_lm.append(QString::number(frame.dlc())); // Datalength
    str_lm.append("LIN"); // bustype
    QByteArray output;
    std::copy(frame.data().begin(), frame.data().end(), std::back_inserter(output));
    QString myData = output.toHex(' ');
    str_lm.append(myData); //data

    UUIDv4::UUID uuid = uuidGenerator.getUUID();
    std::string s = uuid.str();
    QString uuid_str = QString::fromStdString(s);

    TraceTreeWidgetItem* Item = new TraceTreeWidgetItem(str_lm);
    Item->setSource("linmessage");
    Item->setUUID(uuid_str);
    Item->setData(0, Qt::UserRole, uuid_str);
    Item->setToolTip(0, "This is the second item.<br>Details: This item is less important.<br>Usage: Special cases.");

    if (Item == nullptr)
        return;
    else
    {
        emit(popToRoot(Item));
        list_items_queue.enqueue(Item);
    }
}

void multiThread::formatRow_linparser_thread(linFrames frames)
{
    /*  linFrame Definition
        |-------------------------+-----------------|
        | uint32_t                | channel()       |
        | uint32_t                | id()            |
        | uint8_t                 | dlc()           |
        | std::vector<uint8_t>&   | data()          |
        | uint16_t                | flags()         |
        | uint64_t                | timeStamp()     |
        | int64_t                 | timeStamp_d()   |
        | uint8_t                 | rxtx()          |
        | uint8_t                 | errorCode()     |
        | uint8_t                 | NMstate()       |
        | uint8_t                 | isMasterFrame() |
        | std::vector<linSignal>& | zone_signals()  |
        | std::string&            | name()          |
        |-------------------------+-----------------|
    */
    int frames_length = frames.len();
    qDebug() << "linFrames length ->" << frames_length;
 /*
    QStringList str_parser = {};
    linFrame first_frame = frames.linframes().at(0);
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(first_frame.timeStamp() / 1000000);
    str_parser.append(timestamp.toString("hh:mm:ss.zzz"));
    str_parser.append(""); // placeholder
    str_parser.append(""); // placeholder
    str_parser.append(""); // placeholder
    str_parser.append(""); // placeholder
    str_parser.append(""); // placeholder
    str_parser.append("LIN Frames");
    str_parser.append(""); // placeholder
    str_parser.append("LIN");

    UUIDv4::UUID uuid = uuidGenerator.getUUID();
    std::string s = uuid.str();
    QString uuid_str = QString::fromStdString(s);

    TraceTreeWidgetItem* Item = new TraceTreeWidgetItem(str_parser); // Frames level
    Item->setSource("linframes");
    Item->setUUID(uuid_str);
    Item->setData(0, Qt::UserRole, uuid_str);
    Item->setToolTip(0, "Lin Frames");
*/
    for (linFrame lf : frames.linframes()) // Frame level
    {
        full_count_linparser++;
        QStringList str_frame = {};
        QDateTime timestamp_frame = QDateTime::fromMSecsSinceEpoch(lf.timeStamp() / 1000000);
        str_frame.append(timestamp_frame.toString("hh:mm:ss.zzz"));
        str_frame.append("LIN " + QString::number(lf.channel()));
        str_frame.append(QString::number(lf.id(), 16).toUpper());
        str_frame.append(QString::fromStdString(lf.name()));
        QString sDir = "Rx";
        if (lf.rxtx() == 1) sDir = "Tx";
        str_frame.append(sDir);

        str_frame.append(QString::number(lf.dlc()));
        str_frame.append("LIN Frame");
        str_frame.append(QString::number(lf.dlc()));
        str_frame.append("LIN");

        QByteArray output;
        std::copy(lf.data().begin(), lf.data().end(), std::back_inserter(output));
        QString myData = output.toHex(' ');
        str_frame.append(myData);

        //str_frame.append(QString::number(lf.flags()));
        //str_frame.append(QString::number(lf.errorCode()));
        //str_frame.append(QString::number(lf.NMstate()));
        //str_frame.append(QString::number(lf.isMasterFrame()));
        //str_frame.append(QString::number(lf.zone_signals()));
        //str_frame.append(QString::number(lf.data()));

        UUIDv4::UUID uuid = uuidGenerator.getUUID();
        std::string s = uuid.str();
        QString uuid_str = QString::fromStdString(s);

        TraceTreeWidgetItem* Item_Frame = new TraceTreeWidgetItem(str_frame);
        Item_Frame->setSource("linframe");
        Item_Frame->setUUID(uuid_str);
        Item_Frame->setData(0, Qt::UserRole, uuid_str);
        Item_Frame->setToolTip(0, "Lin Frame");

        // since there is only one frame, frame should be at very top level. 2024-11-14 17:04:07
        //Item->addChild(Item_Frame);
        // signal level: 2024-11-14 16:14:31
        //str_frame.append(QString::number(lf.zone_signals()));

        for (linSignal signal : lf.zone_signals())
        {
            qDebug() << "Lin Frame Signal -> " << QString::fromStdString(signal.name());
            QStringList str_signal = {};
            str_signal.append(QString::fromStdString(signal.name()));
            str_signal.append(QString::number(signal.raw_value()));
            str_signal.append(QString::fromStdString(signal.phy_value()));
            str_signal.append(QString::fromStdString(signal.logical_value()));
            TraceTreeWidgetItem* Item_Signal = new TraceTreeWidgetItem(str_signal);
            Item_Signal->setSource("linsignal");
            Item_Signal->setUUID(uuid_str);
            Item_Signal->setData(0, Qt::UserRole, uuid_str);
            Item_Signal->setToolTip(0, "Lin Signal");
            Item_Frame->addChild(Item_Signal);
        }
        emit(popToRoot(Item_Frame));
        list_items_queue.enqueue(Item_Frame);
    }
    //if (Item == nullptr)
    //    return;
    //else
    //{
    //    emit(popToRoot(Item));
    //    list_items_queue.enqueue(Item);
    //}
}

void multiThread::setFilterOption(QString colName, QList<QList<QString>> items)
{
    filter_colName = colName;
    filter_items = items;
}

void multiThread::clear_items_queue()
{
    for (TraceTreeWidgetItem* it : list_items_queue)
    {
        QString myuuid = it->getUUID().toUpper();
        if (it->handled){}
        else {
            qDebug() << "NOT Handled!" << myuuid;
        }
        QString mysource = it->getSource().toUpper();
        //qDebug() << "ITEM SOURCE ->" << mysource;
        if (mysource == "CAN_PARSER") {
            int k = it->childCount();
            if (k > 0) {
                for (int i = 0; i <k; i++) {
                    TraceTreeWidgetItem* p = static_cast<TraceTreeWidgetItem*>(it->child(i));
                    if (p && p->getSource() == "can_pdu") {
                        for (int m = p->childCount() - 1; m >= 0; m--) {
                            TraceTreeWidgetItem* mc = static_cast<TraceTreeWidgetItem*>(p->child(m));
                            if (mc) {
                                delete mc;
                                mc = nullptr;
                            }
                        }
                        delete p;
                        p = nullptr;
                        continue;
                    }
                    else if (p && p->getSource() == "can_container_pdu") {
                        qDebug() << "container_pdu ->" << p->getUUID();
                        for (int m = p->childCount() - 1; m > 0; m--)
                        {
                            TraceTreeWidgetItem* mc = static_cast<TraceTreeWidgetItem*>(p->child(m));
                            if (mc) {
                                for (int n = mc->childCount() - 1; n > 0; n--) {
                                    TraceTreeWidgetItem* msignal = static_cast<TraceTreeWidgetItem*>(mc->child(n));
                                    delete msignal;
                                    msignal = nullptr;
                                }
                                delete mc;
                                mc = nullptr;
                            }
                        }
                        delete p;
                        p = nullptr;
                        continue;
                    }
                }
            }
        }
        if (mysource == "LINFRAME") {
            // todo: 2024-10-28 17:55:14
            qDebug() << "SOMETHING DELETED IN LINFRAME...";
            int k = it->childCount();
            if (k > 0) {
                for (int i = 0; i < k; i++) {
                    TraceTreeWidgetItem* p = static_cast<TraceTreeWidgetItem*>(it->child(i));
                    if (p && p->getSource() == "linsignal") {
                        delete p;
                        p = nullptr;
                        continue;
                    }
                }
            }
        }
        delete it;
        it = nullptr;
    }
    list_items_queue.clear();
}

void multiThread::formatRow_someip_calling_thread(someipFrame frame)
{
    qDebug() << "someip package calling ->" << frame.timeStamp() << " Source -> " << QString::fromStdString(frame.src_ip());
}

void multiThread::formatRow_someip_package_thread(someipFrame frame)
{
    qDebug() << "someip package frame ->" << frame.timeStamp() << " Source -> " << QString::fromStdString(frame.src_ip());
}

void multiThread::formatRow_someip_sd_thread(linMessage frame)
{
}

void multiThread::formatRow_someip_state_thread(linMessage frame)
{
}

void multiThread::formatRow_someip_eth_frame_thread(linMessage frame)
{
}
