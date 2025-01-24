#ifndef QMLITEMMANAGER_H
#define QMLITEMMANAGER_H

#include <QObject>

class PagesListModel;
class QmlExercisesDatabaseInterface;
class QmlTDayInterface;
class QmlUserInterface;
class TPListModel;
class TPWorkoutsCalendar;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

//Avoid including tpglobals.h only for this value
#define IFC_ANY_ 12

class QmlItemManager : public QObject
{

Q_OBJECT

public:
	explicit QmlItemManager(QQmlApplicationEngine* qml_engine);
	~QmlItemManager();
	void configureQmlEngine();

	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void chooseFileToImport();
	Q_INVOKABLE void tryToImport(const QList<bool>& selectedFields);
	Q_INVOKABLE void displayImportDialogMessageAfterMesoSelection(const int meso_idx);
	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);
	Q_INVOKABLE void getExercisesPage(QmlTDayInterface* connectPage = nullptr);
	Q_INVOKABLE void getWeatherPage();
	Q_INVOKABLE void getStatisticsPage();
	Q_INVOKABLE void getAllWorkoutsPage();

	const QString& setExportFileName(const QString& filename);
	void continueExport(int exportMessageId, const bool bShare);
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void getPasswordDialog(const QString& title, const QString& message) const;
	void selectWhichMesoToImportInto();
	void displayImportDialogMessage(const uint fileContents, const QString& filename);
	void openRequestedFile(const QString& filename, const int wanted_content = IFC_ANY_);
	void importFromFile(const QString& filename, const int wanted_content = IFC_ANY_);
	int incorporateImportedData(TPListModel* model, const int wanted_content = 0);

signals:
	void mesoForImportSelected();
	void qmlPasswordDialogClosed(int resultCode, QString password);

public slots:
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString& fileName = QString()) const;
	void exportSlot(const QString& filePath = QString());
	void importSlot_FileChosen(const QString& filePath = QString(), const int fileType = IFC_ANY_);
	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);
	inline void qmlPasswordDialogClosed_slot(int resultCode, QString password) { emit qmlPasswordDialogClosed(resultCode, password); }

private:
	uint m_fileContents;
	QString m_exportFilename, m_importFilename;
	QmlUserInterface* m_usersManager;
	QmlExercisesDatabaseInterface* m_exercisesListManager;
	QQmlComponent* m_weatherComponent, *m_statisticsComponent, *m_allWorkoutsComponent;
	QQuickItem* m_weatherPage, *m_statisticsPage, *m_allWorkoutsPage;
	PagesListModel* m_pagesManager;
	TPWorkoutsCalendar* m_wokoutsCalendar;

	static QmlItemManager* _appItemManager;
	friend QmlItemManager* appItemManager();

	static QQmlApplicationEngine* _appQmlEngine;
	friend QQmlApplicationEngine* appQmlEngine();

	static QQuickWindow* _appMainWindow;
	friend QQuickWindow* appMainWindow();

	void createWeatherPage_part2();
	void createStatisticsPage_part2();
	void createAllWorkoutsPage_part2();
};

inline QmlItemManager* appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine* appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow* appMainWindow() { return QmlItemManager::_appMainWindow; }

#endif // QMLITEMMANAGER_H
