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

class QmlUserInterface;
class QmlExercisesDatabaseInterface;
class QMLMesoInterface;
class QmlMesoCalendarInterface;
class QmlMesoSplitInterface;
class QmlTDayInterface;

class QmlItemManager : public QObject
{

Q_OBJECT
Q_PROPERTY(int mesoIdx READ mesoIdx WRITE setMesoIdx NOTIFY mesoIdxChanged)

public:
	inline explicit QmlItemManager(const uint meso_idx, QObject* parent = nullptr)
		: QObject{parent}, m_mesoIdx(meso_idx),
			m_tDayComponent(nullptr), m_currenttDayPage(nullptr),
			m_tDayExercisesComponent(nullptr), m_setComponents(nullptr), m_appUsersManager(nullptr) { app_qml_manager = this; }
	~QmlItemManager();
	void configureQmlEngine(QQmlApplicationEngine *qml_engine);

	inline uint mesoIdx() const { return m_mesoIdx; }
	inline void setMesoIdx(const uint new_mesoidx) { m_mesoIdx = new_mesoidx; emit mesoIdxChanged(static_cast<int>(new_mesoidx)); }

	QmlUserInterface* usersManager();
	QmlExercisesDatabaseInterface* exercisesListManager();
	QMLMesoInterface* mesocyclesManager(const uint meso_idx);
	QmlMesoCalendarInterface* calendarManager(const uint meso_idx);
	QmlMesoSplitInterface* splitManager(const uint meso_idx);
	QmlTDayInterface* tDayManager(const uint meso_idx);

	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------
	Q_INVOKABLE void createExerciseObject();
	Q_INVOKABLE void removeExerciseObject(const uint exercise_idx);
	Q_INVOKABLE void clearExercises();
	Q_INVOKABLE void moveExercise(const uint exercise_idx, const uint new_idx);
	Q_INVOKABLE void manageRestTime(const uint exercise_idx, const bool bTrackRestTime, bool bAutoRestTime, const uint new_set_type);
	Q_INVOKABLE uint exerciseSetsCount(const uint exercise_idx) const;
	Q_INVOKABLE uint exerciseDefaultSetType(const uint exercise_idx);
	Q_INVOKABLE void setExerciseDefaultSetType(const uint exercise_idx, const uint set_type);
	Q_INVOKABLE const QString exerciseSets(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseSets(const uint exercise_idx, const QString& new_nsets);
	Q_INVOKABLE const QString exerciseRestTime(const uint exercise_idx) const;
	Q_INVOKABLE void setExerciseRestTime(const uint exercise_idx, const QString& new_resttime);
	Q_INVOKABLE const QString exerciseReps(const uint exercise_idx, const uint composite_idx) const;
	Q_INVOKABLE void setExerciseReps(const uint exercise_idx, const uint composite_idx, const QString& new_nreps);
	Q_INVOKABLE const QString exerciseWeight(const uint exercise_idx, const uint composite_idx) const;
	Q_INVOKABLE void setExerciseWeight(const uint exercise_idx, const uint composite_idx, const QString& new_nweight);
	//-----------------------------------------------------------EXERCISE OBJECTS-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS-------------------------------------------------------------
	Q_INVOKABLE void getSetObjects(const uint exercise_idx);
	Q_INVOKABLE void addNewSet(const uint exercise_idx);
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

	void displayImportDialogMessage(const uint fileContents, const QString& filename);
	//-----------------------------------------------------------OTHER ITEMS-----------------------------------------------------------

public slots:
	//-----------------------------------------------------------SLOTS-----------------------------------------------------------
	void mainWindowStarted() const;
	void displayMessageOnAppWindow(const int message_id, const QString& fileName = QString()) const;
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
	void mesoIdxChanged(const int new_mesoidx);
	void setObjectReady();

private:
	uint m_mesoIdx;

	//-----------------------------------------------------------TRAININGDAY PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------TRAININGDAY PRIVATE-----------------------------------------------------------

	//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------
	QVariantMap m_tDayExerciseEntryProperties;
	QQmlComponent* m_tDayExercisesComponent;

	void createExercisesObjects();
	void createExerciseObject_part2(const int object_idx = -1);
	inline uint exercisesCount() const;
	inline QQuickItem* exerciseEntryItem(const uint exercise_idx);
	inline QQuickItem* exerciseEntryItem(const uint exercise_idx) const;
	inline QQuickItem* exerciseSetItem(const uint exercise_idx, const uint set_number);
	inline QQuickItem* exerciseSetItem(const uint exercise_idx, const uint set_number) const;
	inline void appendExerciseEntry();
	void removeExerciseEntry(const uint exercise_idx, const bool bDeleteNow = false);
	inline void setExerciseItem(const uint exercise_idx, QQuickItem* new_exerciseItem);
	inline const QString& exerciseReps(const uint exercise_idx) const;
	inline void setExerciseReps(const uint exercise_idx, const QString& nreps);
	inline const QString& exerciseWeights(const uint exercise_idx) const;
	inline void setExerciseWeight(const uint exercise_idx, const QString& nweight);
	inline void insertExerciseSet(const uint set_number, const uint exercise_idx, QQuickItem* new_setObject);
	inline void appendExerciseSet(const uint exercise_idx, QQuickItem* new_setObject);
	inline void removeExerciseSet(const uint exercise_idx, const uint set_number);
	inline void clearExerciseEntries(const bool bDeleteNow = false);
	//-----------------------------------------------------------EXERCISE OBJECTS PRIVATE-----------------------------------------------------------

	//-------------------------------------------------------------SET OBJECTS PRIVATE-------------------------------------------------------------
	QVariantMap m_setObjectProperties;
	QQmlComponent* m_setComponents[3];
	uint m_expectedSetNumber;

	void createSetObject(const uint exercise_idx, const uint set_number, const bool bNewSet, const uint set_type,
									 const QString& nReps = QString(), const QString& nWeight = QString(), const QString& nRestTime = QString());
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

	void addMainMenuShortCut(const QString& label, QQuickItem* page);
	void removeMainMenuShortCut(QQuickItem* page);

	QQmlApplicationEngine* m_appQmlEngine;
	QQuickWindow* m_appMainWindow;
	QmlUserInterface* m_appUsersManager;
	QmlExercisesDatabaseInterface* m_appExercisesListManager;
	QList<QMLMesoInterface*> m_appMesosManager;
	QList<QmlMesoCalendarInterface*> m_appCalendarManager;
	QList<QmlMesoSplitInterface*> m_appSplitManager;
	QList<QmlTDayInterface*> m_appTDayManager;

	static QmlItemManager* app_qml_manager;
	friend QmlItemManager* appQmlManager();
	//-----------------------------------------------------------OTHER ITEMS PRIVATE-----------------------------------------------------------
};

inline QmlItemManager* appQmlManager() { return QmlItemManager::app_qml_manager; }

#endif // QMLITEMMANAGER_H
