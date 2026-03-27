pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import TpQml
import TpQml.Widgets
import TpQml.Exercises

TPPage {
	id: pagePlanner
	imageSource: ":/images/backgrounds/backimage-splits.jpg"
	backgroundOpacity: 0.6
	objectName: "exercisesPlanner"

//public:
	required property SplitManager splitManager
	property WorkoutOrSplitExercisesList currentSplitPage: null

//private:
	readonly property int splitPageHeight: AppSettings.pageHeight - topToolBar.height - bottomToolBar.height

	Keys.onPressed: (event) => {
		if (event.key === ItemManager.AppPagesManager.backKey()) {
			if (swipeView.currentIndex !== 0) {
				event.accepted = true;
				swipeView.decrementCurrentIndex();
			}
		}
	}

	header: TPToolBar {
		id: topToolBar
		height: AppSettings.pageHeight * 0.3

		ColumnLayout {
			id: toolbarLayout
			spacing: 5
			anchors {
				fill: parent
				margins: 5
			}

			Row {
				Layout.fillWidth: true
				spacing: 2
				padding: 0

				TPButton {
					imageSource: "prev"
					Layout.preferredWidth: AppSettings.itemDefaultHeight
					Layout.preferredHeight: AppSettings.itemDefaultHeight

					onClicked: {
						if (swipeView.currentIndex > 0)
							swipeView.decrementCurrentIndex();
						else
							swipeView.setCurrentIndex(swipeView.count - 1);
					}
				}

				TPLabel {
					id: lblMain
					text: qsTr("Training Division ") + pagePlanner.currentSplitPage && pagePlanner.currentSplitPage.exercisesModel ?
							  pagePlanner.currentSplitPage.exercisesModel.splitLetter : ""
					font: AppGlobals.largeFont
					horizontalAlignment: Text.AlignHCenter
					Layout.preferredWidth: parent.width - AppSettings.itemDefaultHeight * 2 - 5
				}

				TPButton {
					imageSource: "next"
					Layout.preferredWidth: AppSettings.itemDefaultHeight
					Layout.preferredHeight: AppSettings.itemDefaultHeight

					onClicked: {
						if (swipeView.currentIndex < swipeView.count - 1)
							swipeView.incrementCurrentIndex();
						else
							swipeView.setCurrentIndex(0);
					}
				}
			} //Row

			TPLabel {
				id: lblGroups
				text: qsTr("Muscle groups trained in this division:")
				Layout.fillWidth: true
			}

			TPMuscularGroupsList {
				id: cboGroups
				Layout.fillWidth: true
			}

			Row {
				enabled: pagePlanner.currentSplitPage && pagePlanner.currentSplitPage.exercisesModel &&
																pagePlanner.currentSplitPage.exercisesModel.exerciseCount > 1
				spacing: 0
				Layout.preferredHeight: AppSettings.itemDefaultHeight
				Layout.fillWidth: true

				TPLabel {
					text: qsTr("Go to exercise: ")
					Layout.preferredWidth: parent.width * 0.4
				}

				TPComboBox {
					id: cboGoToExercise
					Layout.preferredWidth: parent.width * 0.6

					property int current_exercise

					model: ListModel {
						id: cboModel
					}

					onActivated: (cboIndex) => {
						pagePlanner.currentSplitPage.positionViewAtIndex(cboIndex, ListView.Contain);
						pagePlanner.currentSplitPage.exercisesModel.workingExercise = cboIndex;
					}

					function changeExercisename(exercise_number: int, new_name: string): void {
						cboModel.setProperty(exercise_number, "text", new_name);

					}

					function addExerciseToCombo(exercise_number: int): void {
						cboModel.append({
							"text": String(exercise_number+1) + ": " +
												pagePlanner.currentSplitPage.exercisesModel.exerciseName(exercise_number, 0),
							"value": exercise_number,
							"enabled": exercise_number !== pagePlanner.currentSplitPage.exercisesModel.workingExercise});
					}

					function delExerciseFromCombo(exercise_number: int): void {
						cboModel.remove(exercise_number);
					}

					function manageComboItems(exercise_number: int): void {
						if (cboModel.count > 0) {
							cboModel.get(exercise_number).enabled = false;
							if (cboGoToExercise.current_exercise >= 0 && cboGoToExercise.current_exercise !== exercise_number)
								cboModel.get(cboGoToExercise.current_exercise).enabled = true;
							cboGoToExercise.current_exercise = exercise_number;
							cboGoToExercise.currentIndex = exercise_number;
						}
					}

					function populateComboModel(): void {
						for (let i = 0; i < pagePlanner.currentSplitPage.exercisesModel.exerciseCount; ++i)
							addExerciseToCombo(i);
						manageComboItems(pagePlanner.currentSplitPage.exercisesModel.workingExercise);
					}

					Connections {
						target: pagePlanner.currentSplitPage ? pagePlanner.currentSplitPage.exercisesModel : null

						function onWorkingExerciseChanged(exercise_number: int): void {
							cboGoToExercise.manageComboItems(exercise_number);
						}

						function onExerciseCountChanged(): void {
							if (pagePlanner.currentSplitPage.exercisesModel.exerciseCount > cboModel.count)
								cboGoToExercise.addExerciseToCombo(pagePlanner.currentSplitPage.exercisesModel.exerciseCount - 1);
							else if (pagePlanner.currentSplitPage.exercisesModel.exerciseCount < cboModel.count)
								cboGoToExercise.delExerciseFromCombo(pagePlanner.currentSplitPage.exercisesModel.workingExercise);
						}

						function onExerciseNameChanged(exercise_number: int, exercise_idx: int): void {
							if (exercise_idx === 0)
								cboGoToExercise.changeExercisename(exercise_number,
														pagePlanner.currentSplitPage.exercisesModel.exerciseName(exercise_number, 0));
						}
					}
				}
			}
		}
	}

	SwipeView {
		id: swipeView
		objectName: "swipeView"
		interactive: true
		anchors.fill: parent

		onCurrentIndexChanged: {
			pagePlanner.currentSplitPage = pagePlanner.splitManager.setCurrentPage(currentIndex);
			cboModel.clear();
			cboGroups.fillMuscularGroupsModel(pagePlanner.currentSplitPage.exercisesModel.muscularGroup);
			cboGoToExercise.populateComboModel();
		}
	} //SwipeView

	PageIndicator {
		id: indicator
		count: swipeView.count
		currentIndex: swipeView.currentIndex
		height: 20

		delegate: Label {
			id: lblSplitLetter
			text: pagePlanner.splitManager.splitModel(index)
			color: AppSettings.fontColor
			font.bold: true
			fontSizeMode: Text.Fit
			width: AppSettings.itemSmallHeight
			height: AppSettings.itemSmallHeight
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter

			required property int index

			background: Rectangle {
				radius: width / 2
				opacity: lblSplitLetter.index === indicator.currentIndex ? 0.95 : 0.7
				color: AppSettings.paneBackgroundColor
			}

			MouseArea {
				anchors.fill: parent
				onClicked: swipeView.setCurrentIndex(lblSplitLetter.index);
			}

			Behavior on opacity {
				OpacityAnimator {
					duration: 200
				}
			}
		}

		anchors {
			bottom: parent.bottom
			bottomMargin: 10
			horizontalCenter: parent.horizontalCenter
		}
	}

	footer: TPToolBar {
		id: bottomToolBar
		height: pagePlanner.footerHeight

		readonly property int buttonWidth: width * 0.22

		TPButton {
			id: btnClearPlan
			text: qsTr("Clear")
			imageSource: "clear.png"
			textUnderIcon: true
			rounded: false
			enabled: pagePlanner.splitManager.haveExercises
			width: bottomToolBar.buttonWidth
			height: pagePlanner.footerHeight - 4

			anchors {
				left: parent.left
				leftMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: pagePlanner.splitManager.currentSplitModel.clearExercises();
		}

		TPButton {
			id: btnSwapPlan
			text: pagePlanner.splitManager.currentSplitLetter + " <-> " + pagePlanner.splitManager.currentSwappableLetter
			imageSource: "swap.png"
			textUnderIcon: true
			visible: pagePlanner.splitManager.canSwapExercises
			rounded: false
			width: bottomToolBar.buttonWidth
			height: pagePlanner.footerHeight - 4

			anchors {
				left: btnClearPlan.right
				leftMargin: 3
				verticalCenter: parent.verticalCenter
			}

			onClicked: pagePlanner.splitManager.swapMesoPlans();
		}

		TPButton {
			text: qsTr("+Exercise")
			imageSource: "exercises-add.png"
			textUnderIcon: true
			rounded: false
			width: bottomToolBar.buttonWidth * 1.3
			height: pagePlanner.footerHeight - 4

			anchors {
				right: parent.right
				rightMargin: 5
				verticalCenter: parent.verticalCenter
			}

			onClicked: {
				cboGroups.forceActiveFocus(); //Force triggering of onEditFinished of the last text control to receive input
				pagePlanner.currentSplitPage.appendNewExerciseToDivision();
			}
		}
	}

	function getExerciseNameFieldYPos(): int {
		return currentSplitPage.exerciseNameFieldYPosition();
	}
} //Page
