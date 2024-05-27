#include "multiThread.h"
#include <QDebug>

multiThread::multiThread()
    :is_stop(false),is_paused(false)
{
}

multiThread::~multiThread() {}

void multiThread::stopThread() {
    is_stop = true;
    is_paused = false;

    if (mysub_ != nullptr) {
        delete mysub_;
        mysub_ = nullptr;
    }
}

void multiThread::pauseThread() {
    if (!is_stop) {
        is_paused = true;
    }
}

void multiThread::restartThread() {
    is_stop = false;
    is_paused = false;
}

void multiThread::run() {
    // from_t = get_timestamp();

    if (mysub_ != nullptr) 
        if (mysub_->init())
            mysub_->run(samples_);

    while (true) {
        if (!is_stop) {
            qDebug() << "running...";
            msleep(2000); // every 2 seconds
        }
        else {
            break;
        }
    }
}

std::time_t multiThread::get_timestamp()
{
    std::chrono::time_point<std::chrono::system_clock,
        std::chrono::microseconds> tp =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::time_t timestamp = tp.time_since_epoch().count() * 1000;
    return timestamp;
}

void multiThread::bindDataToTraceTree()
{

    try {
        m_from_id = to_id;
    }
    catch (...) {
        qDebug() << "mongodb problem from " << from_t << " to " << to_t;
    }

    try {
            m_from_id = to_id;

            m_lock.unlock();
            msleep(20);
    }
    catch (...) {
        qDebug() << "handling result problem.";
    }
}

void multiThread::setQueryString(QString str)
{
    this->query_string = str;
}

void multiThread::setSubscriber(CanMessageDataWorkerSubscriber* subscriber, int samples)
{
    mysub_ = subscriber;
    samples_ = samples;
}