#pragma once

#include "dbmesocyclesmodel.h"
#include "pageslistmodel.h"
#include "return_codes.h"

#include <QObject>

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
	static uint da_id{TP_RET_CODE_DEFERRED_ACTION};
	return da_id++;
}

QT_FORWARD_DECLARE_CLASS(QmlExercisesDatabaseInterface)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlUserInterface)
QT_FORWARD_DECLARE_CLASS(TPChat)
QT_FORWARD_DECLARE_CLASS(TPListModel)
QT_FORWARD_DECLARE_CLASS(TPWorkoutsCalendar)

QT_FORWARD_DECLARE_CLASS(QQmlApplicationEngine)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

class QmlItemManager : public QObject
{

Q_OBJECT

Q_PROPERTY(DBMesocyclesModel* appMesocyclesModel READ appMesocyclesModel NOTIFY userChangedSignal FINAL)
Q_PROPERTY(PagesListModel* appPagesModel READ appPagesModel NOTIFY userChangedSignal FINAL)

public:
	explicit inline QmlItemManager(QQmlApplicationEngine *qml_engine)
		: QObject{nullptr}, m_usersManager{nullptr}, m_exercisesListManager{nullptr}, m_weatherPage{nullptr},
				m_statisticsPage{nullptr}, m_allWorkoutsPage{nullptr}
	{
		_appItemManager = this;
		_appQmlEngine = qml_engine;
		configureQmlEngine();
	}
	~QmlItemManager();
	void configureQmlEngine();

	inline DBMesocyclesModel *appMesocyclesModel() const { return appMesoModel(); }
	inline PagesListModel *appPagesModel() const { return appPagesListModel(); }
	inline QQuickItem* appHomePage() const { return m_homePage; }

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
	const QString &setExportFileName(const QString &filename);
	void continueExport(int exportMessageId, const bool bShare);
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void getPasswordDialog(const QString &title, const QString &message) const;
	void openRequestedFile(const QString &filename, const int wanted_content = 0x3FF);

signals:
	void userChangedSignal();
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
	QQuickItem *m_homePage, *m_weatherPage, *m_statisticsPage, *m_allWorkoutsPage;
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
