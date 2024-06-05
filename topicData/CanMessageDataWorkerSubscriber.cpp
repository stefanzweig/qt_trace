#include "topicData/CanMessageDataPubSubTypes.h"
#include "CanMessageDataWorkerSubscriber.h"
#include "topicData/BaseNodeData.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

bool CanMessageDataWorkerSubscriber::init()
{
    DomainParticipantQos participantQos;
    participantQos.name("Participant_subscriber");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(DDS_DOMAINID, participantQos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the Type
    type_.register_type(participant_);

    // Create the subscriptions Topic
     topic_ = participant_->create_topic("CanMessageDataTopic", type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    //topic_ = participant_->create_topic("canMessageTopic", type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

    if (subscriber_ == nullptr)
    {
        return false;
    }

    // Create the DataReader
    reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);

    if (reader_ == nullptr)
    {
        return false;
    }

    return true;
}

void CanMessageDataWorkerSubscriber::run(uint32_t samples)
{
    //while (1) 
    //{
        while (listener_.samples_ < samples)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    //}
}

void CanMessageDataWorkerSubscriber::setOuterThread(QThread* thread, QTreeView* treeview)
{
    this->listener_.outerThread = thread;
    this->listener_.tree_ = treeview;
}

// in the commandline 
//int main(
//    int argc,
//    char** argv)
//{
//    std::cout << "Starting subscriber." << std::endl;
//    uint32_t samples = 10;
//
//    CanMessageDataWorkerSubscriber* mysub = new CanMessageDataWorkerSubscriber();
//    if (mysub->init())
//    {
//        mysub->run(samples);
//    }
//
//    delete mysub;
//    return 0;
//}
