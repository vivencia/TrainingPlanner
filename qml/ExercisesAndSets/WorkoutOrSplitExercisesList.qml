import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"
import "../Pages"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPListView {
	id: control
	model: exercisesModel
	spacing: 5

	property QtObject pageManager
	property DBExercisesModel exercisesModel
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
		property int nSubExercises: exercisesModel.subExercisesCount(exerciseNumber)
		property bool restTimeEditable: exercisesModel.trackRestTime(exerciseNumber) && !exercisesModel.autoRestTime(exerciseNumber)
		property bool setCompleted: exercisesModel.setCompleted(exerciseNumber, exercisesModel.workingSubExercise, exercisesModel.workingSet)
		property bool allSetsCompleted: exercisesModel.allSetsCompleted(exerciseNumber, exercisesModel.workingSubExercise)

		function exerciseFieldYPos(): int {
			return delegate.mapToItem(mainwindow.contentItem, txtExerciseName.x, txtExerciseName.y).y;
		}

		onClicked: exercisesModel.workingExercise = index;

		contentItem: Rectangle {
			id: listItem
			border.color: "transparent"
			color: workingExercise === index ? appSettings.primaryColor :
							(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
			radius: 8
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
				setsGroup.nSets = exercisesModel.setsNumber(exercise_number, exercise_idx);
				setsTabBar.currentIndex = set_number;
				delegate.changeFields(exercise_number, exercise_idx, set_number, false);
			}

			function onWorkingSetChanged(exercise_number: int, exercise_idx: int, set_number: int) : void {
				delegate.changeFields(exercise_number, exercise_idx, set_number, true);
			}

			function onSubExerciseCountChanged(exercise_number: int): void {
				delegate.nSubExercises = exercisesModel.subExercisesCount(exercise_number);
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
						case 4: //EXERCISES_FIELD_TRACKRESTTIMES
						case 5: //EXERCISES_FIELD_AUTORESTTIMES
							delegate.restTimeEditable = exercisesModel.trackRestTime(exerciseNumber) && !exercisesModel.autoRestTime(exerciseNumber);
						break;
						case 7: //EXERCISES_FIELD_SETTYPES
							delegate.changeFields(index, exercise_idx, set_number, true);
						break;
						case 13: //EXERCISES_FIELD_COMPLETED
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

			function onSetsNumberChanged(exercise_number: int, exercise_idx: int): void {
				if (exercise_number === index)
					setsGroup.nSets = exercisesModel.setsNumber(exercise_number, exercise_idx);
			}

			function onSetTypeChanged(exercise_number: int, exercise_idx: int, set_number: int, mode: int): void {
				if (exercise_number === index) {
					txtRestTime.text = exercisesModel.setRestTime(exercise_number, exercise_idx, set_number);
					if (subSetsLoader.active && subSetsLoader.status === Loader.Ready)
						subSetsLoader.item.subSetsNumber = exercisesModel.setSubSets(exercise_number, exercise_idx, set_number);
					txtNWeight.text = exercisesModel.setWeight(exercise_number, exercise_idx, set_number);
					txtNReps.text = exercisesModel.setReps(exercise_number, exercise_idx, set_number);
				}
			}

			function onSetModeChanged(exercise_number: int, exercise_idx: int, set_number: int, mode: int): void {
				if (exercise_number === index)
					btnSetMode.text = exercisesModel.setModeLabel(exercise_number, exercise_idx, set_number);
			}
		} //Connections

		function changeFields(exercise_number: int, exercise_idx: int, set_number: int, only_settype_dependent_fields: bool): void {
			if (exercise_number === index) {
				if (!only_settype_dependent_fields)
					txtExerciseName.text = exercisesModel.exerciseName(exercise_number, exercise_idx);

				cboSetType.setCurIndex(exercisesModel.setType(exercise_number, exercise_idx, set_number));
				txtRestTime.text = exercisesModel.setRestTime(exercise_number, exercise_idx, set_number);
				if (subSetsLoader.active && subSetsLoader.status === Loader.Ready)
					subSetsLoader.item.subSetsNumber = exercisesModel.setSubSets(exercise_number, exercise_idx, set_number);
				txtNReps.text = exercisesModel.setReps(exercise_number, exercise_idx, set_number);
				txtNWeight.text = exercisesModel.setWeight(exercise_number, exercise_idx, set_number);
				txtNotes.text = exercisesModel.setNotes(exercise_number, exercise_idx, set_number);
				if (exercisesModel.isWorkout) {
					setCompleted = exercisesModel.setCompleted(exercise_number, exercise_idx, set_number);
					setModeLoader.item.buttonEnabled = pageManager.canChangeSetMode(exercise_number, exercise_idx, set_number);
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
				height: appSettings.itemDefaultHeight
				Layout.fillWidth: true

				TPRadioButtonOrCheckBox {
					id: optCurrentExercise
					text: qsTr("Exercise ") + "<b>" + (index + 1) + "</b>"
					checked: delegate.exerciseNumber === exercisesModel.workingExercise
					width: parent.width * 0.6

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}

					onClicked: exercisesModel.workingExercise = index;
				} //optCurrentExercise

				TPButton {
					id: btnPrevExercise
					imageSource: "goto-prev"
					width: appSettings.itemDefaultHeight
					height: width
					enabled: delegate.exerciseNumber === exercisesModel.workingExercise && delegate.exerciseNumber > 0

					anchors {
						right: btnNextExercise.left
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						const prev_exercise_number = index - 1;
						exercisesModel.workingExercise = prev_exercise_number;
						control.positionViewAtIndex(prev_exercise_number, ListView.Contain);
					}
				} //btnPrevExercise

				TPButton {
					id: btnNextExercise
					imageSource: "goto-next"
					width: appSettings.itemDefaultHeight
					height: width
					enabled: delegate.exerciseNumber === exercisesModel.workingExercise &&
															delegate.exerciseNumber < exercisesModel.exerciseCount - 1

					anchors {
						right: btnDelExercise.left
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						const next_exercise_number = index + 1;
						exercisesModel.workingExercise = next_exercise_number;
						control.positionViewAtIndex(next_exercise_number, ListView.Contain);
					}
				} //btnNextExercise

				TPButton {
					id: btnDelExercise
					imageSource: "remove"
					width: appSettings.itemDefaultHeight
					height: width
					enabled: delegate.exerciseNumber === exercisesModel.workingExercise

					anchors {
						right: btnMoveExerciseUp.left
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: showDeleteDialog();
				} //btnDelExercise

				TPButton {
					id: btnMoveExerciseUp
					imageSource: "up.png"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight
					height: width
					enabled: delegate.exerciseNumber === exercisesModel.workingExercise ? (index >= 1) : false

					anchors {
						right: btnMoveExerciseDown.left
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: exercisesModel.moveExercise(index, index-1);
				} //btnMoveExerciseUp

				TPButton {
					id: btnMoveExerciseDown
					imageSource: "down.png"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight
					height: width
					enabled: delegate.exerciseNumber === exercisesModel.workingExercise ? (index < exercisesModel.exerciseCount - 1) : false

					anchors {
						right: parent.right
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: sexercisesModel.moveExercise(index, index+1);
				} //btnMoveExerciseDown
			} //Item

			Item {
				enabled: delegate.exerciseNumber === exercisesModel.workingExercise
				Layout.minimumWidth: listItem.width
				Layout.maximumWidth: listItem.width
				Layout.preferredHeight: appSettings.itemLargeHeight

				TPButton {
					id: btnAddSubExercise
					imageSource: "plus"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight
					height: width

					onClicked: {
						exercisesModel.addSubExercise(delegate.exerciseNumber);
						subExercisesTabBar.setCurrentIndex(exercisesModel.workingSubExercise);
					}

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}
				}

				TabBar {
					id: subExercisesTabBar
					height: appSettings.itemLargeHeight
					contentWidth: width
					clip: true

					anchors {
						left: btnAddSubExercise.right
						right: btnDelSubExercise.left
						verticalCenter: parent.verticalCenter
					}

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
									itemManager.showSimpleExercisesList(pageManager.qmlPage(), exercisesModel.muscularGroup);
								else
									ToolTip.show(text, 2000);
							}
						} //subExercisesTabButton
					} //subExerciseButtonsRepeater
				} //subExercisesTabBar

				TPButton {
					id: btnDelSubExercise
					imageSource: "minus"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						right: parent.right
						rightMargin: 5
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						exercisesModel.delSubExercise(exercisesModel.workingExercise, exercisesModel.workingSubExercise);
					}
				}
			} //Item

			ExerciseNameField {
				id: txtExerciseName
				text: exercisesModel.exerciseName(index, exercisesModel.workingSubExercise)
				showRemoveButton: false
				editable: delegate.nSubExercises > 0 && delegate.exerciseNumber === exercisesModel.workingExercise
				Layout.preferredWidth: parent.width
				Layout.preferredHeight: appSettings.pageHeight * 0.1

				onExerciseChanged: (new_text) => exercisesModel.setExerciseName(exercisesModel.workingExercise, exercisesModel.workingSubExercise, new_text);
				onItemClicked: exercisesModel.workingExercise = index;
				onShowExercisesListButtonClicked: itemManager.showSimpleExercisesList(pageManager.qmlPage(), exercisesModel.muscularGroup);
			} //txtExerciseName

			Row {
				id: trackRestTimeRow
				enabled: delegate.exerciseNumber === exercisesModel.workingExercise
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
						if (!checked) {
							chkAutoRestTime.checked = false;
							exercisesModel.setAutoRestTime(index, false);
						}
					}
				}

				TPRadioButtonOrCheckBox {
					id: chkAutoRestTime
					text: exercisesModel.autoRestTimeLabel
					radio: false
					multiLine: true
					checked: exercisesModel.autoRestTime(index)
					width: listItem.width * 0.5

					onClicked: exercisesModel.setAutoRestTime(index, checked);
				}
			}

			GroupBox {
				id: setsGroup
				padding: 0
				spacing: 0
				enabled: delegate.nSubExercises > 0 && delegate.exerciseNumber === exercisesModel.workingExercise
				Layout.maximumWidth: parent.width
				Layout.minimumWidth: parent.width

				property int nSets: exercisesModel.setsNumber(delegate.exerciseNumber, exercisesModel.workingSubExercise)

				onNSetsChanged: {
					if (nSets === 0 && exercisesModel.workingSubExercise !== 0)
						chkSyncGiantSets.checked = false;
				}

				background: Rectangle {
					color: "transparent"
					border.color: appSettings.fontColor
					radius: 6
				}

				ColumnLayout {
					id: setsItemsLayout
					anchors.fill: parent
					spacing: 5

					Item {
						enabled: delegate.nSubExercises > 0
						Layout.fillWidth: true
						Layout.preferredHeight: appSettings.itemLargeHeight

						TPButton {
							id: btnAddSet
							imageSource: "plus"
							hasDropShadow: false
							width: appSettings.itemDefaultHeight
							height: width

							onClicked: {
								exercisesModel.addSet(exercisesModel.workingExercise, exercisesModel.workingSubExercise) + 1;
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
							height: appSettings.itemLargeHeight

							anchors {
								left: btnAddSet.right
								right: btnDelSet.left
								verticalCenter: parent.verticalCenter
							}

							Item {
								height: parent.height
								Layout.maximumWidth: setsStack.width - 10
								Layout.minimumWidth: setsStack.width - 10

								TPLabel {
									text: qsTr(" <<-- Add some sets")
									horizontalAlignment: Text.AlignHCenter
									visible: subExercisesTabBar.currentIndex === 0 || exercisesModel.syncGiantSets(
																			exercisesModel.workingExercise, exercisesModel.workingSubExercise)
									width: parent.width
								}
								TPRadioButtonOrCheckBox {
									id: chkSyncGiantSets
									text: qsTr("Follow first exercise sets")
									radio: false
									checked: exercisesModel.syncGiantSets(exercisesModel.workingExercise, exercisesModel.workingSubExercise)
									visible: subExercisesTabBar.currentIndex !== 0 && setsGroup.nSets === 0
									width: parent.width

									onClicked: exercisesModel.setSyncGiantSets(exercisesModel.workingExercise,
																									exercisesModel.workingSubExercise, checked);
								}
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
										text: qsTr("Set ") + parseInt(index + 1)
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
							width: appSettings.itemDefaultHeight
							height: width

							anchors {
								right: parent.right
								top: setsStack.top
								topMargin: -appSettings.itemDefaultHeight/4 - 5
							}

							onClicked: exercisesModel.delSet(exercisesModel.workingExercise, exercisesModel.workingSubExercise, exercisesModel.workingSet);
						}

						TPButton {
							id: btnDelAllSets
							imageSource: "remove"
							hasDropShadow: false
							enabled: setsGroup.nSets > 0
							width: appSettings.itemDefaultHeight
							height: width

							anchors {
								right: parent.right
								bottom: setsStack.bottom
								bottomMargin: -appSettings.itemDefaultHeight/4
							}

							onClicked: exercisesModel.removeAllSets(exercisesModel.workingExercise, exercisesModel.workingSubExercise);
						}
					} //Item

					RowLayout {
						Layout.minimumWidth: listItem.width * 0.9
						Layout.maximumWidth: listItem.width * 0.9
						Layout.alignment: Qt.AlignCenter

						TPLabel {
							text: exercisesModel.setTypeLabel
							Layout.maximumWidth: listItem.width * 0.3
						}

						TPComboBox {
							id: cboSetType
							enabled: setsGroup.nSets > 0 && !delegate.setCompleted
							model: AppGlobals.setTypesModel
							currentIndex: exercisesModel.setType(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
							Layout.fillWidth: true

							onActivated: (cboIndex) => exercisesModel.setSetType(index, exercisesModel.workingSubExercise, exercisesModel.workingSet, cboIndex);
						}
					} //RowLayout

					Loader {
						id: subSetsLoader
						active: setsGroup.nSets > 0 && cboSetType.currentIndex >= 3
						asynchronous: true
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width * 0.9

						sourceComponent: RowLayout {
							enabled: !delegate.setCompleted

							property string subSetsNumber: exercisesModel.setSubSets(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)

							TPLabel {
								text: exercisesModel.setTotalSubsets
								Layout.preferredWidth: listItem.width * 0.63
							}

							SetInputField {
								text: parent.subSetsNumber
								type: SetInputField.Type.SetType
								availableWidth: listItem.width * 0.25
								showLabel: false

								onValueChanged: (str) => exercisesModel.setSetSubSets(exercisesModel.workingExercise,
															exercisesModel.workingSubExercise, exercisesModel.workingSet, str);
								onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
							}
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
						Layout.minimumWidth: txtNWeight.width
						Layout.maximumWidth: txtNWeight.width
						Layout.alignment: Qt.AlignCenter

						onEditFinished: (new_text) => exercisesModel.setSetNotes(exercisesModel.workingExercise,
														exercisesModel.workingSubExercise, exercisesModel.workingSet, new_text);
					}

					Loader {
						id: setModeLoader
						active: exercisesModel.isWorkout ? pageManager.todaysWorkout && cboSetType.currentIndex >= 0 : false
						asynchronous: true
						width: parent.width * 0.55
						Layout.alignment: Qt.AlignCenter
						Layout.maximumWidth: width
						Layout.minimumWidth: width
						Layout.minimumHeight: appSettings.itemDefaultHeight
						Layout.maximumHeight: appSettings.itemDefaultHeight

						sourceComponent: Item {
							property bool buttonEnabled: pageManager.canChangeSetMode(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)

							TPImage {
								id: imgSetCompleted
								source: "set-completed"
								width: appSettings.itemDefaultHeight
								height: width
								enabled: delegate.setCompleted

								anchors {
									left: parent.left
									verticalCenter: parent.verticalCenter
								}
							}

							TPButton {
								text: exercisesModel.setModeLabel(index, exercisesModel.workingSubExercise, exercisesModel.workingSet)
								width: parent.width - imgSetCompleted.width
								height: parent.height
								enabled: parent.buttonEnabled
								onClicked: pageManager.setWorkingSetMode();

								anchors {
									left: imgSetCompleted.right
									verticalCenter: parent.verticalCenter
								}
							}
						}
					} //Loader setMode
				} //setsItemsLayout
			} //setsGroup

			Loader {
				active: exercisesModel.isWorkout ? pageManager.todaysWorkout : false
				asynchronous: true
				Layout.alignment: Qt.AlignCenter
				Layout.preferredWidth: listItem.width * 0.9

				sourceComponent: RowLayout {
					enabled: delegate.allSetsCompleted

					TPLabel {
						text: qsTr("Exercise completed")
						Layout.preferredWidth: listItem.width * 0.63
					}

					TPImage {
						source: "set-completed"
						width: appSettings.itemDefaultHeight
						height: width
					}
				} //RowLayout
			} //Loader
		} //contentsLayout
	} //delegate: ItemDelegate

	function showDeleteDialog(exercise_number: int): void {
		if (appSettings.alwaysAskConfirmation)
			delExerciseLoader.active = true;
		else
			exercisesModel.delExercise(exercisesModel.workingExercise);
	}

	Loader {
		id: delExerciseLoader
		active: false
		asynchronous: true

		sourceComponent: TPBalloonTip {
			title: qsTr("Remove Exercise?")
			message: exerciseName + qsTr("\nThis action cannot be undone.")
			imageSource: "remove"
			keepAbove: true
			button1Text: qsTr("Yes")
			button2Text: qsTr("No")
			onButton1Clicked: exercisesModel.delExercise(exercisesModel.workingExercise);
			parentPage: control.pageManager

			onClosed: delExerciseLoader.active = false;
		}

		onLoaded: {
			item.exerciseName = exercisesModel.exerciseName(exercisesModel.workingExercise, exercisesModel.allExerciseNames);
			item.show(-1);
		}
	}

	function showClearExercisesMessage(): void {
		if (appSettings.alwaysAskConfirmation)
			clearExercises.active = true;
		else
			exercisesModel.clearExercises();
	}

	Loader {
		id: clearExercises
		active: false
		asynchronous: true

		sourceComponent: TPBalloonTip {
			title: qsTr("Remove all exercises?")
			message: qsTr("This action cannot be undone.")
			imageSource: "remove"
			keepAbove: true
			button1Text: qsTr("Yes")
			button2Text: qsTr("No")
			onButton1Clicked: exercisesModel.clearExercises();
			parentPage: control.pageManager

			onClosed: clearExercises.active = false;
		}

		onLoaded: item.show(-1);
	}

	function appendNewExerciseToDivision(): void {
		exercisesModel.setWorkingExercise = exercisesModel.addExercise();
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
