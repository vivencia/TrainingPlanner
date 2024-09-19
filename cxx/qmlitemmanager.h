#ifndef QMLITEMMANAGER_H
#define QMLITEMMANAGER_H

#include "dbmesosplitmodel.h"
#include "dbtrainingdaymodel.h"
#include "tptimer.h"

#include <QObject>
#include <QMap>
#include <QDate>

class DBInterface;
class DBUserModel;
class DBMesocyclesModel;
class DBExercisesModel;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

static const uint mesoPageCreateId(175);
static const uint calPageCreateId(35);
static const uint tDayPageCreateId(70);
static const uint tDayExerciseCreateId(105);
static const uint tDaySetCreateId(140);
static const uint menuShortCutCreatedId(200);
static const uint exercisesPlannerCreateId(235);

enum
{
	APPWINDOW_MSG_EXPORT_OK = 2,
	APPWINDOW_MSG_SHARE_OK = 1,
	APPWINDOW_MSG_IMPORT_OK = 0,
	APPWINDOW_MSG_OPEN_FAILED = -1,
	APPWINDOW_MSG_UNKNOWN_FILE_FORMAT = -2,
	APPWINDOW_MSG_CORRUPT_FILE = -3,
	APPWINDOW_MSG_NOTHING_TODO = -4,
	APPWINDOW_MSG_NO_MESO = -5,
	APPWINDOW_MSG_NOTHING_TO_EXPORT = -6,
	APPWINDOW_MSG_SHARE_FAILED = -7,
	APPWINDOW_MSG_EXPORT_FAILED = -8,
	APPWINDOW_MSG_OPEN_CREATE_FILE_FAILED = -9,
	APPWINDOW_MSG_UNKNOWN_ERROR = -100
} typedef appWindowMessageID;

class QmlItemManager : public QObject
{

Q_OBJECT
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged)

