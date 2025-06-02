#pragma once
#include <QObject>

class PagesListModel;
class QmlExercisesDatabaseInterface;
class QmlWorkoutInterface;
class QmlUserInterface;
class TPListModel;
class TPWorkoutsCalendar;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlItemManager : public QObject
{

Q_OBJECT

public:
	explicit QmlItemManager(QQmlApplicationEngine *qml_engine);
	~QmlItemManager();
	void configureQmlEngine();

	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void chooseFileToImport();
	Q_INVOKABLE void importFromSelectedFile(const QList<bool> &selectedFields);
	Q_INVOKABLE void displayImportDialogMessageAfterMesoSelection(const int meso_idx);
	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();
	Q_INVOKABLE void getExercisesPage(QmlWorkoutInterface *connectPage = nullptr);
	Q_INVOKABLE void getWeatherPage();
	Q_INVOKABLE void getStatisticsPage();
	Q_INVOKABLE void getAllWorkoutsPage();

	const QString &setExportFileName(const QString &filename);
	void continueExport(int exportMessageId, const bool bShare);
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void getPasswordDialog(const QString &title, const QString &message) const;
	void openRequestedFile(const QString &filename, const int wanted_content = 0x3FF);

signals:
	void mesoForImportSelected();
	void qmlPasswordDialogClosed(int resultCode, QString password);

public slots:
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString &fileName = QString()) const;
	void exportSlot(const QString &filePath = QString());
	void importSlot_FileChosen(const QString &filePath = QString(), const int fileType = -1);
	void addMainMenuShortCut(const QString &label, QQuickItem *page);
	void removeMainMenuShortCut(QQuickItem *page);
	inline void qmlPasswordDialogClosed_slot(int resultCode, const QString &password) { emit qmlPasswordDialogClosed(resultCode, password); }

private:
	QString m_exportFilename, m_importFilename;
	QmlUserInterface *m_usersManager;
	QmlExercisesDatabaseInterface *m_exercisesListManager;
	QQmlComponent *m_weatherComponent, *m_statisticsComponent, *m_allWorkoutsComponent;
	QQuickItem *m_weatherPage, *m_statisticsPage, *m_allWorkoutsPage;
	PagesListModel *m_pagesManager;
	TPWorkoutsCalendar *m_wokoutsCalendar;

	static QmlItemManager *_appItemManager;
	friend QmlItemManager *appItemManager();

	static QQmlApplicationEngine *_appQmlEngine;
	friend QQmlApplicationEngine *appQmlEngine();

	static QQuickWindow *_appMainWindow;
	friend QQuickWindow *appMainWindow();

	void createWeatherPage_part2();
	void createStatisticsPage_part2();
	void createAllWorkoutsPage_part2();
};

inline QmlItemManager *appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine *appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow *appMainWindow() { return QmlItemManager::_appMainWindow; }
