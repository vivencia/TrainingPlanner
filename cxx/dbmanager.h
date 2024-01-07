#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QSqlDatabase>

class DbManager : public QObject
{

Q_OBJECT
QML_ELEMENT

public:
	explicit DbManager(const QString& dbPath);
	bool updateExercisesList(const QStringList& exercisesList);

private:
	QSqlDatabase mSqlLiteDB;
};

#endif // DBMANAGER_H
