#ifndef DBUSERMODEL_H
#define DBUSERMODEL_H

#include "tplistmodel.h"

#define USER_COL_ID 0
#define USER_COL_NAME 1
#define USER_COL_BIRTHDAY 2
#define USER_COL_SEX 3
#define USER_COL_CONTACT 4
#define USER_COL_ROLE 5
#define USER_COL_GOAL 6
#define USER_COL_COACH 7

class DBUserModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:

	enum RoleNames {
		idRole = Qt::UserRole,
		nameRole = Qt::UserRole+USER_COL_NAME,
		birthdayRole = Qt::UserRole+USER_COL_BIRTHDAY,
		sexRole = Qt::UserRole+USER_COL_SEX,
		contactRole = Qt::UserRole+USER_COL_CONTACT,
		roleRole = Qt::UserRole+USER_COL_ROLE,
		goalRole = Qt::UserRole+USER_COL_GOAL,
		coachRole = Qt::UserRole+USER_COL_COACH
	};

	explicit DBUserModel(QObject *parent = 0);

	virtual bool updateFromModel(const TPListModel* model) override;
	virtual void exportToText(QFile* outFile, const bool bFancy) const override;
	virtual bool importFromFancyText(QFile* inFile, QString& inData) override;

	Q_INVOKABLE int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return 8; }
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:

};
#endif // DBUSERMODEL_H
