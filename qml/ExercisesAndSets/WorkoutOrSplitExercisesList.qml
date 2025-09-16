import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"
import "../Pages"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ListView {
	id: control
	model: exercisesModel
	reuseItems: true
	snapMode: ListView.SnapOneItem
	boundsBehavior: Flickable.StopAtBounds
	flickableDirection: Flickable.VerticalFlick
	spacing: 5

	required property QtObject pageManager
	required property DBExercisesModel exercisesModel
	property TPPage parentPage
	property bool viewPositionAtBeginning: true
	property bool viewPositionAtEnd: false

	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: ScrollBar.AsNeeded
		active: ScrollBar.AsNeeded

		onPositionChanged: {
			if (exercisesModel.isWorkout) return;
			viewPositionAtBeginning = contentY <= 100;
			if (itemAt(0, contentY))
				viewPositionAtEnd = contentY + itemAt(0, contentY).height >= contentHeight - 100;
		}
	}

	delegate: ItemDelegate {
		id: delegate
		spacing: 10
		padding: 0
		implicitWidth: control.width
		implicitHeight: contentsLayout.implicitHeight * 1.1

		readonly property int exerciseNumber: index
		property int nSubExercises: exercisesModel.subExercisesCount(delegate.exerciseNumber)
		property bool restTimeEditable: !exercisesModel.autoRestTime(exerciseNumber)
		property bool setCompleted: exercisesModel.setCompleted(exerciseNumber, exercisesModel.workingSubExercise, exercisesModel.workingSet)
		property bool allSetsCompleted: exercisesModel.allSetsCompleted(exerciseNumber, exercisesModel.workingSubExercise)

		function exerciseFieldYPos(): int {
			return delegate.mapToItem(mainwindow.contentItem, txtExerciseName.x, txtExerciseName.y).y;
		}

		onClicked: workingExercise = index;

		contentItem: Rectangle {
			id: listItem
			border.color: "transparent"
			color: workingExercise === index ? userSettings.primaryColor :
							(index % 2 === 0 ? userSettings.listEntryColor1 : userSettings.listEntryColor2)
			radius: 5
		}

		Connections {
			target: pageManager
			ignoreUnknownSignals: true

			function onUpdateRestTime(exercise_number: int, rest_time: string) : void {
				if (exercise_number === index) {
					txtRestTime.text = rest_time;
				}
			}
		}

		Connections {
			target: exercisesModel

			function onWorkingExerciseChanged(exercise_number: int) : void {
				if (exercise_number === index) {
					const exercise_idx = exercisesModel.workingSubExercise;
					subExercisesTabBar.currentIndex = exercise_idx;
					onWorkingSubExerciseChanged(exercise_number, exercise_idx);
				}
			}

			function onWorkingSubExerciseChanged(exercise_number: int, exercise_idx: int) : void {
				const set_number = exercisesModel.workingSet;
				setsTabBar.currentIndex = set_number;
				delegate.changeFields(exercise_number, exercise_idx, set_number, false);
			}

			function onWorkingSetChanged(exercise_number: int, exercise_idx: int, set_number: int) : void {
				delegate.changeFields(exercise_number, exercise_idx, set_number, true);
			}

			function onExerciseNameChanged(exercise_number: int, exercise_idx: int) : void {
				if (exercise_number === index) {
					txtExerciseName.text = exercisesModel.exerciseName(exercise_number, exercise_idx);
					subExerciseButtonsRepeater.itemAt(exercise_idx).text = txtExerciseName.text;
				}
			}

			function onExerciseModified(exercise_number: int, exercise_idx: int, set_number: int, field: int): void {
				if (exercise_number === index) {
					switch (field) {
						case 7: //EXERCISES_COL_SETTYPES
							changeFields(index, exercise_idx, set_number, true);
						break;
						case 13: //EXERCISES_COL_COMPLETED
							let completed = exercisesModel.setCompleted(exercise_number, exercise_idx, set_number);
							delegate.setCompleted = completed;
							if (completed) {
								delegate.allSetsCompleted = exercisesModel.allSetsCompleted(exercise_number, exercise_idx);
								if (++set_number < exercisesModel.setsNumber(exercise_number, exercise_idx))
									exercisesModel.workingSet = set_number;
							}
							else
								delegate.allSetsCompleted = false;
						break;
					}
				}
			}

			function onSetModeChanged(exercise_number: int, exercise_idx: int, set_number: int, mode: int): void {
				if (exercise_number === index) {
					btnSetMode.text = exercisesModel.setModeLabel(exercise_number, exercise_idx, set_number);
				}
			}
		} //Connections

		function changeFields(exercise_number: int, exercise_idx: int, set_number: int, only_settype_dependent_fields: bool): void {
			if (exercise_number === index) {
				if (!only_settype_dependent_fields) {
					txtExerciseName.text = exercisesModel.exerciseName(exercise_number, exercise_idx);
					txtNotes.text = exercisesModel.setNotes(exercise_number, exercise_idx, set_number);
				}
				cboSetType.currentIndex = exercisesModel.setType(exercise_number, exercise_idx, set_number);
				txtRestTime.text = exercisesModel.setRestTime(exercise_number, exercise_idx, set_number);
				txtNSubsets.text = exercisesModel.setSubSets(exercise_number, exercise_idx, set_number);
				txtNReps.text = exercisesModel.setReps(exercise_number, exercise_idx, set_number);
				txtNWeight.text = exercisesModel.setWeight(exercise_number, exercise_idx, set_number);
				if (exercisesModel.isWorkout) {
					setCompleted = exercisesModel.setCompleted(exercise_number, exercise_idx, set_number);
					btnSetMode.enabled = pageManager.canChangeSetMode(exercise_number, exercise_idx, set_number);
				}
			}
		}

		ColumnLayout {
			id: contentsLayout
			spacing: 10

			anchors {
				fill: parent
				topMargin: 10
				leftMargin: 5
				rightMargin: 5
				bottomMargin: 10
			}

			Item {
				height: userSettings.itemDefaultHeight
				Layout.fillWidth: true

				TPRadioButtonOrCheckBox {
					id: optCurrentExercise
					text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>" + (delegate.nSubExercises > 1 ? qsTr(" - Giant sets") : "")
					checked: index === exercisesModel.workingExercise
					width: parent.width * 0.7

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}

					onClicked: exercisesModel.workingExercise = index;
				} //optCurrentExercise

				TPButton {
					id: btnNextExercise
					imageSource: "goto-next"
					width: userSettings.itemDefaultHeight
					height: width
					enabled: index === exercisesModel.workingExercise && index < exercisesModel.exerciseCount - 1 && delegate.allSetsCompleted

					anchors {
						right: btnDelExercise.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						const next_exercise_number = index + 1;
						exercisesModel.workingExercise = next_exercise_number;
						if (exercisesModel.isWorkout)
							parentPage.placeExerciseIntoView(control.itemAtIndex(index+1).y)
						else
							control.positionViewAtIndex(next_exercise_number, ListView.Contain);
					}
				} //btnNextExercise

				TPButton {
					id: btnDelExercise
					imageSource: "remove"
					width: userSettings.itemDefaultHeight
					height: width
					enabled: index === exercisesModel.workingExercise

					anchors {
						right: btnMoveExerciseUp.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: pageManager.removeExercise(index);
				} //btnDelExercise

				TPButton {
					id: btnMoveExerciseUp
					imageSource: "up.png"
					hasDropShadow: false
					width: userSettings.itemSmallHeight
					height: width
					enabled: index === exercisesModel.workingExercise ? (index >= 1) : false

					anchors {
						right: btnMoveExerciseDown.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: exercisesModel.moveExercise(index, index-1);
				} //btnMoveExerciseUp

				TPButton {
					id: btnMoveExerciseDown
					imageSource: "down.png"
					hasDropShadow: false
					width: userSettings.itemSmallHeight
					height: width
					enabled: index === exercisesModel.workingExercise ? (index < exercisesModel.exerciseCount - 1) : false

					anchors {
						right: parent.right
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: sexercisesModel.moveExercise(index, index+1);
				} //btnMoveExerciseDown
			} //Item

			Item {
				enabled: index === exercisesModel.workingExercise
				Layout.minimumWidth: listItem.width
				Layout.maximumWidth: listItem.width
				Layout.preferredHeight: userSettings.itemLargeHeight

				TPButton {
					id: btnAddSubExercise
					imageSource: "plus"
					hasDropShadow: false
					width: userSettings.itemDefaultHeight
					height: width

					onClicked: {
						nSubExercises = exercisesModel.addSubExercise(delegate.exerciseNumber) + 1;
						subExercisesTabBar.setCurrentIndex(exercisesModel.workingSubExercise);
					}

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}
				}

				StackLayout{
					id: subExercisesStack
					currentIndex: delegate.nSubExercises > 0 ? 1 : 0
					height: userSettings.itemLargeHeight

					anchors {
						left: btnAddSubExercise.right
						right: btnDelSubExercise.left
						verticalCenter: parent.verticalCenter
					}

					TPLabel {
						text: qsTr(" <<-- Add some machine or free weight exercise")
						wrapMode: Text.WordWrap
						horizontalAlignment: Text.AlignHCenter
						Layout.maximumWidth: parent.width * 0.8
						Layout.minimumWidth: parent.width * 0.8
						Layout.minimumHeight: userSettings.itemDefaultHeight * 2
					}

					TabBar {
						id: subExercisesTabBar
						Layout.fillWidth: true
						Layout.fillHeight: true
						contentWidth: width
						clip: true

						Repeater {
							id: subExerciseButtonsRepeater
							model: delegate.nSubExercises

							TPTabButton {
								id: subExercisesTabButton
								text: exercisesModel.exerciseName(delegate.exerciseNumber, index)
								checked: index === exercisesModel.workingSubExercise
								parentTab: subExercisesTabBar

								onClicked: {
									exercisesModel.workingSubExercise = index;
									if (text.startsWith(qsTr("Choose")))
										pageManager.simpleExercisesList(true);
								}
							} //subExercisesTabButton
						} //subExerciseButtonsRepeater
					} //subExercisesTabBar
				} //StackLayout

				TPButton {
					id: btnDelSubExercise
					imageSource: "minus"
					hasDropShadow: false
					width: userSettings.itemDefaultHeight
					height: width

					anchors {
						right: parent.right
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						exercisesModel.delSubExercise(exercisesModel.workingExercise, exercisesModel.workingSubExercise);
						delegate.nSubExercises--;
					}
				}
			} //Item

			ExerciseNameField {
				id: txtExerciseName
				text: exercisesModel.exerciseName(index, exercisesModel.workingSubExercise)
				showRemoveButton: false
				editable: delegate.nSubExercises > 0 && index === exercisesModel.workingExercise
				Layout.preferredWidth: parent.width
				Layout.preferredHeight: userSettings.pageHeight * 0.1

				onExerciseChanged: (new_text) => exercisesModel.setExerciseName(exercisesModel.workingExercise, exercisesModel.workingSubExercise, new_text);
				onItemClicked: exercisesModel.workingExercise = index;
				onShowExercisesListButtonClicked: pageManager.simpleExercisesList(true);
			} //txtExerciseName

			Row {
				id: trackRestTimeRow
				enabled: index === exercisesModel.workingExercise
				spacing: 0
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10

				TPRadioButtonOrCheckBox {
					id: chkTrackRestTime
					text: exercisesModel.trackRestTimeLabel
					radio: false
					multiLine: true
					checked: exercisesModel.trackRestTime(index)
					width: listItem.width * 0.5

					onClicked: {
						exercisesModel.setTrackRestTime(index, checked);
						delegate.restTimeEditable = !chkAutoRestTime.checked;
					}
				}

				TPRadioButtonOrCheckBox {
					id: chkAutoRestTime
					text: exercisesModel.autoRestTimeLabel
					radio: false
					multiLine: true
					checked: exercisesModel.autoRestTime(index)
					width: listItem.width * 0.5

					onClicked: {
						exercisesModel.setAutoRestTime(index, checked);
						delegate.restTimeEditable = !checked;
					}
				}
			}

			GroupBox {
				id: setsGroup
				padding: 0
				spacing: 0
				enabled: delegate.nSubExercises > 0 && index === exercisesModel.workingExercise
				Layout.maximumWidth: parent.width
				Layout.minimumWidth: parent.width

				property int nSets: exercisesModel.setsNumber(delegate.exerciseNumber, exercisesModel.workingSubExercise)

				background: Rectangle {
					color: "transparent"
					border.color: userSettings.fontColor
					radius: 6
				}

				ColumnLayout {
					id: setsItemsLayout
					anchors.fill: parent
					spacing: 5

					Item {
						enabled: delegate.nSubExercises > 0
						Layout.fillWidth: true
						Layout.preferredHeight: userSettings.itemLargeHeight

						TPButton {
							id: btnAddSet
							imageSource: "plus"
							hasDropShadow: false
							width: userSettings.itemDefaultHeight
							height: width

							onClicked: {
								setsGroup.nSets = exercisesModel.addSet(exercisesModel.workingExercise, exercisesModel.workingSubExercise) + 1;
								setsTabBar.setCurrentIndex(exercisesModel.workingSet);
							}

							anchors {
								left: parent.left
								verticalCenter: parent.verticalCenter
							}
						}

						StackLayout{
							id: setsStack
							currentIndex: setsGroup.nSets > 0 ? 1 : 0
							height: userSettings.itemLargeHeight

							anchors {
								left: btnAddSet.right
								right: btnDelSet.left
								verticalCenter: parent.verticalCenter
							}

							TPLabel {
								text: qsTr(" <<-- Add some sets")
								horizontalAlignment: Text.AlignHCenter
								Layout.fillWidth: true
							}

							TabBar {
								id: setsTabBar
								Layout.fillWidth: true
								Layout.fillHeight: true
								contentWidth: width
								clip: true

								Repeater {
									id: buttonsRepeater
									model: setsGroup.nSets

									TPTabButton {
										id: tabButton
										text: qsTr("Set # ") + parseInt(index + 1)
										checked: delegate.exerciseNumber === exercisesModel.workingExercise &&
											subExercisesTabBar.currentIndex === exercisesModel.workingSubExercise && index === exercisesModel.workingSet
										parentTab: setsTabBar
										width: setsGroup.width * 0.22

										onClicked: exercisesModel.workingSet = index;
									} //tabButton
								} //buttonsRepeater
							} //setsTabBar
						} //stackLayout

						TPButton {
							id: btnDelSet
							imageSource: "minus"
							hasDropShadow: false
							enabled: setsGroup.nSets > 0
							width: userSettings.itemDefaultHeight
							height: width

							anchors {
								right: parent.right
								verticalCenter: parent.verticalCenter
							}

							onClicked: {
								exercisesModel.delSet(exercisesModel.workingExercise, exercisesModel.workingSubExercise, exercisesModel.workingSet);
								setsGroup.nSets--;
							}
						}
					} //Item

					Row {
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width * 0.9
						Layout.minimumHeight: userSettings.itemDefaultHeight
						padding: 10
						enabled: setsGroup.nSets > 0 && !delegate.setCompleted

						TPLabel {
							text: exercisesModel.setTypeLabel
							width: listItem.width * 0.36
						}

						TPComboBox {
							id: cboSetType
							enabled: index === exercisesModel.workingExercise
							model: AppGlobals.setTypesModel
							currentIndex: exercisesModel.setType(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
							width: listItem.width * 0.45

							onActivated: (cboIndex) => exercisesModel.setSetType(index, exercisesModel.workingSubExercise, exercisesModel.workingSet, cboIndex);
						}
					} //RowLayout

					RowLayout {
						visible: setsGroup.nSets > 0 && cboSetType.currentIndex >= 3
						enabled: !delegate.setCompleted
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width * 0.9

						TPLabel {
							text: exercisesModel.setTotalSubsets
							Layout.preferredWidth: listItem.width * 0.63
						}

						SetInputField {
							id: txtNSubsets
							text: exercisesModel.setSubSets(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
							type: SetInputField.Type.SetType
							availableWidth: listItem.width * 0.25
							showLabel: false

							onValueChanged: (str) => exercisesModel.setSetSubSets(exercisesModel.workingExercise,
															exercisesModel.workingSubExercise, exercisesModel.workingSet, str);
							onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
						}
					} //RowLayout

					SetInputField {
						id: txtRestTime
						type: SetInputField.Type.TimeType
						text: exercisesModel.setRestTime(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
						availableWidth: listItem.width * 0.9
						editable: delegate.restTimeEditable
						enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && !delegate.setCompleted
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => exercisesModel.setSetRestTime(exercisesModel.workingExercise,
															exercisesModel.workingSubExercise, exercisesModel.workingSet, str);
					}

					SetInputField {
						id: txtNReps
						text: exercisesModel.setReps(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
						type: SetInputField.Type.RepType
						availableWidth: listItem.width * 0.9
						enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && !delegate.setCompleted
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => exercisesModel.setSetReps(exercisesModel.workingExercise,
															exercisesModel.workingSubExercise, exercisesModel.workingSet, str);
						onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
					} //txtNReps

					SetInputField {
						id: txtNWeight
						text: exercisesModel.setWeight(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
						type: SetInputField.Type.WeightType
						availableWidth: listItem.width * 0.9
						enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && !delegate.setCompleted
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => exercisesModel.setSetWeight(exercisesModel.workingExercise,
															exercisesModel.workingSubExercise, exercisesModel.workingSet, str);
					} //txtNWeight

					SetNotesField {
						id: txtNotes
						info: exercisesModel.setNotesLabel
						text: exercisesModel.setNotes(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
						enabled: cboSetType.currentIndex >= 0 && !delegate.setCompleted
						Layout.fillWidth: true

						onEditFinished: (new_text) => exercisesModel.setSetNotes(exercisesModel.workingExercise,
														exercisesModel.workingSubExercise, exercisesModel.workingSet, new_text);
					}

					Item {
						id: setModeLayout
						visible: exercisesModel ? (exercisesModel.isWorkout && cboSetType.currentIndex >= 0) : false
						width: parent.width * 0.55
						Layout.alignment: Qt.AlignCenter
						Layout.maximumWidth: width
						Layout.minimumWidth: width
						Layout.minimumHeight: userSettings.itemDefaultHeight
						Layout.maximumHeight: userSettings.itemDefaultHeight

						TPImage {
							id: imgSetCompleted
							source: "set-completed"
							width: userSettings.itemDefaultHeight
							height: width
							enabled: delegate.setCompleted

							anchors {
								left: parent.left
								verticalCenter: parent.verticalCenter
							}
						}

						TPButton {
							id: btnSetMode
							text: exercisesModel.setModeLabel(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
							width: parent.width - imgSetCompleted.width
							height: parent.height
							enabled: exercisesModel.isWorkout ? pageManager.canChangeSetMode(index, exercisesModel.workingSubExercise, exercisesModel.workingSet) : false
							onClicked: pageManager.setWorkingSetMode();

							anchors {
								left: imgSetCompleted.right
								verticalCenter: parent.verticalCenter
							}
						}
					} //setModeLayout
				} //setsItemsLayout
			} //setsGroup

			RowLayout {
				visible: exercisesModel.isWorkout
				enabled: delegate.allSetsCompleted
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: listItem.width * 0.9

				TPLabel {
					text: qsTr("Exercise completed")
					Layout.preferredWidth: listItem.width * 0.63
				}

				TPImage {
					source: "set-completed"
					width: userSettings.itemDefaultHeight
					height: width
				}
			} //RowLayout
		} //contentsLayout
	} //delegate: ItemDelegate

	function reloadModel(): void {
		control.model = 0;
		control.model = exercisesModel.count;
	}

	function appendNewExerciseToDivision(): void {
		pageManager.addExercise();
		control.currentIndex = exercisesModel.workingExercise;
		control.positionViewAtIndex(exercisesModel.workingExercise, ListView.Contain);
	}

	function exerciseNameFieldYPosition(): int {
		if (itemAtIndex(exercisesModel.workingExercise))
			return itemAtIndex(exercisesModel.workingExercise).exerciseFieldYPos();
		else
			return 0;
	}
} //ListView
