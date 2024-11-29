#include "ZoneMasterSomeipDataSubscriber.h"
#include "topicData/BaseNodeData.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

bool ZoneMasterSomeipSubscriber::init()
{
    DomainParticipantQos participantQos;
    DomainParticipantQos participantQos_calling;
    participantQos.name("PARTICIPANT_SOMEIP_ZWEIG");
    participantQos_calling.name("PARTICIPANT_SOMEIP_CALLING_ZWEIG");
    DomainParticipantFactory*  factory = DomainParticipantFactory::get_instance();
    if (factory)
    {
        participant_ = factory->create_participant(domainid, participantQos);
        participant_calling = factory->create_participant(domainid, participantQos_calling);
    }

    if (participant_ == nullptr)
    {
        return false;
    }
    if (participant_calling == nullptr)
    {
        return false;
    }

    // Register the Type
    participant_->register_type(type_);
    participant_calling->register_type(type_calling);

    TopicQos tqos;
    TopicQos tqos_calling;
    participant_->get_default_topic_qos(tqos);
    participant_calling->get_default_topic_qos(tqos_calling);

    topic_ = participant_->create_topic(
        "someipPackageTopic",
        type_.get_type_name(),
        tqos);
    if (topic_ == nullptr)
    {
        return false;
    }

    topic_calling = participant_calling->create_topic(
        "someIpCallingTopic",
        type_calling.get_type_name(),
        tqos_calling);
    if (topic_calling == nullptr)
    {
        return false;
    }

    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    SubscriberQos subscriber_qos_calling = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(subscriber_qos);
    participant_calling->get_default_subscriber_qos(subscriber_qos_calling);

    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    subscriber_calling = participant_calling->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_calling == nullptr)
    {
        return false;
    }

    // Create the DataReader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    DataReaderQos reader_qos_calling = DATAREADER_QOS_DEFAULT;
    reader_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    reader_qos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    reader_qos.data_sharing().automatic();

    reader_qos_calling.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    reader_qos_calling.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    reader_qos_calling.data_sharing().automatic();

    subscriber_->set_default_datareader_qos(reader_qos);
    reader_ = subscriber_->create_datareader(topic_, reader_qos, &listener_package);
    if (reader_ == nullptr)
    {
        return false;
    }

    subscriber_calling->set_default_datareader_qos(reader_qos_calling);
    reader_calling = subscriber_calling->create_datareader(topic_calling, reader_qos_calling, &listener_calling);
    if (reader_calling == nullptr)
    {
        return false;
    }
    return true;
}

void ZoneMasterSomeipSubscriber::run(uint32_t samples)
{
    while (listener_package.samples_ < samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    while (listener_calling.samples_ < samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ZoneMasterSomeipSubscriber::setOuterThread(QThread* thread, QTreeView* treeview)
{
    listener_package.outerThread = thread;
    listener_calling.outerThread = thread;
    listener_state.outerThread = thread;
    listener_sd.outerThread = thread;
    listener_eth_frame.outerThread = thread;

    listener_package.tree_ = treeview;
    listener_calling.tree_ = treeview;
    listener_state.tree_ = treeview;
    listener_sd.tree_ = treeview;
    listener_eth_frame.tree_ = treeview;
}
