#pragma once
#include "topicData/ZoneMasterDataPubSubTypes.h"
#include "topicData/ZoneMasterLinParserDataListener.h"
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
class ZoneMasterLinParserSubscriber
{
private:

    DomainParticipant* participant_;

    Subscriber* subscriber_;

    DataReader* reader_;

    Topic* topic_;

    TypeSupport type_;
    int domainid = 90;
public:
    ZoneMasterLinParserSubscriber(int domainid)
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(nullptr) // this should be matched.
    {
        linFramesPubSubType* lin_frames_pubsubtype = new linFramesPubSubType();
        lin_frames_pubsubtype->setName("linParserData");
        type_ = TypeSupport(lin_frames_pubsubtype);
        this->domainid = domainid;
    }

    virtual ~ZoneMasterLinParserSubscriber()
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
        //DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init();

    void run(uint32_t samples);
    void setOuterThread(QThread* thread, QTreeView* treeview);
    LinParserListener listener_;

};
