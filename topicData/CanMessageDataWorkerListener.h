#pragma once

#include "topicData/CanMessageDataPubSubTypes.h"

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
                //std::cout << " with Length: " << can_messages_.len()
                //    << " RECEIVED." << std::endl;
                //qDebug() << " with Length: " << can_messages_.len() << " RECEIVED.";
                emit traceItemUpdate_internal(can_messages_.len());
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
    void traceItemUpdate_internal(int i);/* {
        qDebug() << i;
    };*/
};