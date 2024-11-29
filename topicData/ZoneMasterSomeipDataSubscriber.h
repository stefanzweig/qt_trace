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
	Subscriber* subscriber_;
	DataReader* reader_;
	Topic* topic_;
	TypeSupport type_;
	int domainid = 90;
public:
	ZoneMasterSomeipSubscriber(int domainid)
		: participant_(nullptr)
		, subscriber_(nullptr)
		, topic_(nullptr)
		, reader_(nullptr)
		, type_(nullptr)
	{
		someipFramePubSubType* someip_framepubsubtype = new someipFramePubSubType();
		someip_framepubsubtype->setName("SomeIpParserData");
		type_ = TypeSupport(someip_framepubsubtype);
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
		DomainParticipantFactory::get_instance()->delete_participant(participant_);
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
