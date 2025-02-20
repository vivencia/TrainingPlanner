#ifndef ONLINEUSERINFO_H
#define ONLINEUSERINFO_H

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

class OnlineUserInfo : public QAbstractListModel
{

Q_OBJECT
QML_ELEMENT

Q_PROPERTY(uint count READ count NOTIFY countChanged)

enum RoleNames {
	IDRole = Qt::UserRole,
	nameRole = Qt::UserRole+1
};

public:
	explicit OnlineUserInfo(QObject *parent = nullptr);

private:
	QHash<int, QByteArray> m_roleNames;
};

#endif // ONLINEUSERINFO_H
