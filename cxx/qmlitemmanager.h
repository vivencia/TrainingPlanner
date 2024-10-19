#ifndef QMLITEMMANAGER_H
#define QMLITEMMANAGER_H

#include "tpglobals.h"
#include "tptimer.h"

#include <QObject>
#include <QMap>
#include <QDate>

class DBInterface;
class DBUserModel;
class DBMesocyclesModel;
class DBExercisesModel;
class DBTrainingDayModel;
class DBMesoSplitModel;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlUserInterface;
class QmlExercisesDatabaseInterface;
class QMLMesoInterface;
class QmlMesoCalendarInterface;
class QmlMesoSplitInterface;
class QmlTDayInterface;

class QmlItemManager : public QObject
{

Q_OBJECT

public:
	inline explicit QmlItemManager(QObject* parent = nullptr)
		: QObject{parent} {}
	~QmlItemManager();
	void configureQmlEngine(QQmlApplicationEngine *qml_engine);

	QmlUserInterface* usersManager();
	QmlExercisesDatabaseInterface* exercisesListManager();
	QMLMesoInterface* mesocyclesManager(const uint meso_idx);
	QmlMesoCalendarInterface* calendarManager(const uint meso_idx);
	QmlMesoSplitInterface* splitManager(const uint meso_idx);
	QmlTDayInterface* tDayManager(const uint meso_idx, const QDate& date);

	Q_INVOKABLE void openMainMenuShortCut(const int button_id);
	Q_INVOKABLE inline void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	Q_INVOKABLE void tryToImport(const QList<bool>& selectedFields);
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void displayImportDialogMessage(const uint fileContents, const QString& filename);

public slots:
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString& fileName = QString()) const;
	void requestTimerDialog(QQuickItem* requester, const QVariant& args);
	void requestExercisesList(QQuickItem* requester, QVariant visible, QVariant multipleSelection, int id);
	void requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset);
	void showRemoveExerciseMessage(int exercise_idx);
	void showRemoveSetMessage(int set_number, int exercise_idx);
	void exerciseCompleted(int exercise_idx);
	void exportSlot(const QString& filePath = QString());
	void importSlot_FileChosen(const QString& filePath = QString());

signals:
	void setObjectReady();

private:
	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	uint m_fileContents;
	QString m_exportFilename, m_importFilename;

	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);

	QQmlApplicationEngine* m_appQmlEngine;
	QQuickWindow* m_appMainWindow;
	QmlUserInterface* m_appUsersManager;
	QmlExercisesDatabaseInterface* m_appExercisesListManager;
	QList<QMLMesoInterface*> m_appMesosManager;
	QList<QmlMesoCalendarInterface*> m_appCalendarManager;
	QList<QmlMesoSplitInterface*> m_appSplitManager;
	QList<QmlTDayInterface*> m_appTDayManager;
};

#endif // QMLITEMMANAGER_H
