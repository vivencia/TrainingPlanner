#include "tpmessagesmodel.h"
#include "tpmessage.h"
#include "../tputils.h"

enum RoleNames {
	createRole(tpMessage, 0)
};

TPMessagesModel::TPMessagesModel(QObject *parent)
	: QAbstractItemModel{parent}, m_rootMessage{std::make_unique<TPMessage>(nullptr)}
{
	m_rootMessage->setObjectName("Root message");
	roleToString(tpMessage)
}

TPMessage *TPMessagesModel::findMessage(int field, const QVariant &field_value, const QLatin1StringView &type) const
{
	TPMessage *found_message{nullptr};
	const std::vector<std::unique_ptr<TPMessage>> &messages{m_rootMessage->children()};
	for (const auto &message : messages) {
		if (message->type() == type) {
			found_message = message->findChild(field_value, static_cast<TPMessage::TPMessageFields>(field));
			if (found_message) break;
		}
	}
	return found_message;
}

QList<TPMessage*> TPMessagesModel::findMessages(int field, const QVariant &field_value, const QLatin1StringView& type) const
{
	QList<TPMessage*> found_messages;
	const std::vector<std::unique_ptr<TPMessage>> &messages{m_rootMessage->children()};
	for (const auto &message : messages) {
		if (message->type() == type)
			found_messages.append(message.get());
	}
	return found_messages;
}

void TPMessagesModel::insertMessage(TPMessage *message, int row)
{
	const QModelIndex &parent_index{indexFromItem(message->parentMessage())};
	if (row == -1)
		row = message->parentMessage()->childCount();
	message->parentMessage()->insertChild(message, row);
	connect(message, &TPMessage::killMessage, this, [this,message] () { removeMessage(message); });
	static_cast<void>(insertRow(row, parent_index));
}

void TPMessagesModel::removeMessage(TPMessage *message)
{
	const QModelIndex &parent_index{createIndex(message->parentMessage()->row(), 0, message->parent())};
	beginRemoveRows(parent_index, message->row(), message->row());
	removeRow(message->row(), parent_index);
	message->parentMessage()->removeChild(message);
	endRemoveRows();
}

bool TPMessagesModel::insertRows(int row, int count, const QModelIndex &parent)
{
	TPMessage *parent_item{getItem(parent)};
	if (!parent_item)
		return false;
	beginInsertRows(parent, row, row + count - 1);
	endInsertRows();
	if (parent_item == m_rootMessage.get()) {
		if (rowCount(parent) == 1)
			emit hasMessageChanged();
	}
	return true;
}

bool TPMessagesModel::removeRows(int row, int count, const QModelIndex &parent)
{
	TPMessage *parent_item{getItem(parent)};
	if (!parent_item)
		return false;
	beginRemoveRows(parent, row, row + count - 1);
	endRemoveRows();
	if (parent_item == m_rootMessage.get()) {
		if (rowCount(parent) == 0)
			emit hasMessageChanged();
	}
	return true;
}

QVariant TPMessagesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != tpMessageRole)
		return QVariant{};
	const TPMessage *message{static_cast<const TPMessage*>(index.internalPointer())};
	return QVariant::fromValue(message);
}

Qt::ItemFlags TPMessagesModel::flags(const QModelIndex &index) const
{
	return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QModelIndex TPMessagesModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex{};
	TPMessage *parent_message{getItem(parent)};
	if (auto *child_item{parent_message->child(row)})
		return createIndex(row, column, child_item);
	return QModelIndex{};
}

QModelIndex TPMessagesModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex{};
	TPMessage *child_item{getItem(index)};
	TPMessage *parent_item{child_item->parentMessage()};
	if (!parent_item || parent_item == m_rootMessage.get())
		return QModelIndex{};
	return createIndex(parent_item->row(), 0, parent_item);
}

int TPMessagesModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;
	return getItem(parent)->childCount();
}

inline TPMessage *TPMessagesModel::getItem(const QModelIndex &index) const
{
	if (index.isValid()) {
		auto *item{static_cast<TPMessage*>(index.internalPointer())};
		if (item)
			return item;
	}
	return m_rootMessage.get();
}

inline QModelIndex TPMessagesModel::indexFromItem(TPMessage *message) const
{
	if (!message || message == m_rootMessage.get())
		return QModelIndex{};                 // root → invalid index
	// Walk up to build the correct index
	return createIndex(message->row(), 0, message);
}

inline TPMessage *TPMessagesModel::itemFromIndex(const QModelIndex &index) const
{
	return getItem(index);   // your existing helper
}
