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


class LinParserListener : public QObject, public DataReaderListener
{
    Q_OBJECT
public:

    LinParserListener()
        : samples_(0)
    {
    }

    ~LinParserListener() override
    {
    }

    void on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info) override
    {
        if (info.current_count_change == 1)
        {
            qDebug() << "LinParserSubscriber matched.";
        }
        else if (info.current_count_change == -1)
        {
            qDebug() << "LinParserSubscriber unmatched.";
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
        if (reader->take_next_sample(&linframe_, &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.valid_data)
            {
                samples_++;
                emit traceItemUpdate_internal_linparser(linframe_);
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

    //linMessages lin_messages_;
    linFrame linframe_;


    std::atomic_int samples_;

    QThread* outerThread;
    QTreeView* tree_;

signals:
    void traceItemUpdate_internal_linparser(linFrame lin_frame);
};