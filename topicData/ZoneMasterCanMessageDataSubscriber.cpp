#include "ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/BaseNodeData.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

bool ZoneMasterCanMessageDataSubscriber::init()
{
    DomainParticipantQos participantQos;
    participantQos.name("Participant_ZWEIG");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(90, participantQos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the Type
    type_.register_type(participant_);

    // Create the subscriptions Topic
    topic_ = participant_->create_topic(
        //"ZoneMasterDataTopic", 
        "canMessageTopic",
        type_.get_type_name(), 
        TOPIC_QOS_DEFAULT);
        //eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // Create the Subscriber
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(subscriber_qos);

    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    //subscriber_ = participant_->create_subscriber(subscriber_qos, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // Create the DataReader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    reader_qos.reliable_reader_qos();
    subscriber_->set_default_datareader_qos(reader_qos);

    //reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
    reader_ = subscriber_->create_datareader(topic_, reader_qos, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;

}

void ZoneMasterCanMessageDataSubscriber::run(uint32_t samples)
{
    while (listener_.samples_ < samples)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ZoneMasterCanMessageDataSubscriber::setOuterThread(QThread* thread, QTreeView* treeview)
{
    this->listener_.outerThread = thread;
    this->listener_.tree_ = treeview;
}