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
	void configureQmlEngine();

	Q_INVOKABLE void openMainMenuShortCut(const int button_id);
	Q_INVOKABLE void tryToImport(const QList<bool>& selectedFields);
	Q_INVOKABLE inline void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getExercisesPage(QQuickItem* connectPage = nullptr);

	const QString& setExportFileName(const QString& filename);
	void continueExport(int exportMessageId, const bool bShare);
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void displayImportDialogMessage(const uint fileContents, const QString& filename);
	void openRequestedFile(const QString& filename, const int wanted_content = 0xFF);
	void importFromFile(const QString& filename, const int wanted_content = 0xFF);
	void incorporateImportedData(TPListModel* model);

public slots:
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString& fileName = QString()) const;
	void exportSlot(const QString& filePath = QString());
	void importSlot_FileChosen(const QString& filePath = QString());
	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);

private:
	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	uint m_fileContents;
	QString m_exportFilename, m_importFilename;

	static QmlItemManager* _appItemManager;
	friend QmlItemManager* appItemManager();

	static QQmlApplicationEngine* _appQmlEngine;
	friend QQmlApplicationEngine* appQmlEngine();

	static QQuickWindow* _appMainWindow;
	friend QQuickWindow* appMainWindow();

	QmlUserInterface* m_usersManager;
	QmlExercisesDatabaseInterface* m_exercisesListManager;
};

inline QmlItemManager* appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine* appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow* appMainWindow() { return QmlItemManager::_appMainWindow; }

#endif // QMLITEMMANAGER_H
