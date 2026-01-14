#pragma once

#include "tpdatabasetable.h"

#include <QObject>

QT_FORWARD_DECLARE_CLASS(DBModelInterfaceUser)

class DBUserTable final: public TPDatabaseTable
{

Q_OBJECT

public:
	explicit DBUserTable(DBModelInterfaceUser *dbmodel_interface);

	QString dbFileName(const bool fullpath = true) const override final;
	void updateTable() override final {}
	bool getAllUsers(void *);

signals:
	void userInfoAcquired(QStringList user_info, const bool all_info_acquired = false);
};

