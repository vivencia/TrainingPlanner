set( QML_EXERCISES
	"ExerciseNameField.qml"
	"ExercisesListDelegate.qml"
	"ExercisesListView.qml"
	"SetInputField.qml"
	"SetNotesField.qml"
	"SimpleExercisesListPanel.qml"
	"WorkoutOrSplitExercisesList.qml"
)

qt_add_library(tp_exercises_module STATIC)
qt_add_qml_module(tp_exercises_module
	URI TpQml.Exercises
	VERSION 1.0
	QML_FILES
		${QML_EXERCISES}
	RESOURCE_PREFIX
		"/"
)
