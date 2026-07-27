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
QT_FORWARD_DECLARE_STRUCT(st_generalMessage)
QT_FORWARD_DECLARE_STRUCT(st_qmlPropertyChangesBuffer)
QT_FORWARD_DECLARE_CLASS(QQmlApplicationEngine)
QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlItemManager : public QObject
{

Q_OBJECT

Q_PROPERTY(QQuickWindow* appMainWindow READ appMainWindow CONSTANT FINAL)
Q_PROPERTY(PagesListModel* appPagesManager READ appPagesManager CONSTANT FINAL)

public:
	explicit QmlItemManager();
	void startQmlEngine(QQmlApplicationEngine *qml_engine);

	Q_INVOKABLE inline QQuickItem *appHomePage() const { return m_homePage; }
	inline QQuickItem *appPagesVisualParent() const { return m_appPagesVisualParent; }
	inline QQuickWindow *appMainWindow() const { return _appMainWindow; }
	inline PagesListModel *appPagesManager() const { return appPagesListModel(); }

	Q_INVOKABLE void exitApp();
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
	Q_INVOKABLE void showOnlineMessagesManagerDialog(const bool show);

	Q_INVOKABLE void displayWindowMessage(const int message_id, const int msecs,
										QFlags<Qt::AlignmentFlag> position = Qt::AlignTop|Qt::AlignHCenter,
										const QString &title = QString{}, const QString &message = QString{});

	void displayMessageOnAppWindow(const int message_id, QString &&message = QString{},
										QFlags<Qt::AlignmentFlag> position = Qt::AlignTop|Qt::AlignHCenter,
										QString &&image_source = QString{}, const int msecs = 4000,
										QString &&button1text = QString{}, QString &&button2text = QString{}) const;

	void showPasswordDialog(const int request_id, QQuickItem *parent_page, const QString &title,
							const QString &message, const std::optional<bool> store_passwd = std::nullopt);
	void startMessagesManager();

	Q_INVOKABLE DBExercisesModel *workoutModel() const { return m_workout_model; }
	DBExercisesModel *m_workout_model;

signals:
	void selectedExerciseFromSimpleExercisesList(QQuickItem *parentPage);
	void mesoForImportSelected();
	void qmlPasswordDialogClosed(int resultCode, QString password);
	void passwordAcquired(const bool proceed, const int request_id, const QString &passwd, const bool store);
	/**
	 * @brief generalMessagesPopupClicked
	 * @param button: 0 (dialog was closed via close button or back_key() or something else; 1: button1; 2: button2
	 */
	void generalMessagesPopupClicked(const uint8_t button);

#ifndef QT_NO_DEBUG
	void cppDataForQMLReady();
#endif

public slots:
	void homePageViewChanged(const bool own_mesos_view);
	inline void qmlPasswordDialogClosed_slot(int resultCode, const QString &password) { emit qmlPasswordDialogClosed(resultCode, password); }
	void generalMessagesPopupClosed(const int btn_id);

private:
	QmlExercisesDatabaseInterface *m_exercisesListManager{nullptr};
	QQmlComponent *m_simpleExercisesListComponent{nullptr}, *m_weatherComponent{nullptr},
		*m_statisticsComponent{nullptr}, *m_firstTimeDlgComponent{nullptr}, *m_generalMessagesPopupComponent{nullptr},
								*m_messagesManagerComponent{nullptr}, *m_passwordDialogComponent{nullptr};
	QQuickItem *m_homePage{nullptr}, *m_appPagesVisualParent{nullptr}, *m_weatherPage{nullptr},
																				*m_statisticsPage{nullptr};
	QObject *m_simpleExercisesList{nullptr}, *m_firstTimeDlg{nullptr}, *m_generalMessagesPopup{nullptr},
												*m_messagesManagerPopup{nullptr}, *m_passwordDialog{nullptr};
	QVariantMap m_simpleExercisesListProperties, m_generalMessagesPopupProperties;
	QList<st_generalMessage*> m_messagesQueue;
	QList<uint16_t> m_bufferProperties;
	bool m_canDisplayMessage{false};

#ifndef Q_OS_ANDROID
	#ifndef QT_NO_DEBUG
	enum testType {
		TT_NO_TEST = 0,
		TT_CORE = 1,
		TT_QML = 2,
	};

	uint m_testType{TT_NO_TEST};
	bool runTests();
	#endif
#endif

	static QmlItemManager *_appItemManager;
	friend QmlItemManager *appItemManager();

	static QQmlApplicationEngine *_appQmlEngine;
	friend QQmlApplicationEngine *appQmlEngine();

	static QQuickWindow *_appMainWindow;
	friend QQuickWindow *appMainWindow();

	void createGeneralMessagesPopup();
	void createSimpleExercisesList(QQuickItem *parentPage);
	void createStatisticsPage_part2();
	QmlUserInterface *usersManager();
};
DECLARE_QML_NAMED_SINGLETON(QmlItemManager, ItemManager)

inline QmlItemManager *appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine *appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow *appMainWindow() { return QmlItemManager::_appMainWindow; }
