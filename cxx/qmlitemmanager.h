#pragma once
#include <QObject>

#define APPWINDOW_MSG_IMPORT_OK 0
#define APPWINDOW_MSG_EXPORT_OK 1
#define APPWINDOW_MSG_SHARE_OK 2
#define APPWINDOW_MSG_OPEN_FAILED -1
#define APPWINDOW_MSG_UNKNOWN_FILE_FORMAT -2
#define APPWINDOW_MSG_CORRUPT_FILE -3
#define APPWINDOW_MSG_NOTHING_TODO -4
#define APPWINDOW_MSG_NO_MESO -5
#define APPWINDOW_MSG_NOTHING_TO_EXPORT -6
#define APPWINDOW_MSG_SHARE_FAILED -7
#define APPWINDOW_MSG_EXPORT_FAILED -8
#define APPWINDOW_MSG_IMPORT_FAILED_SAME_DATA -9
#define APPWINDOW_MSG_IMPORT_CANCELED -10
#define APPWINDOW_MSG_IMPORT_FAILED -11
#define APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED -12
#define APPWINDOW_MSG_WRONG_IMPORT_FILE_TYPE -13
#define APPWINDOW_MSG_UNKNOWN_ERROR -100
#define APPWINDOW_MSG_CUSTOM_MESSAGE 1000
#define APPWINDOW_MSG_CUSTOM_WARNING 1001
#define APPWINDOW_MSG_CUSTOM_ERROR 1002
#define APPWINDOW_MSG_DEFERRED_ACTION 2000

enum {
	IFC_USER = 0,
	IFC_MESO = 1,
	IFC_MESOSPLIT = 2,
		IFC_MESOSPLIT_A = 3,
		IFC_MESOSPLIT_B = 4,
		IFC_MESOSPLIT_C = 5,
		IFC_MESOSPLIT_D = 6,
		IFC_MESOSPLIT_E = 7,
		IFC_MESOSPLIT_F = 8,
	IFC_EXERCISES = 9,
	IFC_WORKOUT = 10
} typedef importFileContents;
constexpr short ifc_count{11};

static inline int deferredActionId()
{
	static uint da_id{APPWINDOW_MSG_DEFERRED_ACTION};
	return da_id++;
}

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

	void showSimpleExercisesList(QQuickItem *parentPage, const QString &filter) const;
	void hideSimpleExercisesList(QQuickItem *parentPage) const;
	void openPage(const QString &label, QQuickItem *page, const std::function<void ()> &clean_up_func = nullptr);
	void closePage(QQuickItem *page);
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
	void displayMessageOnAppWindow(const int message_id, const QString &fileName = QString{},
						const QString &image_source = QString{}, const uint msecs = 5000) const;
	void exportSlot(const QString &filePath = QString{});
	void importSlot_FileChosen(const QString &filePath = QString{}, const int content_type = -1);
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
