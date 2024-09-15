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

class QmlItemManager : public QObject
{

Q_OBJECT
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged)

public:
	QmlItemManager(const uint meso_idx, QObject* parent = nullptr)
		: QObject{parent}, m_mesoIdx(meso_idx),
			m_mesoComponent(nullptr), m_plannerComponent(nullptr),
			m_splitComponent(nullptr), m_calComponent(nullptr), m_tDayComponent(nullptr), m_tDayExercisesComponent(nullptr),
			m_setComponents{nullptr} {}
	~QmlItemManager();
	static void configureQmlEngine();

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; emit mesoIdxChanged(); }

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	void createExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	void createExercisesPage_part2(QQuickItem* connectPage);
	void getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();
	void getMesoPage();
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	Q_INVOKABLE void createPlannerPage();
	void createPlannerPage_part2();
	void getExercisesPlannerPage();

	void createMesoSplitPage();
	void createMesoSplitPage_part2();

	inline DBMesoSplitModel* getSplitModel(const QChar& splitLetter)
	{
		if (!m_splitModels.contains(splitLetter))
		{
			DBMesoSplitModel* splitModel{new DBMesoSplitModel(this, true, m_mesoIdx)};
			connect(this, &QmlItemManager::mesoIdxChanged, splitModel, [&,splitModel] { splitModel->setMesoIdx(m_mesoIdx); });
			m_splitModels.insert(splitLetter, splitModel);
		}
		return m_splitModels.value(splitLetter);
	}
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	void swapPlans(const QString& splitLetter1, const QString& splitLetter2);
	void updateMuscularGroup(DBMesoSplitModel* splitModel);
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	uint createMesoCalendarPage();
	void createMesoCalendarPage_part2();
	void getCalendarPage();
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	uint createTrainingDayPage(const QDate& date);
	void createTrainingDayPage_part2();
	void getTrainingDayPage(const QDate& date);

	DBTrainingDayModel* gettDayModel(const QDate& date);
	inline DBTrainingDayModel* currenttDayModel() { return m_CurrenttDayModel; }
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void setCurrenttDay(const QDate& date);
	inline bool setsLoaded(const uint exercise_idx) const { return m_currentExercises->setCount(exercise_idx) > 0; }
	void updateOpenTDayPagesWithNewCalendarInfo(const uint meso_idx, const QDate& startDate, const QDate& endDate);
	void setTrainingDayPageEmptyDayOptions(const DBTrainingDayModel* const model);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	Q_INVOKABLE uint createExerciseObject();
	void createExerciseObject_part2(const int object_idx = -1);
	void createExercisesObjects();

	Q_INVOKABLE inline QQuickItem* getExerciseObject(const uint exercise_idx) const { return m_currentExercises->exerciseEntry(exercise_idx); }
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	void clearExercises();
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

	//-----------------------------------------------------------USER-----------------------------------------------------------
	Q_INVOKABLE void openSettingsPage(const uint startPageIndex);
	void createSettingsPage();
	Q_INVOKABLE void openClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);
	void createClientsOrCoachesPage();
	void setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches);
	void removeUser(const uint user_row, const bool bCoach);
	//-----------------------------------------------------------USER-----------------------------------------------------------

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
	QString m_createdSplits;
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
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

	static QmlItemManager* app_rootItemsManager;
	friend QmlItemManager* rootItemsManager();
};

inline QQuickWindow* appMainWindow() { return QmlItemManager::app_MainWindow; }
inline QQuickItem* appStackView() { return QmlItemManager::app_StackView; }

inline QmlItemManager* rootItemsManager()
{
	if (!QmlItemManager::app_rootItemsManager)
		QmlItemManager::app_rootItemsManager = new QmlItemManager(0xFFFF);
	return QmlItemManager::app_rootItemsManager;
}
#endif // QMLITEMMANAGER_H