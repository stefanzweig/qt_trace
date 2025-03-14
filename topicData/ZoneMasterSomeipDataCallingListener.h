#pragma once

#include "topicData/ZoneMasterDataPubSubTypes.h"

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
#include <QDateTime>
#include <QVector>
#include <QByteArray>
#include "../zm_struct.h"

using namespace eprosima::fastdds::dds;

class SomeipCallingListener : public virtual QObject, public DataReaderListener
{
    Q_OBJECT
public:

    SomeipCallingListener()
    {
    }

    ~SomeipCallingListener()
    {
    }

    void on_subscription_matched(DataReader*,const SubscriptionMatchedStatus& info) override
    {
        if (info.current_count_change == 1)
            {
                qDebug() << "Calling Subscriber matched.";
            }
        else if (info.current_count_change == -1)
            {
                qDebug() << "Calling Subscriber unmatched.";
            }
        else
            {
                qDebug() << info.current_count_change;
            }
    }

    void on_data_available(DataReader* reader) override
    {
        SampleInfo info;
        if (reader->take_next_sample(&someip_calling, &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.valid_data)
            {
                samples_++;
                emit ItemUpdate_internal_someip_calling(someip_calling);
            }
        }
    }

    someipFrame someip_calling;
    std::atomic_int samples_;
    QThread* outerThread = nullptr;
    QTreeView* tree_ = nullptr;

signals:
    void ItemUpdate_internal_someip_calling(someipFrame sf);
};
