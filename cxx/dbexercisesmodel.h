#ifndef DBEXERCISESMODEL_H
#define DBEXERCISESMODEL_H

#include "tplistmodel.h"

class DBExercisesModel : public TPListModel
{

Q_OBJECT
QML_ELEMENT

public:	
	// Define the role names to be used
	enum RoleNames {
		exerciseIdRole = Qt::UserRole,
		mainNameRole = Qt::UserRole+1,
		subNameRole = Qt::UserRole+2,
		muscularGroupRole = Qt::UserRole+3,
		nSetsRole = Qt::UserRole+4,
		nRepsRole = Qt::UserRole+5,
		nWeightRole = Qt::UserRole+6,
		uWeightRole = Qt::UserRole+7,
		mediaPathRole = Qt::UserRole+8,
		actualIndexRole = Qt::UserRole+9
	};

	explicit DBExercisesModel(QObject *parent = 0);

	Q_INVOKABLE const QString& data(const uint row, int role) const;
	Q_INVOKABLE bool setData(const uint row, const QString& value, int role);
};

#endif // DBEXERCISESMODEL_H