public:
	inline QmlItemManager(const uint meso_idx, QObject* parent = nullptr)
		: QObject{parent}, m_mesoIdx(meso_idx),
			m_mesoComponent(nullptr), m_plannerComponent(nullptr),
			m_splitComponent(nullptr), m_calComponent(nullptr), m_tDayComponent(nullptr), m_tDayExercisesComponent(nullptr),
			m_setComponents{nullptr}, m_importDlgComponent(nullptr) {}
	~QmlItemManager();
	static void configureQmlEngine();
	void initQML();

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; emit mesoIdxChanged(); }

	void displayMessageOnAppWindow(const appWindowMessageID message_id) const;
	void displayMessageOnAppWindow(const QString& title, const QString& message) const;
	void displayImportDialogMessage(const uint fileContents, const QString& filename);
	void createImportDialog();

	//-----------------------------------------------------------USER-----------------------------------------------------------
	void getSettingsPage(const uint startPageIndex);
	void createSettingsPage();
	void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);
	void createClientsOrCoachesPage();
	void setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches);
	void removeUser(const uint user_row, const bool bCoach);
	//-----------------------------------------------------------USER-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	void createExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	void createExercisesPage_part2(QQuickItem* connectPage);
	void getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	Q_INVOKABLE const uint removeExercise(const uint row);
	Q_INVOKABLE void exportExercises(const bool bShare);
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();
	void getMesocyclePage();
	Q_INVOKABLE void scheduleMesocycleRemoval();
	void exportMeso(const bool bShare, const bool bCoachInfo);
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	Q_INVOKABLE void createPlannerPage();
	void createPlannerPage_part2();
	void getExercisesPlannerPage();

	Q_INVOKABLE void getMesoSplitPage(const uint page_index);
	void createMesoSplitPage(const uint page_index);
	void setSplitPageProperties(QQuickItem* splitPage, const DBMesoSplitModel* const splitModel);

	DBMesoSplitModel* getSplitModel(const QChar& splitLetter);
	void initializeSplitModels();
	inline QMap<QChar,DBMesoSplitModel*>& allSplitModels() { initializeSplitModels(); return m_splitModels; }
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	Q_INVOKABLE void swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2);
	void updateMuscularGroup(DBMesoSplitModel* splitModel);
	void changeMuscularGroup(const QString& new_musculargroup, DBMesoSplitModel* splitModel);
	Q_INVOKABLE void exportMesoSplit(const bool bShare, const QString& splitLetter, const QString& filePath = QString(), const bool bJustExport = false);
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	uint createMesoCalendarPage();
	void createMesoCalendarPage_part2();
	void getMesoCalendarPage();
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	uint createTrainingDayPage(const QDate& date);
	void createTrainingDayPage_part2();
	void getTrainingDayPage(const QDate& date);
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan();
	Q_INVOKABLE void convertTDayToPlan();

	DBTrainingDayModel* gettDayModel(const QDate& date);
	inline DBTrainingDayModel* currenttDayModel() { return m_CurrenttDayModel; }
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void setCurrenttDay(const QDate& date);
	inline bool setsLoaded(const uint exercise_idx) const { return m_currentExercises->setCount(exercise_idx) > 0; }
	void updateOpenTDayPagesWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void setTrainingDayPageEmptyDayOptions(const DBTrainingDayModel* const model);
	Q_INVOKABLE void exportTrainingDay(const bool bShare, const DBTrainingDayModel *const tDayModel);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	Q_INVOKABLE uint createExerciseObject();
	void createExerciseObject_part2(const int object_idx = -1);
	void createExercisesObjects();

	Q_INVOKABLE inline QQuickItem* getExerciseObject(const uint exercise_idx) const { return m_currentExercises->exerciseEntry(exercise_idx); }
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	Q_INVOKABLE void clearExercises();
	Q_INVOKABLE void moveExercise(const uint exercise_idx, const uint new_idx);
	Q_INVOKABLE void rollUpExercises() const;
	Q_INVOKABLE void manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	Q_INVOKABLE void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, const bool bNewSet,
									 const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	void createSetObject_part2(const uint set_type = 0, const uint set_number = 0, const uint exercise_idx = 0, const bool bNewSet = false);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
									  const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	Q_INVOKABLE void removeSetObject(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE void changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text, const bool bChangeModel = true);
	Q_INVOKABLE void changeSetType(const uint set_number, const uint exercise_idx, const uint new_type);
	Q_INVOKABLE QQuickItem* nextSetObject(const uint exercise_idx, const uint set_number) const;
	Q_INVOKABLE void copyTypeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTimeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyRepsValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void copyWeightValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	void enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed);
	void enableDisableSetsRestTime(const uint exercise_idx, const uint bTrackRestTime, const uint bAutoRestTime, const uint except_set_number = 0);
	void findSetMode(const uint exercise_idx, const uint set_number);
	void findCurrentSet(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void changeSetMode(const uint exercise_idx, const uint set_number);
	void startRestTimer(const uint exercise_idx, const uint set_number);
	void stopRestTimer(const uint exercise_idx, const uint set_number);
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);
	Q_INVOKABLE void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

public slots:
	void requestTimerDialog(QQuickItem* requester, const QVariant& args);
	void requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id);
	void requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset);
	void showRemoveExerciseMessage(int exercise_idx);
	void showRemoveSetMessage(int set_number, int exercise_idx);
	void exerciseCompleted(int exercise_idx);
	void openMainMenuShortCut(const int button_id);
	void exportSlot(const QString& filePath = QString());
	void importSlot();

signals:
	void itemReady(QQuickItem* item, const uint id);
	void mesoIdxChanged();

