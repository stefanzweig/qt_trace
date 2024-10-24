#pragma once
#include <qtreewidget.h>
#include <QQueue>
#include <QString>
#include <QDebug>

enum MyItemType {
	canframe_,
	can_parser_,
	can_pdu_,
	can_pdu_signal_,
	can_container_pdu_,
	can_container_pdu_signal_,
	linframe_,
	lin_parser_,
	lin_pdu_,
	lin_pdu_signal_,
	lin_container_pdu_,
	lin_container_pdu_signal_
};

const QString& MyTypeToString(MyItemType type) {
	static const QString typeNames[] = {
		"canframe",
		"can_parser",
		"can_pdu",
		"can_pdu_signal",
		"can_container_pdu",
		"can_container_pdu_signal",
		"linframe",
		"lin_parser",
		"lin_pdu",
		"lin_pdu_signal",
		"lin_container_pdu",
		"lin_container_pdu_signal"
	};
	return typeNames[type];
}

//QTreeWidgetItem* deepCopyTreeWidgetItem(QTreeWidgetItem* originalItem) {
//	QTreeWidgetItem* copiedItem = new QTreeWidgetItem(*originalItem);
//
//	for (int i = 0; i < originalItem->childCount(); ++i) {
//		QTreeWidgetItem* childCopy = deepCopyTreeWidgetItem(originalItem->child(i));
//		copiedItem->addChild(childCopy);
//	}
//
//	return copiedItem;
//}


class TraceTreeWidgetItem :
	public QTreeWidgetItem
{
public:
	explicit TraceTreeWidgetItem(QTreeWidgetItem* parent = nullptr, int type = QTreeWidgetItem::Type) :
		QTreeWidgetItem(parent, type), source_("canframe") {}
	explicit TraceTreeWidgetItem(const QStringList& strings, int type = Type) :
		QTreeWidgetItem(strings, type),
		source_("canframe"),
		changed(false),
		ref_counter(0)
	{
		for (QString s : strings)
		{
			original_data.append(s);
		}
	}

	TraceTreeWidgetItem* clone() const {
		TraceTreeWidgetItem* newItem = new TraceTreeWidgetItem(*this);
		newItem->item_type = item_type;
		newItem->source_ = source_;
		newItem->changed = changed;
		newItem->ref_counter = 0;
		newItem->uuid_ = uuid_;
		//newItem->original_data.append(original_data);

		//if (newItem->source_ == "can_pdu") {
		//	int k = childCount();
		//	if (k > 0) {
		//		for (int i = 0; i < k; ++i) {
		//			TraceTreeWidgetItem* ch = static_cast<TraceTreeWidgetItem*>(child(i));
		//			qDebug() << "CHILD ->" << ch->uuid_;
		//		}
		//	}
		//}

		for (int i = 0; i < childCount(); ++i) {
			newItem->addChild(static_cast<TraceTreeWidgetItem*>(child(i))->clone());
		}
		//qDebug() << "Cloning SOURCE -> " << newItem->source_.toUpper();
		return newItem;
	}

	QVariant data(int column, int role) const override;
	
	bool operator==(const TraceTreeWidgetItem& other) const {
		bool equal = false;
		equal = (source_ == other.source_);
		equal = (item_type == other.item_type);
		equal = (ref_counter == other.ref_counter);
		equal = (uuid_ == other.uuid_);
		return equal;
	}

	void setSource(QString source) {
		source_ = source;
	}

	QString getSource() const {
		return source_;
	}

	void setDirty(bool b) {
		this->changed = b;
	}

	bool isDirty() const {
		return this->changed;
	}

	QStringList get_original_data() const
	{
		return original_data;
	}

	void inc_ref()
	{
		ref_counter++;
	}

	int get_ref() const
	{
		return ref_counter;
	}

	QString getUUID() const
	{
		return uuid_;
	}

	void setUUID(QString uuid)
	{
		this->uuid_ = uuid;
	}

	bool handled = false;

private:
	MyItemType item_type = MyItemType::canframe_;
	QString source_, uuid_;
	QStringList original_data;
	bool changed = false;
	int ref_counter = 0;
};
