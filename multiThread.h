#pragma once
#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include <QThread>
#include <QMutex>
#include "topicData/ZoneMasterCanMessageDataListener.h"
#include "topicData/ZoneMasterCanMessageDataSubscriber.h"
#include "topicData/ZoneMasterCanParserListener.h"
#include "topicData/ZoneMasterCanParserSubscriber.h"

#include "topicData/ZoneMasterLinMessageDataSubscriber.h"
#include "topicData/ZoneMasterLinParserDataSubscriber.h"
#include "topicData/ZoneMasterLinMessageDataListener.h"
#include "topicData/ZoneMasterLinParserDataListener.h"
#include "topicData/ZoneMasterSomeipDataSubscriber.h"

#include <QtWidgets/QTreeView>
#include <QTreeWidgetItem>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "uuid_v4.h"
#include "TraceTreeWidgetItem.h"
#include "json.hpp"

class multiThread :
	public QThread
    // , public QObject
    // , public SubListener
{
	Q_OBJECT

public:
	multiThread();
	~multiThread();
	void setCanSubscriber(ZoneMasterCanMessageDataSubscriber* subscriber, int samples, QTreeView* treeview);
	void setCanParserSubscriber(ZoneMasterCanParserSubscriber* subscriber, int samples, QTreeView* treeview);
	void setLinSubscriber(ZoneMasterLinMessageDataSubscriber* subscriber, int samples, QTreeView* treeview);
	void setLinParserSubscriber(ZoneMasterLinParserSubscriber* subscriber, int samples, QTreeView* treeview);
	void setSomeipSubscriber(ZoneMasterSomeipSubscriber* subscriber, int samples, QTreeView* treeview);
	void setFilterOption(QString colName, QList<QList<QString>> items);
	void clear_items_queue();

	uint64_t full_count_canparser = 0;
	uint64_t full_count_canframes = 0;
	uint64_t full_count_linparser = 0;
	uint64_t full_count_linframes = 0;
	uint64_t full_count_someip_package = 0;
	uint64_t full_count_someip_calling = 0;
	uint64_t full_count_someip_sd = 0;
	uint64_t full_count_someip_state = 0;
	uint64_t full_count_someip_eth = 0;

	bool isSTOPPED() { return is_stop; };
	bool isPAUSED() { return is_paused; }
	bool isRUN() { return (!is_stop && !is_paused); }
	void reset_counters();

	QList<QString> monitor_modules;
	QQueue<TraceTreeWidgetItem*>* queue_ = nullptr;
	QQueue<TraceTreeWidgetItem*> list_items_queue;


public slots:
	void stopThread();
	void stopFlag();
	void restartThread();
	void pauseThread();
	void setQueryString(QString str);
	static std::time_t get_timestamp();

protected:
	virtual void run() override;

private:
	bool is_stop = true;
	bool is_paused = false;
	QMutex m_lock;
	std::string m_last_realtime_id = "-1";
	std::string m_from_id = "-1";
	time_t from_t = 0;
	time_t to_t = 0;
	std::string to_id = "-1";
	QString query_string;
	int samples_ = 100;
	ZoneMasterCanMessageDataSubscriber* mysub_can_frames = nullptr;
	ZoneMasterCanParserSubscriber* mysub_can_parser = nullptr;
	ZoneMasterLinMessageDataSubscriber* mysub_lin_frames = nullptr;
	ZoneMasterLinParserSubscriber* mysub_lin_parser = nullptr;
	ZoneMasterSomeipSubscriber* mysub_someip = nullptr;

	QTreeView* tree_ = nullptr;
	QVector<canframe> full_canparserdata;
	QVector<can_frame> full_canframes;
	uint64_t last_timestamp_canparser = 0;
	uint64_t last_timestamp_canframe = 0;
	QString filter_colName;
	QList<QList<QString>> filter_items;

	bool bconnected_cf = false;
	bool bconnected_cp = false;
	bool bconnected_lf = false;
	bool bconnected_lp = false;
	bool bconnected_someip = false;
	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;

	void bindDataToTraceTree();
	void formatRow_canparser_thread(canframe frame);
	void formatRow_canframe_thread(can_frame frame);
	void formatRow_linparser_thread(linFrames frames);
	void formatRow_linframe_thread(linMessage frame);

	void formatRow_someip_calling_thread(someipFrame frame);
	void formatRow_someip_package_thread(someipFrame frame);
	void formatRow_someip_sd_thread(someipFrame frame);
	void formatRow_someip_state_thread(someipFrame frame);
	void formatRow_someip_eth_frame_thread(someipFrame frame);

	void construct_someip_frame(someipFrame frame, QString frame_type);
	void parseJsonAndAddToTree(QTreeWidgetItem* parentItem, const QJsonValue& jsonValue, const QString& keyPrefix = "");

signals:
	void popToRoot(TraceTreeWidgetItem* item);
};

#endif // MULTITHREAD_H
