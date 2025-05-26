#pragma once

#include <QObject>
#include <QVariantMap>

QT_FORWARD_DECLARE_CLASS(DBExercisesModel)
QT_FORWARD_DECLARE_CLASS(QmlExerciseEntry)
QT_FORWARD_DECLARE_CLASS(QmlWorkoutInterface)
QT_FORWARD_DECLARE_CLASS(TPTimer)

QT_FORWARD_DECLARE_CLASS(QQmlComponent)
QT_FORWARD_DECLARE_CLASS(QQuickItem)

class QmlExerciseInterface : public QObject
{

Q_OBJECT

public:
	inline explicit QmlExerciseInterface(QObject *parent, QmlWorkoutInterface *workout_Page,
														DBExercisesModel *workout_Model, QQuickItem *parent_Layout)
		: QObject{parent}, m_workoutPage{workout_Page}, m_workoutModel{workout_Model}, m_parentLayout{parent_Layout},
			m_exercisesComponent{nullptr}, m_simpleExercisesListRequester{-1} {}
	~QmlExerciseInterface();

	void createExerciseObject();
	void createExercisesObjects();
	void removeExerciseObject(const uint exercise_number);
	void removeExerciseSet(const uint exercise_number, const uint set_number);
	void clearExercises();
	void setExercisesEditable(const bool editable);
	void moveExercise(const uint exercise_number, const uint new_exercisenumber);
	void gotoNextExercise(const uint exercise_number) const;
	void hideSets() const;
	void showSimpleExercisesList(const uint exercise_number, const bool bMultiSel);
	inline uint exercisesCount() const { return m_exercisesList.count(); }
	inline QmlExerciseEntry *exerciseEntry(const uint exercise_number) const { return m_exercisesList.at(exercise_number); }

private:
	QmlWorkoutInterface *m_workoutPage;
	DBExercisesModel *m_workoutModel;
	QQuickItem *m_parentLayout;
	QVariantMap m_exercisesProperties;
	QQmlComponent *m_exercisesComponent;
	QList<QmlExerciseEntry*> m_exercisesList;
	int m_simpleExercisesListRequester;

	void createExerciseObject_part2(const uint exercise_number);
};
