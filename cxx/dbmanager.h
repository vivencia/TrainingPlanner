#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "dbexerciseslist.h"
#include "dbexercisesmodel.h"

#include <QQmlEngine>
#include <QSettings>
#include <QQmlApplicationEngine>

class DbManager : public QObject
{

Q_OBJECT

public:
	explicit DbManager(const QString& dbFilename, QSettings* appSettigs, QQmlApplicationEngine* QMlEngine);

	//--------------------EXERCISES TABLE---------------------------------
	Q_INVOKABLE void getAllExercises();
	Q_INVOKABLE void updateExercisesList();
	Q_INVOKABLE void newExercise();
	Q_INVOKABLE void updateExercise();
	//--------------------EXERCISES TABLE---------------------------------

private:
	QString m_DBFileName;
	QSettings* m_appSettings;
	QQmlApplicationEngine* m_QMlEngine;
};

#endif // DBMANAGER_H
