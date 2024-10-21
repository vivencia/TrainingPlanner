#ifndef QMLITEMMANAGER_H
#define QMLITEMMANAGER_H

#include "tpglobals.h"

#include <QObject>

class TPListModel;
class QmlUserInterface;
class QmlExercisesDatabaseInterface;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlItemManager : public QObject
{

Q_OBJECT

public:
	explicit QmlItemManager(QQmlApplicationEngine* qml_engine);
	~QmlItemManager();
	void configureQmlEngine();

	QmlUserInterface* usersManager();
	QmlExercisesDatabaseInterface* exercisesListManager();

	Q_INVOKABLE void openMainMenuShortCut(const int button_id);
	Q_INVOKABLE void tryToImport(const QList<bool>& selectedFields);
	Q_INVOKABLE inline void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void displayImportDialogMessage(const uint fileContents, const QString& filename);
	void openRequestedFile(const QString& filename, const int wanted_content = 0xFF);
	void importFromFile(const QString& filename, const int wanted_content = 0xFF);
	void incorporateImportedData(const TPListModel* const model);

public slots:
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString& fileName = QString()) const;
	void exportSlot(const QString& filePath = QString());
	void importSlot_FileChosen(const QString& filePath = QString());

private:
	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	uint m_fileContents;
	QString m_exportFilename, m_importFilename;

	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);

	static QQmlApplicationEngine* _appQmlEngine;
	friend QQmlApplicationEngine* appQmlEngine();

	static QQuickWindow* _appMainWindow;
	friend QQuickWindow* appMainWindow();

	static QmlUserInterface* _appUsersManager;
	friend QmlUserInterface* appUsersManager();

	static QmlExercisesDatabaseInterface* _appExercisesListManager;
	friend QmlExercisesDatabaseInterface* appExercisesListManager();
};

inline QQmlApplicationEngine* appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow* appMainWindow() { return QmlItemManager::_appMainWindow; }
inline QmlUserInterface* appUsersManager() { return QmlItemManager::_appUsersManager; }
inline QmlExercisesDatabaseInterface* appExercisesListManager() { return QmlItemManager::_appExercisesListManager; }

#endif // QMLITEMMANAGER_H
