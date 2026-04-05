#pragma once

#include "pageslistmodel.h"
#include "qml_singleton.h"
#include "return_codes.h"

#include <QObject>
#include <QVariantMap>
#include <QQuickWindow>

static inline int deferredActionId()
{
	static uint da_id{TP_RET_CODE_DEFERRED_ACTION};
	return da_id++;
}

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlExercisesDatabaseInterface)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlUserInterface)

QT_FORWARD_DECLARE_CLASS(QQmlApplicationEngine)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlItemManager : public QObject
{

Q_OBJECT

Q_PROPERTY(QQuickWindow* AppMainWindow READ AppMainWindow CONSTANT FINAL)
Q_PROPERTY(PagesListModel* AppPagesManager READ AppPagesManager CONSTANT FINAL)

public:
	explicit QmlItemManager(QQmlApplicationEngine *qml_engine);

	inline QQuickItem* AppHomePage() const { return m_homePage; }
	inline QQuickWindow *AppMainWindow() const { return _appMainWindow; }
	inline PagesListModel *AppPagesManager() const { return appPagesListModel(); }

	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void chooseFileToImport();
	Q_INVOKABLE void displayImportDialogMessageAfterMesoSelection(const int meso_idx);
	Q_INVOKABLE void showFirstTimeDialog();
	Q_INVOKABLE void getSettingsPage();
	Q_INVOKABLE void getUserPage();
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();
	Q_INVOKABLE void getExercisesPage(QmlWorkoutInterface *connectPage = nullptr);
	Q_INVOKABLE void showSimpleExercisesList(QQuickItem *parentPage, const QString &filter);
	Q_INVOKABLE void getWeatherPage();
	Q_INVOKABLE void getStatisticsPage();
	Q_INVOKABLE void displayMessageOnAppWindow(const int message_id, const QString &filename_or_message = QString{},
						QFlags<Qt::AlignmentFlag> postion = Qt::AlignTop|Qt::AlignHCenter, const QString &image_source = QString{},
							const uint msecs = 4000, const QString& button1text = QString{}, const QString &button2text = QString{}) const;
	Q_INVOKABLE inline void showTextCopiedMessage(const QString &message = QString{})
	{
		displayMessageOnAppWindow(TP_RET_CODE_CUSTOM_MESSAGE, message.isEmpty() ? tr("Text copied to the clipboard") : message);
	}

	void displayActivityResultMessage(const int requestCode, const int resultCode) const;

	Q_INVOKABLE DBExercisesModel *workoutModel() const { return m_workout_model; }
	DBExercisesModel *m_workout_model;

signals:
	void selectedExerciseFromSimpleExercisesList(QQuickItem *parentPage);
	void mesoForImportSelected();
	void qmlPasswordDialogClosed(int resultCode, QString password);
	void qmlImportDialogClose(bool result);
	/**
	 * @brief generalMessagesPopupClicked
	 * @param button_idx: 0 (dialog was closed via close button or back_key() or something else; 1: button1; 2: button2
	 */
	void generalMessagesPopupClicked(const uint8_t button_idx = 0);

#ifndef QT_NO_DEBUG
	void cppDataForQMLReady();
#endif

public slots:
	void mainWindowStarted() const;
	void openTPFile(uint32_t tp_filetype, const QString &filename, const bool formatted, const QVariant &extra_info);
	void homePageViewChanged(const bool own_mesos_view);
	inline void qmlPasswordDialogClosed_slot(int resultCode, const QString &password) { emit qmlPasswordDialogClosed(resultCode, password); }

private:
	QmlExercisesDatabaseInterface *m_exercisesListManager{nullptr};
	QQmlComponent *m_simpleExercisesListComponent{nullptr}, *m_weatherComponent{nullptr}, *m_statisticsComponent{nullptr},
																									*m_firstTimeDlgComponent{nullptr};
	QQuickItem *m_homePage{nullptr}, *m_weatherPage{nullptr}, *m_statisticsPage{nullptr};
	QObject *m_simpleExercisesList{nullptr}, *m_firstTimeDlg{nullptr};
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
	void createStatisticsPage_part2();
	QmlUserInterface *usersManager();
};
DECLARE_QML_NAMED_SINGLETON(QmlItemManager, ItemManager)

inline QmlItemManager *appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine *appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow *appMainWindow() { return QmlItemManager::_appMainWindow; }
