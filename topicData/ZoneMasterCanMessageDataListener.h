#pragma once

#include "topicData/ZoneMasterDataPubSubTypes.h"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <QDebug>
#include <QThread>
#include <QtWidgets/QTreeView>
#include <QDateTime>
#include <QVector>
#include <QByteArray>
#include "..\zm_struct.h"

using namespace eprosima::fastdds::dds;


class SubListener : public QObject, public DataReaderListener
{
    Q_OBJECT
public:

    SubListener()
        : samples_(0)
    {
    }

    ~SubListener() override
    {
    }

    void on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info) override
    {
        if (info.current_count_change == 1)
        {
            qDebug() << "Subscriber matched.";
        }
        else if (info.current_count_change == -1)
        {
            qDebug() << "Subscriber unmatched.";
        }
        else
        {
            qDebug() << info.current_count_change;
        }
    }

    void on_data_available(
        DataReader* reader) override
    {
        SampleInfo info;
        if (reader->take_next_sample(&can_messages_, &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.valid_data)
            {
                samples_++;
                for (int i = 0; i < can_messages_.len(); i++ ) {
                    canMessage msg = can_messages_.canMsgs()[i];
                    QString repr = QString::number(msg.id());
                    QDateTime timestamp = QDateTime::fromSecsSinceEpoch(msg.timeStamp()/1000000000);
                    repr += "-" + timestamp.toString("yyyy-MM-dd hh:mm:ss");
                    can_frame cf;                    
                    cf.Timestamp = msg.timeStamp();
                    cf.Chn = msg.channel();
                    cf.ID = msg.id();
                    cf.Name = "";
                    cf.Dir = QString::number(msg.rxtx());
                    cf.DLC = msg.dlc();
                    QByteArray output;
                    std::copy(msg.data().begin(), msg.data().end(), std::back_inserter(output));
                    cf.Data_Str = output.toHex(' ');
                    cf.EventType = QString::number(msg.isFd());
                    cf.DataLength = msg.dataLen();
                    cf.BusType = "0";
                    emit traceItemUpdate_internal_cf(cf);
                }
            }
        }
    }

    void on_requested_incompatible_qos(
        DataReader* reader,
        const RequestedIncompatibleQosStatus& status)
    {
        (void)reader;
        (void)status;
    }

    canMessages can_messages_;


    std::atomic_int samples_;

    QThread* outerThread;
    QTreeView* tree_;

signals:
    void traceItemUpdate_internal(int i);
    void traceItemUpdate_internal_str(QString s);
    void traceItemUpdate_internal_cf(can_frame cf);
};