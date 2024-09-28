#ifndef QMLITEMMANAGER_H
#define QMLITEMMANAGER_H

#include "tpglobals.h"
#include "tptimer.h"

#include <QObject>
#include <QMap>
#include <QDate>

class DBInterface;
class DBUserModel;
class DBMesocyclesModel;
class DBExercisesModel;
class DBTrainingDayModel;
class DBMesoSplitModel;

class QQmlApplicationEngine;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;

class QmlItemManager : public QObject
{

Q_OBJECT
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged)

public:
	inline explicit QmlItemManager(const uint meso_idx, QObject* parent = nullptr)
		: QObject{parent}, m_mesoIdx(meso_idx),
			m_mesoComponent(nullptr), m_plannerComponent(nullptr),
			m_splitComponent(nullptr), m_calComponent(nullptr), m_tDayComponent(nullptr), m_tDayExercisesComponent(nullptr),
			m_setComponents{nullptr} { if (!app_root_items_manager) app_root_items_manager = this; }
	~QmlItemManager();
	void configureQmlEngine(QQmlApplicationEngine *qml_engine);

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; emit mesoIdxChanged(); }

	//-----------------------------------------------------------USER-----------------------------------------------------------
	Q_INVOKABLE void getSettingsPage(const uint startPageIndex);
	Q_INVOKABLE void getClientsOrCoachesPage(const bool bManageClients, const bool bManageCoaches);
	Q_INVOKABLE void removeUser(const uint user_row, const bool bCoach);
	//-----------------------------------------------------------USER-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------
	Q_INVOKABLE const uint removeExercise(const uint row);
	Q_INVOKABLE void exportExercises(const bool bShare);
	Q_INVOKABLE void importExercises(const QString& filename = QString());

	Q_INVOKABLE void getExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	//-----------------------------------------------------------EXERCISES TABLE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------
	Q_INVOKABLE void changeMesoCalendar(const bool preserve_old_cal, const bool preserve_untilyesterday);

	void getMesocyclePage();
	void exportMeso(const bool bShare, const bool bCoachInfo);
	void importMeso(const QString& filename = QString());
	//-----------------------------------------------------------MESOCYCLES-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------
	Q_INVOKABLE void getMesoCalendarPage();
	//-----------------------------------------------------------MESOCALENDAR-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------
	Q_INVOKABLE void getExercisesPlannerPage();
	Q_INVOKABLE void getMesoSplitPage(const uint page_index);
	Q_INVOKABLE void swapMesoPlans(const QString& splitLetter1, const QString& splitLetter2);
	Q_INVOKABLE void exportMesoSplit(const bool bShare, const QString& splitLetter, const QString& filePath = QString(), const bool bJustExport = false);
	Q_INVOKABLE void importMesoSplit(const QString& filename = QString());

	DBMesoSplitModel* getSplitModel(const QChar& splitLetter);
	inline QMap<QChar,DBMesoSplitModel*>& allSplitModels() { initializeSplitModels(); return m_splitModels; }
	inline QQuickItem* getSplitPage(const QChar& splitLetter) const { return m_splitPages.value(splitLetter); }
	//-----------------------------------------------------------MESOSPLIT-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------
	Q_INVOKABLE void getTrainingDayPage(const QDate& date);
	Q_INVOKABLE void loadExercisesFromDate(const QString& strDate);
	Q_INVOKABLE void loadExercisesFromMesoPlan();
	Q_INVOKABLE void convertTDayToPlan();
	Q_INVOKABLE void resetWorkout();
	Q_INVOKABLE void setCurrenttDay(const QDate& date);
	Q_INVOKABLE void exportTrainingDay(const bool bShare, const DBTrainingDayModel* const tDayModel);
	Q_INVOKABLE void importTrainingDay(const QString& filename = QString());

	DBTrainingDayModel* gettDayModel(const QDate& date);
	inline DBTrainingDayModel* currenttDayModel() { return m_CurrenttDayModel; }	
	inline bool setsLoaded(const uint exercise_idx) const { return m_currentExercises->setCount(exercise_idx) > 0; }
	//-----------------------------------------------------------TRAININGDAY-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	Q_INVOKABLE void createExerciseObject();
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	Q_INVOKABLE void clearExercises();
	Q_INVOKABLE void moveExercise(const uint exercise_idx, const uint new_idx);
	Q_INVOKABLE void rollUpExercises() const;
	Q_INVOKABLE void manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type);

	inline QQuickItem* getExerciseObject(const uint exercise_idx) const { return m_currentExercises->exerciseEntry(exercise_idx); }
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	Q_INVOKABLE void createSetObject(const uint set_type, const uint set_number, const uint exercise_idx, const bool bNewSet,
									 const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	Q_INVOKABLE void createSetObjects(const uint exercise_idx);
	Q_INVOKABLE void createSetObjects(const uint exercise_idx, const uint first_set, const uint last_set, const uint set_type,
									  const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
	Q_INVOKABLE void removeSetObject(const uint set_number, const uint exercise_idx);
	Q_INVOKABLE void changeSetsExerciseLabels(const uint exercise_idx, const uint label_idx, const QString& new_text, const bool bChangeModel = true);
	Q_INVOKABLE void changeSetType(const uint set_number, const uint exercise_idx, const uint new_type);
	Q_INVOKABLE void changeSetMode(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTypeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyTimeValueIntoOtherSets(const uint exercise_idx, const uint set_number);
	Q_INVOKABLE void copyRepsValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE void copyWeightValueIntoOtherSets(const uint exercise_idx, const uint set_number, const uint sub_set = 0);
	Q_INVOKABLE QQuickItem* nextSetObject(const uint exercise_idx, const uint set_number) const;
	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------
	Q_INVOKABLE void openMainMenuShortCut(const int button_id);
	Q_INVOKABLE inline void addMainMenuShortCutEntry(QQuickItem* entry) { m_mainMenuShortcutEntries.append(entry); }
	Q_INVOKABLE void tryToImport(const QList<bool>& selectedFields);
	void displayActivityResultMessage(const int requestCode, const int resultCode) const;
	void displayMessageOnAppWindow(const int message_id) const;
	void displayMessageOnAppWindow(const QString& title, const QString& message) const;
	void displayImportDialogMessage(const uint fileContents, const QString& filename);
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

public slots:
	//-----------------------------------------------------------SLOTS-----------------------------------------------------------
	void mainWindowStarted() const;
	void requestTimerDialog(QQuickItem* requester, const QVariant& args);
	void requestExercisesList(QQuickItem* requester, const QVariant& visible, const QVariant& multipleSelection, int id);
	void requestFloatingButton(const QVariant& exercise_idx, const QVariant& set_type, const QVariant& nset);
	void showRemoveExerciseMessage(int exercise_idx);
	void showRemoveSetMessage(int set_number, int exercise_idx);
	void exerciseCompleted(int exercise_idx);
	void exportSlot(const QString& filePath = QString());
	void importSlot_FileChosen(const QString& filePath = QString());
	//-----------------------------------------------------------SLOTS-----------------------------------------------------------

signals:
	void mesoIdxChanged();
	void setObjectReady();

private:
	uint m_mesoIdx;

	//-----------------------------------------------------------USER PRIVATE-----------------------------------------------------------
	static QQuickItem* settingsPage, *clientsOrCoachesPage, *userPage;
	static QQmlComponent* settingsComponent, *clientsOrCoachesComponent;
	static QVariantMap settingsProperties, clientsOrCoachesProperties;

	void createSettingsPage();
	void createClientsOrCoachesPage();
	//-----------------------------------------------------------USER PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISES TABLE PRIVATE-----------------------------------------------------------
	static QQmlComponent* exercisesComponent;
	static QQuickItem* exercisesPage;
	static QVariantMap exercisesProperties;

	void createExercisesPage(const bool bChooseButtonEnabled, QQuickItem* connectPage);
	void createExercisesPage_part2(QQuickItem* connectPage);
	//-----------------------------------------------------------EXERCISES TABLE PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCYCLES PRIVATE-----------------------------------------------------------
	QQmlComponent* m_mesoComponent;
	QQuickItem* m_mesoPage;
	QVariantMap m_mesoProperties;
	uint m_mesoMuscularGroupId;

	void createMesocyclePage(const QDate& minimumMesoStartDate = QDate(), const QDate& maximumMesoEndDate = QDate(),
								const QDate& calendarStartDate = QDate());
	void createMesocyclePage_part2();

	friend class TPAppControl;
	//-----------------------------------------------------------MESOCYCLES PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------MESOCALENDAR PRIVATE-----------------------------------------------------------
	QQmlComponent* m_calComponent;
	QQuickItem* m_calPage;
	QVariantMap m_calProperties;
	uint m_lastUsedCalMesoID;

	void createMesoCalendarPage();
	void createMesoCalendarPage_part2();
	//-----------------------------------------------------------MESOCALENDAR PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------MESOSPLIT PRIVATE-----------------------------------------------------------
	QQmlComponent* m_plannerComponent;
	QQuickItem* m_plannerPage;
	QVariantMap m_plannerProperties;

	QQmlComponent* m_splitComponent;
	QMap<QChar,QQuickItem*> m_splitPages;
	QMap<QChar,DBMesoSplitModel*> m_splitModels;
	QVariantMap m_splitProperties;
	uint m_splitMuscularGroupId;

	void createPlannerPage();
	void createPlannerPage_part2();
	void createMesoSplitPage(const uint page_index);
	void initializeSplitModels();
	void setSplitPageProperties(QQuickItem* splitPage, const DBMesoSplitModel* const splitModel);
	void updateMuscularGroup(DBMesoSplitModel* splitModel);
	void changeMuscularGroup(const QString& new_musculargroup, DBMesoSplitModel* splitModel, const uint initiator_id);
	//-----------------------------------------------------------MESOSPLIT PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY PRIVATE-----------------------------------------------------------
	void createTrainingDayPage(const QDate& date);
	void createTrainingDayPage_part2();
	void updateOpenTDayPagesWithNewCalendarInfo(const QDate& startDate, const QDate& endDate);
	void setTrainingDayPageEmptyDayOptions(const DBTrainingDayModel* const model);

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
	//-----------------------------------------------------------TRAININGDAY PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------
	QVariantMap m_tDayExerciseEntryProperties;
	QQmlComponent* m_tDayExercisesComponent;

	void createExercisesObjects();
	void createExerciseObject_part2(const int object_idx = -1);
	//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS PRIVATE-------------------------------------------------------------
	QVariantMap m_setObjectProperties;
	QQmlComponent* m_setComponents[3];
	uint m_expectedSetNumber;

	void createSetObject_part2(const uint set_type = 0, const uint set_number = 0, const uint exercise_idx = 0, const bool bNewSet = false);
	void enableDisableExerciseCompletedButton(const uint exercise_idx, const bool completed);
	void enableDisableSetsRestTime(const uint exercise_idx, const uint bTrackRestTime, const uint bAutoRestTime, const uint except_set_number = 0);
	void findSetMode(const uint exercise_idx, const uint set_number);
	void findCurrentSet(const uint exercise_idx, const uint set_number);

	void startRestTimer(const uint exercise_idx, const uint set_number);
	void stopRestTimer(const uint exercise_idx, const uint set_number);
	//-------------------------------------------------------------SET OBJECTS PRIVATE-------------------------------------------------------------

	//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
	QList<QQuickItem*> m_mainMenuShortcutPages;
	QList<QQuickItem*> m_mainMenuShortcutEntries;
	uint m_fileContents;
	QString m_exportFilename, m_importFilename;

	void setClientsOrCoachesPagesProperties(const bool bManageClients, const bool bManageCoaches);
	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);

	static QmlItemManager* app_root_items_manager;
	friend QmlItemManager* rootItemsManager();
	static QQmlApplicationEngine* app_qml_engine;
	friend QQmlApplicationEngine* appQmlEngine();
	static QQuickWindow* app_MainWindow;
	friend QQuickWindow* appMainWindow();
	static QQuickItem* app_StackView;
	friend QQuickItem* appStackView();
	//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
};

inline QmlItemManager* rootItemsManager() { return QmlItemManager::app_root_items_manager; }
inline QQmlApplicationEngine* appQmlEngine() { return QmlItemManager::app_qml_engine; }
inline QQuickWindow* appMainWindow() { return QmlItemManager::app_MainWindow; }

#endif // QMLITEMMANAGER_H