private:
	uint m_mesoIdx;
	static QQuickWindow* app_MainWindow;
	static QQuickItem* app_StackView;
	friend QQuickWindow* appMainWindow();
	friend QQuickItem* appStackView();

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	static QQmlComponent* exercisesComponent;
	static QQuickItem* exercisesPage;
	static QVariantMap exercisesProperties;
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	QQmlComponent* m_mesoComponent;
	QQuickItem* m_mesoPage;
	QVariantMap m_mesoProperties;
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	QQmlComponent* m_plannerComponent;
	QQuickItem* m_plannerPage;
	QVariantMap m_plannerProperties;

	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QMap<QChar,DBMesoSplitModel*> m_splitModels;
	QVariantMap m_splitProperties;
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	QQmlComponent* m_calComponent;
	QQuickItem* m_calPage;
	QVariantMap m_calProperties;
	uint m_lastUsedCalMesoID;
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	struct tDayExercises {
		struct exerciseObject {
			QQuickItem* m_exerciseEntry;
			QList<QQuickItem*> m_setObjects;
			TPTimer* m_setTimer;

			exerciseObject() : m_setTimer(nullptr) {}
		};
		QList<exerciseObject*> exerciseObjects;

		inline QQuickItem* exerciseEntry_const(const uint exercise_idx) const { return exerciseObjects.at(exercise_idx)->m_exerciseEntry; }
		inline QQuickItem* exerciseEntry(const uint exercise_idx) { return exerciseObjects[exercise_idx]->m_exerciseEntry; }
		inline QQuickItem* setObject_const(const uint exercise_idx, const uint set_number) const { return exerciseObjects.at(exercise_idx)->m_setObjects.at(set_number); }
		inline QQuickItem* setObject(const uint exercise_idx, const uint set_number) const { return exerciseObjects.at(exercise_idx)->m_setObjects[set_number]; }
		inline TPTimer* setTimer(const uint exercise_idx)
		{
			if (!exerciseObjects.at(exercise_idx)->m_setTimer)
				exerciseObjects[exercise_idx]->m_setTimer = new TPTimer();
			return exerciseObjects.at(exercise_idx)->m_setTimer;
		}

		inline uint setCount(const uint exercise_idx) const { return exerciseObjects.at(exercise_idx)->m_setObjects.count(); }
		inline uint exercisesCount() const { return exerciseObjects.count(); }

		void appendExerciseEntry(QQuickItem* new_exerciseItem);
		void removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow = false);
		void removeSet(const uint exercise_idx, const uint set_number);

		inline void insertSet(const uint set_number, const uint exercise_idx, QQuickItem* new_setObject)
		{
			exerciseObjects.at(exercise_idx)->m_setObjects.insert(set_number, new_setObject);
		}
		inline void appendSet(const uint exercise_idx, QQuickItem* new_setObject)
		{
			exerciseObjects.at(exercise_idx)->m_setObjects.append(new_setObject);
		}

		inline void clear(const bool bDeleteNow = false)
		{
			for (int i(exerciseObjects.count() - 1); i >= 0 ; --i)
				removeExerciseEntry(i, bDeleteNow);
		}

		~tDayExercises()
		{
			clear(true);
		}
	};

	QMap<QDate,DBTrainingDayModel*> m_tDayModels;
	QMap<QDate,QQuickItem*> m_tDayPages;
	QMap<QDate,tDayExercises*> m_tDayExercisesList;
	tDayExercises* m_currentExercises;
	QVariantMap m_tDayProperties;
	QQmlComponent* m_tDayComponent;
	DBTrainingDayModel* m_CurrenttDayModel;
	QQuickItem* m_currenttDayPage;

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	QVariantMap m_tDayExerciseEntryProperties;
	QQmlComponent* m_tDayExercisesComponent;
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	QVariantMap m_setObjectProperties;
	QQmlComponent* m_setComponents[3];
	uint m_expectedSetNumber;
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

	//-----------------------------------------------------------USER-----------------------------------------------------------
	static QQuickItem* settingsPage, *clientsOrCoachesPage, *userPage;
	static QQmlComponent* settingsComponent, *clientsOrCoachesComponent;
	static QVariantMap settingsProperties, clientsOrCoachesProperties;
	//-----------------------------------------------------------USER-----------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	QString m_exportFilename;

	QQmlComponent* m_importDlgComponent;
	QQuickItem* m_importDlg;
	QVariantMap m_importDlgProperties;
	uint m_fileContents;
	QString m_importFilename;
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
};

inline QQuickWindow* appMainWindow() { return QmlItemManager::app_MainWindow; }
inline QQuickItem* appStackView() { return QmlItemManager::app_StackView; }
#endif // QMLITEMMANAGER_H
