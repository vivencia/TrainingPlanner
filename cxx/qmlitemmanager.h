#pragma once

#include "return_codes.h"

#include <QObject>
#include <QVariantMap>

static inline int deferredActionId()
{
	static uint da_id{TP_RET_CODE_DEFERRED_ACTION};
	return da_id++;
}

QT_FORWARD_DECLARE_CLASS(QmlExercisesDatabaseInterface)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlUserInterface)
QT_FORWARD_DECLARE_CLASS(PagesListModel)
QT_FORWARD_DECLARE_CLASS(TPChat)
QT_FORWARD_DECLARE_CLASS(TPListModel)

QT_FORWARD_DECLARE_CLASS(QQmlApplicationEngine)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)

class QmlItemManager : public QObject
{

Q_OBJECT

public:
	explicit inline QmlItemManager(QQmlApplicationEngine *qml_engine)
		: QObject{nullptr}, m_usersManager{nullptr}, m_exercisesListManager{nullptr}, m_simpleExercisesList{nullptr},
																		m_weatherPage{nullptr}, m_statisticsPage{nullptr}
	{
		_appItemManager = this;
		_appQmlEngine = qml_engine;
		configureQmlEngine();
	}
	~QmlItemManager();
	void configureQmlEngine();

	inline QQuickItem* appHomePage() const { return m_homePage; }

	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void chooseFileToImport();
	Q_INVOKABLE void displayImportDialogMessageAfterMesoSelection(const int meso_idx);
	Q_INVOKABLE void exportMeso(const uint meso_idx, const bool share);
	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();
	Q_INVOKABLE void getExercisesPage(QmlWorkoutInterface *connectPage = nullptr);
	Q_INVOKABLE void showSimpleExercisesList(QQuickItem *parentPage, const QString &filter);
	Q_INVOKABLE void getWeatherPage();
	Q_INVOKABLE void getStatisticsPage();

	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void getPasswordDialog(const QString &title, const QString &message) const;

	Q_INVOKABLE DBExercisesModel *workoutModel() const { return m_workout_model; }
	DBExercisesModel *m_workout_model;

signals:
	void selectedExerciseFromSimpleExercisesList(QQuickItem *parentPage);
	void mesoForImportSelected();
	void qmlPasswordDialogClosed(int resultCode, QString password);
	void qmlImportDialogClose(bool result);
#ifndef QT_NO_DEBUG
	void cppDataForQMLReady();
#endif

public slots:
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString &fileName = QString{},
											const QString &image_source = QString{}, const uint msecs = 5000) const;
	void openTPFile(uint32_t tp_filetype, const QString &filename, const bool formatted, const QVariant &extra_info);
	void exportSlot(const QString &filePath = QString{});
	void homePageViewChanged(const bool own_mesos_view);
	inline void qmlPasswordDialogClosed_slot(int resultCode, const QString &password) { emit qmlPasswordDialogClosed(resultCode, password); }

private:
	QmlUserInterface *m_usersManager;
	QmlExercisesDatabaseInterface *m_exercisesListManager;
	QQmlComponent *m_simpleExercisesListComponent, *m_weatherComponent, *m_statisticsComponent;
	QQuickItem *m_homePage, *m_weatherPage, *m_statisticsPage;
	QObject *m_simpleExercisesList;
	QVariantMap m_simpleExercisesListProperties;

#ifndef QT_NO_DEBUG
	bool m_qml_testing;
#endif

	static QmlItemManager *_appItemManager;
	friend QmlItemManager *appItemManager();

	static QQmlApplicationEngine *_appQmlEngine;
	friend QQmlApplicationEngine *appQmlEngine();

	static QQuickWindow *_appMainWindow;
	friend QQuickWindow *appMainWindow();

	void showSimpleExercisesList();
	void createSimpleExercisesList(QQuickItem *parentPage);
	void createWeatherPage_part2();
	void createStatisticsPage_part2();
};

inline QmlItemManager *appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine *appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow *appMainWindow() { return QmlItemManager::_appMainWindow; }
