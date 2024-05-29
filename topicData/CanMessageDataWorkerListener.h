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
            //std::cout << "Subscriber matched." << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            qDebug() << "Subscriber unmatched.";
            //std::cout << "Subscriber unmatched." << std::endl;
        }
        else
        {
            qDebug() << info.current_count_change;
            //std::cout << info.current_count_change
                //<< " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
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
                qDebug() << " with Length: " << can_messages_.len() << " RECEIVED.";
                emit traceItemUpdate_internal(0);
            }
        }
    }

    canMessages can_messages_;


    std::atomic_int samples_;

    QThread* outerThread;
    QTreeView* tree_;

signals:
    void traceItemUpdate_internal(int i){
        qDebug() << i;
    };
};