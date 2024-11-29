#pragma once

#include "topicData/ZoneMasterDataPubSubTypes.h"
#include "topicData/ZoneMasterSomeipDataCallingListener.h"
#include "topicData/ZoneMasterSomeipDataEthFrameListener.h"
#include "topicData/ZoneMasterSomeipDataPackageListener.h"
#include "topicData/ZoneMasterSomeipDataSDListener.h"
#include "topicData/ZoneMasterSomeipDataStateListener.h"

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
class ZoneMasterSomeipSubscriber
{
private:

	DomainParticipant* participant_;
	DomainParticipant* participant_calling;
	Subscriber* subscriber_;
	Subscriber* subscriber_calling;
	DataReader* reader_;
	DataReader* reader_calling;
	Topic* topic_;
	Topic* topic_calling;
	TypeSupport type_;
	TypeSupport type_calling;
	int domainid = 90;
public:
	ZoneMasterSomeipSubscriber(int domainid)
		: participant_(nullptr)
		, participant_calling(nullptr)
		, subscriber_(nullptr)
		, subscriber_calling(nullptr)
		, topic_(nullptr)
		, topic_calling(nullptr)
		, reader_(nullptr)
		, reader_calling(nullptr)
		, type_(nullptr)
		, type_calling(nullptr)
	{
		someipFramePubSubType* someip_framepubsubtype = new someipFramePubSubType();
		someip_framepubsubtype->setName("SomeIpParserData");

		someipFramePubSubType* someip_framepubsubtype_calling = new someipFramePubSubType();
		someip_framepubsubtype_calling->setName("SomeIpParserData");

		type_ = TypeSupport(someip_framepubsubtype);
		type_calling = TypeSupport(someip_framepubsubtype_calling);
		this->domainid = domainid;
	}

	virtual ~ZoneMasterSomeipSubscriber()
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

		if (reader_calling != nullptr)
		{
			subscriber_calling->delete_datareader(reader_calling);
		}
		if (topic_calling != nullptr)
		{
			participant_calling->delete_topic(topic_calling);
		}
		if (subscriber_calling != nullptr)
		{
			participant_calling->delete_subscriber(subscriber_calling);
		}

		DomainParticipantFactory::get_instance()->delete_participant(participant_);
		DomainParticipantFactory::get_instance()->delete_participant(participant_calling);
	}

	bool init();
	void run(uint32_t samples);
	void setOuterThread(QThread* thread, QTreeView* treeview);

	SomeipPackageListener listener_package;
	SomeipCallingListener listener_calling;
	SomeipStateListener listener_state;
	SomeipEthFrameListener listener_eth_frame;
	SomeipSDListener listener_sd;
};
