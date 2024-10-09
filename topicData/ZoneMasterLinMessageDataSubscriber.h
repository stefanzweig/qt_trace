#pragma once
#include "topicData/ZoneMasterDataPubSubTypes.h"
#include "topicData/ZoneMasterLinMessageDataListener.h"
#include "topicData/ZoneMasterCanMessageDataListener.h"

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

using namespace eprosima::fastdds::dds;

class ZoneMasterLinMessageDataSubscriber
{
private:

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    DataReader* reader_;

    Topic* topic_;

    int domainid = 90;

    TypeSupport type_;

public:
    ZoneMasterLinMessageDataSubscriber(int domainid = 90)
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(nullptr)
    {
        linMessagesPubSubType* lin_messages_pubsubtype = new linMessagesPubSubType();
        lin_messages_pubsubtype->setName("linMessageData");
        type_ = TypeSupport(lin_messages_pubsubtype);
        domainid = domainid;
    }

    virtual ~ZoneMasterLinMessageDataSubscriber()
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
        if (participant_) {
            participant_->delete_contained_entities();
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

    bool init();
    void run(uint32_t samples);
    void setOuterThread(QThread* thread, QTreeView* treeview);
    LinSubListener listener_;
};

