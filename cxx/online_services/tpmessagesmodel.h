#pragma once

#include <QAbstractItemModel>
#include <QtQml/qqml.h>

QT_FORWARD_DECLARE_CLASS(TPMessage)

class TPMessagesModel : public QAbstractItemModel
{

Q_OBJECT
QML_VALUE_TYPE(TPMessageModel)

Q_PROPERTY(bool hasMessage READ hasMessage NOTIFY hasMessageChanged FINAL)
public:
	explicit TPMessagesModel(QObject *parent = nullptr);

	inline TPMessage *rootMessage() const { return m_rootMessage.get(); }
	TPMessage *findMessage(int field, const QVariant &field_value, const QLatin1StringView& type) const;
	QList<TPMessage*> findMessages(int field, const QVariant &field_value, const QLatin1StringView& type) const;
	void insertMessage(TPMessage *message, int row = -1);
	void removeMessage(TPMessage *message);

	bool hasMessage() const { return rowCount() > 0; }
	bool insertRows(int row, int count, const QModelIndex &parent) override;
	bool removeRows(int row, int count, const QModelIndex &parent) override;
	QVariant data(const QModelIndex &index, int role) const override final;
	inline bool setData(const QModelIndex &index, const QVariant &value, int role) override final { return false; }
	Qt::ItemFlags flags(const QModelIndex &index) const override final;
	QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override final;
	inline QVariant headerData(int section, Qt::Orientation orientation, int role) const override final { return {}; }
	QModelIndex parent(const QModelIndex &index) const override final;
	int rowCount(const QModelIndex &parent = {}) const override final;
	inline int columnCount(const QModelIndex &parent = {}) const override final { Q_UNUSED(parent); return 1; }
	inline QHash<int, QByteArray> roleNames() const override final { return m_roleNames; }

signals:
	void hasMessageChanged();

private:
	QHash<int, QByteArray> m_roleNames;
	std::unique_ptr<TPMessage> m_rootMessage;

	TPMessage *getItem(const QModelIndex &index) const;
	QModelIndex indexFromItem(TPMessage *message) const;
	TPMessage *itemFromIndex(const QModelIndex &index) const;
};
