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

void setQOS(DomainParticipantQos& pqos) {
    eprosima::fastrtps::rtps::Locator_t pdp_locator;
    eprosima::fastrtps::rtps::Locator_t tcp_locator;
    pdp_locator.kind = LOCATOR_KIND_UDPv4;
    //eprosima::fastrtps::rtps::IPLocator::setIPv4(pdp_locator, "239.255.0.1");
    // Define locator for EDP and user data
    tcp_locator.kind = LOCATOR_KIND_TCPv4;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(tcp_locator, "0.0.0.0");
    //eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(tcp_locator, tcp_listening_port);
    eprosima::fastrtps::rtps::IPLocator::setLogicalPort(tcp_locator, 0);

    pqos.wire_protocol().builtin.metatrafficMulticastLocatorList.push_back(pdp_locator);
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(tcp_locator);
    pqos.wire_protocol().default_unicast_locator_list.push_back(tcp_locator);
}

bool CanMessageDataWorkerSubscriber::init()
{
    DomainParticipantQos participantQos = PARTICIPANT_QOS_DEFAULT;
    participantQos.name("Participant_subscriber_Stefan");

    auto factory = DomainParticipantFactory::get_instance();
    factory->load_profiles();
    factory->get_default_participant_qos(participantQos);

    setQOS(participantQos);

    participant_ = factory->create_participant(DDS_DOMAINID, participantQos);

    if (participant_ == nullptr)
    {
        return false;
    }

    // Register the Type
    auto debug1 = type_.register_type(participant_);

    // Create the subscriptions Topic

    topic_ = participant_->create_topic("canMessageTopic", "canMessageData", TOPIC_QOS_DEFAULT);
    //topic_ = participant_->create_topic("canMessageTopic", type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);


    if (topic_ == nullptr)
    {
        return false;
    }

    // Create the Subscriber
    SubscriberQos subscriber_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(subscriber_qos);

    //subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    subscriber_ = participant_->create_subscriber(subscriber_qos, nullptr);

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
