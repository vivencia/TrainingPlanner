#ifndef QMLEXERCISEINTERFACE_H
#define QMLEXERCISEINTERFACE_H

#include <QObject>
#include <QVariantMap>

class QmlExerciseEntry;
class DBWorkoutModel;
class TPTimer;
class QmlTDayInterface;

class QQmlComponent;
class QQuickItem;

class QmlExerciseInterface : public QObject
{

Q_OBJECT

public:
	inline explicit QmlExerciseInterface(QObject* parent, QmlTDayInterface* workoutPage, DBWorkoutModel *tDayModel, QQuickItem* parentLayout)
		: QObject{parent}, m_workoutPage(workoutPage), m_workoutModel(tDayModel), m_parentLayout(parentLayout),
			m_exercisesComponent(nullptr), m_simpleExercisesListRequester(-1) {}
	~QmlExerciseInterface();

	void createExerciseObject();
	void createExercisesObjects();
	void removeExerciseObject(const uint exercise_idx);
	void removeExerciseSet(const uint exercise_idx, const uint set_number);
	void clearExercises();
	void setExercisesEditable(const bool editable);
	void moveExercise(const uint exercise_idx, const uint new_idx);
	void gotoNextExercise(const uint exercise_idx) const;
	void hideSets() const;
	void showSimpleExercisesList(const uint exercise_idx, const bool bMultiSel);
	inline uint exercisesCount() const { return m_exercisesList.count(); }
	inline QmlExerciseEntry* exerciseEntry(const uint exercise_idx) const { return m_exercisesList.at(exercise_idx); }

private:
	QmlTDayInterface* m_workoutPage;
	DBWorkoutModel* m_workoutModel;
	QQuickItem* m_parentLayout;
	QVariantMap m_exercisesProperties;
	QQmlComponent* m_exercisesComponent;
	QList<QmlExerciseEntry*> m_exercisesList;
	int m_simpleExercisesListRequester;

	void createExerciseObject_part2(const uint exercise_idx);
};

#endif // QMLEXERCISEINTERFACE_H
