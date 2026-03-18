#pragma once

#include "pageslistmodel.h"
#include "qml_singleton.h"
#include "return_codes.h"

#include <QObject>
#include <QVariantMap>

static inline int deferredActionId()
{
	static uint da_id{TP_RET_CODE_DEFERRED_ACTION};
	return da_id++;
}

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlExercisesDatabaseInterface)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlUserInterface)
QT_FORWARD_DECLARE_CLASS(TPChat)
QT_FORWARD_DECLARE_CLASS(TPListModel)

QT_FORWARD_DECLARE_CLASS(QQmlApplicationEngine)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

class QmlItemManager : public QObject
{

Q_OBJECT

Q_PROPERTY(QQuickWindow* appMainWindow READ appMainWindow CONSTANT FINAL)
Q_PROPERTY(PagesListModel* appPagesManager READ appPagesManager CONSTANT FINAL)

public:
	explicit QmlItemManager(QQmlApplicationEngine *qml_engine);

	inline QQuickItem* appHomePage() const { return m_homePage; }
	inline QQuickWindow *appMainWindow() const { return _appMainWindow; }
	inline PagesListModel *appPagesManager() const { return appPagesListModel(); }

	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void chooseFileToImport();
	Q_INVOKABLE void displayImportDialogMessageAfterMesoSelection(const int meso_idx);
	Q_INVOKABLE void exportMeso(const uint meso_idx, const bool share);
	Q_INVOKABLE void getSettingsPage();
	Q_INVOKABLE void getUserPage();
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
	void displayMessageOnAppWindow(const int message_id, const QString &filename_or_message = QString{},
																const QString &image_source = QString{}, const uint msecs = 5000) const;
	void openTPFile(uint32_t tp_filetype, const QString &filename, const bool formatted, const QVariant &extra_info);
	void homePageViewChanged(const bool own_mesos_view);
	inline void qmlPasswordDialogClosed_slot(int resultCode, const QString &password) { emit qmlPasswordDialogClosed(resultCode, password); }

private:
	QmlExercisesDatabaseInterface *m_exercisesListManager{nullptr};
	QQmlComponent *m_simpleExercisesListComponent{nullptr}, *m_weatherComponent{nullptr}, *m_statisticsComponent{nullptr};
	QQuickItem *m_homePage{nullptr}, *m_weatherPage{nullptr}, *m_statisticsPage{nullptr};
	QObject *m_simpleExercisesList{nullptr};
	QVariantMap m_simpleExercisesListProperties;

#ifndef QT_NO_DEBUG
	bool m_qml_testing{false};
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
	QmlUserInterface *usersManager();
};

DECLARE_QML_NAMED_SINGLETON(QmlItemManager, ItemManager)

inline QmlItemManager *appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine *appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow *appMainWindow() { return QmlItemManager::_appMainWindow; }
