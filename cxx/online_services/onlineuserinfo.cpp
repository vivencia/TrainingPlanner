#include "onlineuserinfo.h"

OnlineUserInfo::OnlineUserInfo(
	QObject *parent)
	: QAbstractListModel{parent}
{
	m_roleNames[IDRole] = std::move("id");
	m_roleNames[nameRole] = std::move("name");
}
