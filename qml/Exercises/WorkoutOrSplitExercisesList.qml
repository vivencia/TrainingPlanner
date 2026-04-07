pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets
import TpQml.Pages

TPListView {
	id: _control
	model: exercisesModel
	spacing: 5

//public:
	required property WorkoutManager workoutPageManager
	required property SplitManager splitPageManager
	property DBExercisesModel exercisesModel
	property bool viewPositionAtBeginning: true
	property bool viewPositionAtEnd: false

	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: ScrollBar.AsNeeded

		onPositionChanged: {
			if (_control.exercisesModel.isWorkout) return;
			_control.viewPositionAtBeginning = _control.contentY <= 100;
			if (_control.itemAt(0, _control.contentY))
				_control.viewPositionAtEnd = _control.contentY +
												_control.itemAt(0, _control.contentY).height >= _control.contentHeight - 100;
		}
	}

	delegate: ExercisesListDelegate {
		exercisesModel: _control.exercisesModel
		exerciseNumber: index
		onGotoExercise: (exercise_number) => _control.positionViewAtIndex(exercise_number, ListView.Contain);
		onRemoveExerciseRequested: _control.showDeleteDialog();
		onQuickQuestionRequested: (title, message, icon, button1text, button2text) =>
													_control.quickQuestion(this, title, message, icon, button1text, button2text);

		required property int index
	}

	function showDeleteDialog(exercise_number: int): void {
		if (AppSettings.alwaysAskConfirmation)
			delExerciseLoader.active = true;
		else
			_control.exercisesModel.delExercise(_control.exercisesModel.workingExercise);
	}

	Loader {
		id: delExerciseLoader
		active: false
		asynchronous: true

		property TPBalloonTip _balloon
		property string _exerciseName

		sourceComponent: TPBalloonTip {
			parentPage: _control.workoutPageManager ? _control.workoutPageManager.qmlPage() as TPPage :
																					_control.splitPageManager.qmlPage() as TPPage
			keepAbove: true
			title: qsTr("Remove Exercise?")
			message: delExerciseLoader._exerciseName + qsTr("\nThis action cannot be undone.")
			imageSource: "remove"
			onButton1Clicked: _control.exercisesModel.delExercise(_control.exercisesModel.workingExercise);
			onClosed: delExerciseLoader.active = false;
			Component.onCompleted: delExerciseLoader._balloon = this;
		}

		onLoaded: {
			_exerciseName = _control.exercisesModel.exerciseName(_control.exercisesModel.workingExercise,
																						_control.exercisesModel.allExerciseNames);
			_balloon.showInWindow(-Qt.AlignCenter);
		}
	}

	Loader {
		id: quickQuestionLoader
		asynchronous: true
		active: false

		property TPBalloonTip dialog
		property ExercisesListDelegate delegate
		property string title
		property string message
		property string icon
		property string button1Text
		property string button2Text

		sourceComponent: TPBalloonTip {
			parentPage: _control.workoutPageManager ? _control.workoutPageManager.qmlPage() as TPPage :
																					_control.splitPageManager.qmlPage() as TPPage
			keepAbove: true
			title: quickQuestionLoader.title
			message: quickQuestionLoader.message
			imageSource: quickQuestionLoader.icon
			button1Text: quickQuestionLoader.button1Text
			button2Text: quickQuestionLoader.button2Text
			onButton1Clicked: quickQuestionLoader.delegate.quickQuestionAnswered(1);
			onButton2Clicked: quickQuestionLoader.delegate.quickQuestionAnswered(2);
			onClosed: {
				quickQuestionLoader.delegate.quickQuestionAnswered(0);
				quickQuestionLoader.active = false
			}
			Component.onCompleted: quickQuestionLoader.dialog = this;
		}

		onLoaded: dialog.showInWindow(-Qt.AlignCenter);
	}
	function quickQuestion(delegate: ExercisesListDelegate, title: string, message: string, icon: string, button1text:
																							string, button2text: string): void {
		quickQuestionLoader.delegate = delegate;
		quickQuestionLoader.title = title;
		quickQuestionLoader.message = message;
		quickQuestionLoader.icon = icon;
		quickQuestionLoader.button1Text = button1text;
		quickQuestionLoader.button2Text = button2text;
		quickQuestionLoader.active = true;
	}

	Loader {
		id: clearExercisesLoader
		active: false
		asynchronous: true

		property TPBalloonTip _balloon

		sourceComponent: TPBalloonTip {
			title: qsTr("Remove all exercises?")
			message: qsTr("This action cannot be undone.")
			imageSource: "remove"
			keepAbove: true
			onButton1Clicked: _control.exercisesModel.clearExercises();
			parentPage: _control.workoutPageManager ? _control.workoutPageManager.qmlPage() as TPPage : _control.splitPageManager.qmlPage() as TPPage
			onClosed: clearExercisesLoader.active = false;
			Component.onCompleted: clearExercisesLoader._balloon = this;
		}

		onLoaded: _balloon.showInWindow(-Qt.AlignCenter);
	}
	function showClearExercisesMessage(): void {
		if (AppSettings.alwaysAskConfirmation)
			clearExercisesLoader.active = true;
		else
			_control.exercisesModel.clearExercises();
	}

	function appendNewExerciseToDivision(): void {
		_control.exercisesModel.setWorkingExercise = _control.exercisesModel.addExercise();
		_control.currentIndex = _control.exercisesModel.workingExercise;
		_control.positionViewAtIndex(_control.exercisesModel.workingExercise, ListView.Contain);
	}

	function exerciseNameFieldYPosition(): int {
		let list_item = itemAtIndex(_control.exercisesModel.workingExercise) as ExercisesListDelegate;
		if (list_item)
			return list_item.exerciseFieldYPos();
		else
			return 0;
	}
} //ListView
