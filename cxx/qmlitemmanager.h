#pragma once

#include "pageslistmodel.h"
#include "qml_singleton.h"

#include <QObject>
#include <QVariantMap>
#include <QQuickItem>
#include <QQuickWindow>

QT_FORWARD_DECLARE_CLASS(DBCalendarModel)
QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlExercisesDatabaseInterface)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(QmlUserInterface)
QT_FORWARD_DECLARE_STRUCT(st_generalMessage)
QT_FORWARD_DECLARE_STRUCT(st_qmlPropertyChangesBuffer)
QT_FORWARD_DECLARE_CLASS(QQmlApplicationEngine)

class QmlItemManager : public QObject
{

Q_OBJECT

Q_PROPERTY(QQuickWindow* appMainWindow READ appMainWindow CONSTANT FINAL)
Q_PROPERTY(PagesListModel* appPagesManager READ appPagesManager CONSTANT FINAL)
Q_PROPERTY(QQuickItem* popupsVisualParent READ popupsVisualParent CONSTANT FINAL)

public:
	enum PASSWORD_DIALOG_MODE {
		DM_GET_PASSWORD,
		DM_NEW_PASSWORD,
		DM_CHANGE_PASSWORD,
	};
	Q_ENUM(PASSWORD_DIALOG_MODE)

	Q_DISABLE_COPY_MOVE(QmlItemManager)
	explicit QmlItemManager();
	void startQmlEngine(QQmlApplicationEngine *qml_engine);

	Q_INVOKABLE inline QQuickItem *appHomePage() const { return m_homePage; }
	inline QQuickItem *appPagesVisualParent() const { return m_appPagesVisualParent; }
	inline QQuickWindow *appMainWindow() const { return _appMainWindow; }
	inline PagesListModel *appPagesManager() const { return appPagesListModel(); }
	inline QQuickItem *popupsVisualParent() const { return m_popupsVisualParent; }

	Q_INVOKABLE void exitApp();
	Q_INVOKABLE void showFirstTimeDialog();
	Q_INVOKABLE void getSettingsPage();
	Q_INVOKABLE void getUserPage();
	Q_INVOKABLE void getCoachesPage();
	Q_INVOKABLE void getClientsPage();
	Q_INVOKABLE void getExercisesPage(QmlWorkoutInterface *connectPage = nullptr);
	Q_INVOKABLE void showSimpleExercisesList(QQuickItem *parentPage, const QString &filter);
	Q_INVOKABLE void getWeatherPage();
	Q_INVOKABLE void getStatisticsPage();

	Q_INVOKABLE void displayWindowMessage(const int message_id, const int msecs,
										QFlags<Qt::AlignmentFlag> position = Qt::AlignTop|Qt::AlignHCenter,
										const QString &title = QString{}, const QString &message = QString{});

	void displayMessageOnAppWindow(const int message_id, QString &&message = QString{},
										QFlags<Qt::AlignmentFlag> position = Qt::AlignTop|Qt::AlignHCenter,
										QString &&image_source = QString{}, const int msecs = 4000,
										QString &&button1text = QString{}, QString &&button2text = QString{}) const;

	void showPasswordDialog(const int request_id, QQuickItem *parent_page, const QString &title, const QString &message,
				const PASSWORD_DIALOG_MODE mode = DM_GET_PASSWORD, const std::optional<bool> store_passwd = std::nullopt);
	void showImportWorkoutDialog(DBExercisesModel *new_workout, QQuickItem *parent_page, DBCalendarModel *cal_model,
																							const QChar &split_letter);

signals:
	void selectedExerciseFromSimpleExercisesList(QQuickItem *parentPage);
	void mesoForImportSelected();
	void qmlPasswordDialogClosed(int resultCode, QString password);
	void passwordAcquired(const bool proceed, const int request_id, const QString &passwd, const bool store);
	void passwordCreated(const bool proceed, const int request_id, const QString &passwd, const bool store);
	void passwordChanged(const bool proceed, const int request_id, const QString &old_passwd, const QString &new_passwd,
																										const bool store);
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
	void generalMessagesPopupModallyClosed();

private:
	QmlExercisesDatabaseInterface *m_exercisesListManager{nullptr};
	QQmlComponent *m_simpleExercisesListComponent{nullptr}, *m_weatherComponent{nullptr},
		*m_statisticsComponent{nullptr}, *m_firstTimeDlgComponent{nullptr}, *m_generalMessagesPopupComponent{nullptr},
												*m_passwordDialogComponent{nullptr}, *m_importWorkoutComponent{nullptr};
	QQuickItem *m_homePage{nullptr}, *m_appPagesVisualParent{nullptr}, *m_popupsVisualParent{nullptr},
															*m_weatherPage{nullptr}, *m_statisticsPage{nullptr};
	QObject *m_simpleExercisesList{nullptr}, *m_firstTimeDlg{nullptr}, *m_generalMessagesPopup{nullptr},
													*m_passwordDialog{nullptr}, *m_importWorkoutDialog{nullptr};
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
	void createStatisticsPage_part2();
	QmlUserInterface *usersManager();
};
DECLARE_QML_NAMED_SINGLETON(QmlItemManager, ItemManager)

inline QmlItemManager *appItemManager() { return QmlItemManager::_appItemManager; }
inline QQmlApplicationEngine *appQmlEngine() { return QmlItemManager::_appQmlEngine; }
inline QQuickWindow *appMainWindow() { return QmlItemManager::_appMainWindow; }
