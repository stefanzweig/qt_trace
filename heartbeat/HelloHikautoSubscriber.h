// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file HelloHikautoSubscriber.h
 * This header file contains the declaration of the subscriber functions.
 *
 * This file was generated by the tool fastddsgen.
 */


#ifndef _FAST_DDS_GENERATED_HELLOHIKAUTO_SUBSCRIBER_H_
#define _FAST_DDS_GENERATED_HELLOHIKAUTO_SUBSCRIBER_H_

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include "../DataEmitter.h"

class HelloHikautoSubscriber
{
public:

    HelloHikautoSubscriber();

    virtual ~HelloHikautoSubscriber();

    bool init();

    void run();

    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        //SubListener() { this->emitter_ = new DataEmitter(); }
        SubListener() = default;

        ~SubListener() override = default;

        void setEmmiter(DataEmitter* emmitter) { emitter_ = emmitter; }

        void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        int matched = 0;
        uint32_t samples = 0;
    private:
        DataEmitter* emitter_;
    }
    listener_;


private:

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::TypeSupport type_;
};

#endif // _FAST_DDS_GENERATED_HELLOHIKAUTO_SUBSCRIBER_H_