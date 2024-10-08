#include "topicData/ZoneMasterLinMessageDataSubscriber.h"
#include "topicData/ZoneMasterLinMessageDataListener.h"
#include "topicData/BaseNodeData.h"
#include "topicData/ZoneMasterLinMessageDataSubscriber.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>

bool ZoneMasterLinMessageDataSubscriber::init()
{
    DomainParticipantQos participantQos;
    participantQos.name("Participant_Lin_ZWEIG");
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    if (factory)
        participant_ = factory->create_participant(domainid, participantQos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the Type
    participant_->register_type(type_);

    TopicQos tqos;
    participant_->get_default_topic_qos(tqos);

    // Create the subscriptions Topic
    topic_ = participant_->create_topic(
        "linMessageTopic",
        type_.get_type_name(),
        tqos);
    if (topic_ == nullptr)
    {
        return false;
    }

    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(subscriber_qos);
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    reader_qos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    reader_qos.data_sharing().automatic();
    subscriber_->set_default_datareader_qos(reader_qos);

    reader_ = subscriber_->create_datareader(topic_, reader_qos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }
    return true;
}

void ZoneMasterLinMessageDataSubscriber::run(uint32_t samples)
{
}

void ZoneMasterLinMessageDataSubscriber::setOuterThread(QThread* thread, QTreeView* treeview)
{
    this->listener_.outerThread = thread;
    this->listener_.tree_ = treeview;
}