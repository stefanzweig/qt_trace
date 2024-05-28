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

class CanMessageDataWorkerSubscriber
{
private:

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    DataReader* reader_;

    Topic* topic_;

    TypeSupport type_;


    class SubListener : public DataReaderListener, public QObject
    {
    public:

        SubListener()
            : samples_(0)
        {
            /*connect(this, SIGNAL(traceItemUpdate()), outerThread, SIGNAL(traceItemUpdate()));*/
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
                    emit traceItemUpdate();
                }
            }
        }

        canMessages can_messages_;


        std::atomic_int samples_;

        QThread* outerThread;
        QTreeView* tree_;

    signals:
        void traceItemUpdate() {};


    } listener_;

public:
    CanMessageDataWorkerSubscriber()
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new canMessagesPubSubType()) // this should be matched.
    {
    }

    virtual ~CanMessageDataWorkerSubscriber()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init();

    void run(uint32_t samples);

    void setOuterThread(QThread* thread, QTreeView* treeview);

};

