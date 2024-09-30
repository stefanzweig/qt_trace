#pragma once
#include <qtreewidget.h>
#include <QQueue>
#include <QString>

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

const QString& typeToString(MyItemType type) {
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

	QVariant data(int column, int role) const override;
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

private:
	MyItemType item_type = MyItemType::canframe_;
	QString source_;
	QStringList original_data;
	bool changed = false;
	int ref_counter = 0;
};
