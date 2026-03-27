pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ItemDelegate {
	id: delegate
	spacing: 10
	padding: 0
	implicitWidth: parent.width
	implicitHeight: contentsLayout.implicitHeight * 1.1

//public:
	required property DBExercisesModel exercisesModel
	required property int exerciseNumber
	required property int setsNumber
	required property int workingExercise
	required property int workingSubExercise
	required property int workingSet
	required property bool trackRestTime
	required property bool autoRestTime

	property WorkoutManager workoutPageManager: null
	property SplitManager splitPageManager: null

	signal gotoExercise(int exercise_number);
	signal removeExerciseRequested();

	function exerciseFieldYPos(): int {
		return delegate.mapToItem(ItemManager.AppPagesManager.homePage(), txtExerciseName.x, txtExerciseName.y).y;
	}

//private:
	property int nSubExercises: exercisesModel.subExercisesCount(exerciseNumber)
	property bool restTimeEditable: trackRestTime && !autoRestTime
	property bool setCompleted: exercisesModel.setCompleted(exerciseNumber, workingSubExercise, workingSet)
	property bool allSetsCompleted: exercisesModel.allSetsCompleted(exerciseNumber, workingSubExercise)

	onClicked: exercisesModel.workingExercise = exerciseNumber;

	contentItem: Rectangle {
		id: listItem
		border.color: "transparent"
		color: delegate.workingExercise === delegate.exerciseNumber ? AppSettings.primaryColor :
								(delegate.exerciseNumber % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2)
		radius: 8
	}

	Connections {
		target: delegate.workoutPageManager ? delegate.workoutPageManager : null

		function onUpdateRestTime(exercise_number: int, rest_time: string) : void {
			if (exercise_number === delegate.exerciseNumber) {
				txtRestTime.text = rest_time;
			}
		}
	}

	Connections {
		target: delegate.exercisesModel

		function onWorkingExerciseChanged(exercise_number: int) : void {
			if (exercise_number === delegate.exerciseNumber) {
				const exercise_idx = delegate.exercisesModel.workingSubExercise;
				subExercisesTabBar.currentIndex = exercise_idx;
				onWorkingSubExerciseChanged(exercise_number, exercise_idx);
			}
		}

		function onWorkingSubExerciseChanged(exercise_number: int, exercise_idx: int) : void {
			const set_number = delegate.exercisesModel.workingSet;
			setsGroup.nSets = delegate.exercisesModel.setsNumber(exercise_number, exercise_idx);
			setsTabBar.currentIndex = set_number;
			delegate.changeFields(exercise_number, exercise_idx, set_number, false);
		}

		function onWorkingSetChanged(exercise_number: int, exercise_idx: int, set_number: int) : void {
			delegate.changeFields(exercise_number, exercise_idx, set_number, true);
		}

		function onSubExerciseCountChanged(exercise_number: int): void {
			delegate.nSubExercises = delegate.exercisesModel.subExercisesCount(exercise_number);
		}

		function onExerciseNameChanged(exercise_number: int, exercise_idx: int) : void {
			if (exercise_number === delegate.exerciseNumber) {
				txtExerciseName.text = delegate.exercisesModel.exerciseName(exercise_number, exercise_idx);
				subExerciseButtonsRepeater.itemAt(exercise_idx).text = txtExerciseName.text;
			}
		}

		function onExerciseModified(exercise_number: int, exercise_idx: int, set_number: int, field: int): void {
			if (exercise_number === delegate.exerciseNumber) {
				switch (field) {
				case DBExercisesModel.EXERCISES_FIELD_TRACKRESTTIMES:
				case DBExercisesModel.EXERCISES_FIELD_AUTORESTTIMES:
					delegate.restTimeEditable = delegate.exercisesModel.trackRestTime(delegate.exerciseNumber) &&
															!delegate.exercisesModel.autoRestTime(delegate.exerciseNumber);
					break;
				case DBExercisesModel.EXERCISES_FIELD_SETTYPES:
					delegate.changeFields(exercise_number, exercise_idx, set_number, true);
					break;
				case DBExercisesModel.EXERCISES_FIELD_COMPLETED:
					let completed = delegate.exercisesModel.setCompleted(exercise_number, exercise_idx, set_number);
					delegate.setCompleted = completed;
					if (completed) {
						delegate.allSetsCompleted = delegate.exercisesModel.allSetsCompleted(exercise_number, exercise_idx);
						if (++set_number < delegate.exercisesModel.setsNumber(exercise_number, exercise_idx))
							delegate.exercisesModel.workingSet = set_number;
					}
					else
						delegate.allSetsCompleted = false;
					break;
				default: return;
				}
			}
		}

		function onSetsNumberChanged(exercise_number: int, exercise_idx: int): void {
			if (exercise_number === delegate.exerciseNumber)
				setsGroup.nSets = delegate.exercisesModel.setsNumber(exercise_number, exercise_idx);
		}

		function onSetTypeChanged(exercise_number: int, exercise_idx: int, set_number: int, mode: int): void {
			if (exercise_number === delegate.exerciseNumber) {
				txtRestTime.text = delegate.exercisesModel.setRestTime(exercise_number, exercise_idx, set_number);
				if (subSetsLoader.active && subSetsLoader.status === Loader.Ready)
					subSetsLoader.item.subSetsNumber = delegate.exercisesModel.setSubSets(
																					exercise_number, exercise_idx, set_number);
				txtNWeight.text = delegate.exercisesModel.setWeight(exercise_number, exercise_idx, set_number);
				txtNReps.text = delegate.exercisesModel.setReps(exercise_number, exercise_idx, set_number);
			}
		}

		function onSetModeChanged(exercise_number: int, exercise_idx: int, set_number: int, mode: int): void {
			if (exercise_number === delegate.exerciseNumber)
				setModeLoader.setModeButton.text = delegate.exercisesModel.setModeLabel(exercise_number, exercise_idx, set_number);
		}
	} //Connections

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
			Layout.preferredHeight: AppSettings.itemDefaultHeight
			Layout.fillWidth: true

			TPRadioButtonOrCheckBox {
				id: optCurrentExercise
				text: qsTr("Exercise ") + "<b>" + (delegate.exerciseNumber + 1) + "</b>"
				checked: delegate.exerciseNumber === delegate.exercisesModel.workingExercise
				width: parent.width * 0.6

				anchors {
					left: parent.left
					verticalCenter: parent.verticalCenter
				}

				onClicked: delegate.exercisesModel.workingExercise = delegate.exerciseNumber;
			} //optCurrentExercise

			TPButton {
				id: btnPrevExercise
				imageSource: "goto-prev"
				width: AppSettings.itemDefaultHeight
				height: width
				enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise && delegate.exerciseNumber > 0

				anchors {
					right: btnNextExercise.left
					rightMargin: 5
					verticalCenter: parent.verticalCenter
				}

				onClicked: {
					const prev_exercise_number = delegate.exerciseNumber - 1;
					delegate.exercisesModel.workingExercise = prev_exercise_number;
					delegate.gotoExercise(prev_exercise_number);
				}
			} //btnPrevExercise

			TPButton {
				id: btnNextExercise
				imageSource: "goto-next"
				width: AppSettings.itemDefaultHeight
				height: width
				enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise &&
														delegate.exerciseNumber < delegate.exercisesModel.exerciseCount - 1

				anchors {
					right: btnDelExercise.left
					rightMargin: 5
					verticalCenter: parent.verticalCenter
				}

				onClicked: {
					const next_exercise_number = delegate.exerciseNumber + 1;
					delegate.exercisesModel.workingExercise = next_exercise_number;
					delegate.gotoExercise(next_exercise_number);
				}
			} //btnNextExercise

			TPButton {
				id: btnDelExercise
				imageSource: "remove"
				width: AppSettings.itemDefaultHeight
				height: width
				enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise

				anchors {
					right: btnMoveExerciseUp.left
					rightMargin: 5
					verticalCenter: parent.verticalCenter
				}

				onClicked: delegate.removeExerciseRequested();
			} //btnDelExercise

			TPButton {
				id: btnMoveExerciseUp
				imageSource: "up.png"
				hasDropShadow: false
				width: AppSettings.itemDefaultHeight
				height: width
				enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise ? (delegate.exerciseNumber >= 1) : false

				anchors {
					right: btnMoveExerciseDown.left
					rightMargin: 5
					verticalCenter: parent.verticalCenter
				}

				onClicked: delegate.exercisesModel.moveExercise(delegate.exerciseNumber, delegate.exerciseNumber - 1);
			} //btnMoveExerciseUp

			TPButton {
				id: btnMoveExerciseDown
				imageSource: "down.png"
				hasDropShadow: false
				width: AppSettings.itemDefaultHeight
				height: width
				enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise ? (
															delegate.exerciseNumber < delegate.exercisesModel.exerciseCount - 1) : false

				anchors {
					right: parent.right
					rightMargin: 10
					verticalCenter: parent.verticalCenter
				}

				onClicked: delegate.exercisesModel.moveExercise(delegate.exerciseNumber, delegate.exerciseNumber + 1);
			} //btnMoveExerciseDown
		} //Item

		Item {
			enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise
			Layout.minimumWidth: listItem.width
			Layout.maximumWidth: listItem.width
			Layout.preferredHeight: AppSettings.itemLargeHeight

			TPButton {
				id: btnAddSubExercise
				imageSource: "plus"
				hasDropShadow: false
				width: AppSettings.itemDefaultHeight
				height: width

				onClicked: {
					delegate.exercisesModel.addSubExercise(delegate.exerciseNumber);
					subExercisesTabBar.setCurrentIndex(delegate.exercisesModel.workingSubExercise);
				}

				anchors {
					left: parent.left
					verticalCenter: parent.verticalCenter
				}
			}

			TabBar {
				id: subExercisesTabBar
				height: AppSettings.itemLargeHeight
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

					delegate: TPTabButton {
						id: subExercisesTabButton
						text: delegate.exercisesModel.exerciseName(delegate.exerciseNumber, index)
						checked: index === delegate.exercisesModel.workingSubExercise
						parentTab: subExercisesTabBar

						required property int index

						onClicked: {
							delegate.exercisesModel.workingSubExercise = index;
							if (text.startsWith(qsTr("Choose")))
								ItemManager.showSimpleExercisesList(delegate.workoutPageManager ? delegate.workoutPageManager.qmlPage() :
																	delegate.splitPageManager.qmlPage(), delegate.exercisesModel.muscularGroup);
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
				width: AppSettings.itemDefaultHeight
				height: width

				anchors {
					right: parent.right
					rightMargin: 5
					verticalCenter: parent.verticalCenter
				}

				onClicked: delegate.exercisesModel.delSubExercise(delegate.exercisesModel.workingExercise,
																					delegate.exercisesModel.workingSubExercise);
			}
		} //Item

		ExerciseNameField {
			id: txtExerciseName
			text: delegate.exercisesModel.exerciseName(delegate.exerciseNumber, delegate.exercisesModel.workingSubExercise)
			showRemoveButton: false
			editable: delegate.nSubExercises > 0 && delegate.exerciseNumber === delegate.exercisesModel.workingExercise
			Layout.preferredWidth: parent.width
			Layout.preferredHeight: AppSettings.pageHeight * 0.1

			onExerciseChanged: (new_text) => delegate.exercisesModel.setExerciseName(delegate.exercisesModel.workingExercise, delegate.exercisesModel.workingSubExercise, new_text);
			onItemClicked: delegate.exercisesModel.workingExercise = delegate.exerciseNumber;
			onShowExercisesListButtonClicked: ItemManager.showSimpleExercisesList(delegate.workoutPageManager ?
						delegate.workoutPageManager.qmlPage() : delegate.splitPageManager.qmlPage(), delegate.exercisesModel.muscularGroup);
		} //txtExerciseName

		Row {
			id: trackRestTimeRow
			enabled: delegate.exerciseNumber === delegate.exercisesModel.workingExercise
			spacing: 0
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 10

			TPRadioButtonOrCheckBox {
				id: chkTrackRestTime
				text: delegate.exercisesModel.trackRestTimeLabel
				radio: false
				multiLine: true
				checked: delegate.exercisesModel.trackRestTime(delegate.exerciseNumber)
				width: listItem.width * 0.5

				onClicked: {
					delegate.exercisesModel.setTrackRestTime(delegate.exerciseNumber, checked);
					if (!checked) {
						chkAutoRestTime.checked = false;
						delegate.exercisesModel.setAutoRestTime(delegate.exerciseNumber, false);
					}
				}
			}

			TPRadioButtonOrCheckBox {
				id: chkAutoRestTime
				text: delegate.exercisesModel.autoRestTimeLabel
				radio: false
				multiLine: true
				checked: delegate.exercisesModel.autoRestTime(delegate.exerciseNumber)
				width: listItem.width * 0.5

				onClicked: delegate.exercisesModel.setAutoRestTime(delegate.exerciseNumber, checked);
			}
		}

		GroupBox {
			id: setsGroup
			padding: 0
			spacing: 0
			enabled: delegate.nSubExercises > 0 && delegate.exerciseNumber === delegate.exercisesModel.workingExercise
			Layout.maximumWidth: parent.width
			Layout.minimumWidth: parent.width

			property int nSets: delegate.exercisesModel.setsNumber(delegate.exerciseNumber,
																					delegate.exercisesModel.workingSubExercise)

			onNSetsChanged: {
				if (nSets === 0 && delegate.exercisesModel.workingSubExercise !== 0)
					chkSyncGiantSets.checked = false;
			}

			background: Rectangle {
				color: "transparent"
				border.color: AppSettings.fontColor
				radius: 6
			}

			ColumnLayout {
				id: setsItemsLayout
				anchors.fill: parent
				spacing: 5

				Item {
					enabled: delegate.nSubExercises > 0
					Layout.fillWidth: true
					Layout.preferredHeight: AppSettings.itemLargeHeight

					TPButton {
						id: btnAddSet
						imageSource: "plus"
						hasDropShadow: false
						width: AppSettings.itemDefaultHeight
						height: width

						onClicked: {
							delegate.exercisesModel.addSet(delegate.exercisesModel.workingExercise,
																					delegate.exercisesModel.workingSubExercise);
							setsTabBar.setCurrentIndex(delegate.exercisesModel.workingSet);
						}

						anchors {
							left: parent.left
							verticalCenter: parent.verticalCenter
						}
					}

					StackLayout{
						id: setsStack
						currentIndex: setsGroup.nSets > 0 ? 1 : 0
						height: AppSettings.itemLargeHeight

						anchors {
							left: btnAddSet.right
							right: btnDelSet.left
							verticalCenter: parent.verticalCenter
						}

						Item {
							Layout.preferredHeight: parent.height
							Layout.maximumWidth: setsStack.width - 10
							Layout.minimumWidth: setsStack.width - 10

							TPLabel {
								text: qsTr(" <<-- Add some sets")
								horizontalAlignment: Text.AlignHCenter
								visible: subExercisesTabBar.currentIndex === 0 || delegate.exercisesModel.syncGiantSets(
											delegate.exercisesModel.workingExercise, delegate.exercisesModel.workingSubExercise)
								width: parent.width
							}
							TPRadioButtonOrCheckBox {
								id: chkSyncGiantSets
								text: qsTr("Follow first exercise sets")
								radio: false
								checked: delegate.exercisesModel.syncGiantSets(delegate.exercisesModel.workingExercise,
																					delegate.exercisesModel.workingSubExercise)
								visible: subExercisesTabBar.currentIndex !== 0 && setsGroup.nSets === 0
								width: parent.width

								onClicked: delegate.exercisesModel.setSyncGiantSets(delegate.exercisesModel.workingExercise,
																		delegate.exercisesModel.workingSubExercise, checked);
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

								delegate: TPTabButton {
									id: tabButton
									text: qsTr("Set ") + parseInt(index + 1)
									checked: delegate.exerciseNumber === delegate.exercisesModel.workingExercise &&
										subExercisesTabBar.currentIndex === delegate.exercisesModel.workingSubExercise &&
																				index === delegate.exercisesModel.workingSet
									parentTab: setsTabBar
									width: setsGroup.width * 0.22

									required property int index

									onClicked: delegate.exercisesModel.workingSet = index;
								} //tabButton
							} //buttonsRepeater
						} //setsTabBar
					} //stackLayout

					TPButton {
						id: btnDelSet
						imageSource: "minus"
						hasDropShadow: false
						enabled: setsGroup.nSets > 0
						width: AppSettings.itemDefaultHeight
						height: width

						anchors {
							right: parent.right
							top: setsStack.top
							topMargin: -AppSettings.itemDefaultHeight/4 - 5
						}

						onClicked: delegate.exercisesModel.delSet(delegate.exercisesModel.workingExercise,
												delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet);
					}

					TPButton {
						id: btnDelAllSets
						imageSource: "remove"
						hasDropShadow: false
						enabled: setsGroup.nSets > 0
						width: AppSettings.itemDefaultHeight
						height: width

						anchors {
							right: parent.right
							bottom: setsStack.bottom
							bottomMargin: -AppSettings.itemDefaultHeight/4
						}

						onClicked: delegate.exercisesModel.removeAllSets(delegate.exercisesModel.workingExercise,
																					delegate.exercisesModel.workingSubExercise);
					}
				} //Item

				RowLayout {
					Layout.minimumWidth: listItem.width * 0.9
					Layout.maximumWidth: listItem.width * 0.9
					Layout.alignment: Qt.AlignCenter

					TPLabel {
						text: delegate.exercisesModel.setTypeLabel
						Layout.maximumWidth: listItem.width * 0.3
					}

					TPComboBox {
						id: cboSetType
						enabled: setsGroup.nSets > 0 && !delegate.setCompleted
						model: AppGlobals.setTypesModel
						currentIndex: delegate.exercisesModel.setType(delegate.exerciseNumber, delegate.exercisesModel.workingSubExercise,
																											delegate.exercisesModel.workingSet)
						Layout.fillWidth: true

						onActivated: (cboIndex) => delegate.exercisesModel.setSetType(delegate.exerciseNumber,
									delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet, cboIndex);
					}
				} //RowLayout

				Loader {
					id: subSetsLoader
					active: setsGroup.nSets > 0 && cboSetType.currentIndex >= 3
					asynchronous: true
					Layout.alignment: Qt.AlignCenter
					Layout.preferredWidth: listItem.width * 0.9

					property string subSetsNumber: delegate.exercisesModel.setSubSets(delegate.exerciseNumber,
												delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet)

					sourceComponent: RowLayout {
						enabled: !delegate.setCompleted

						TPLabel {
							text: delegate.exercisesModel.setTotalSubsets
							Layout.preferredWidth: listItem.width * 0.63
						}

						SetInputField {
							text: subSetsLoader.subSetsNumber
							type: SetInputField.Type.SetType
							availableWidth: listItem.width * 0.25
							showLabel: false

							onValueChanged: (str) => delegate.exercisesModel.setSetSubSets(delegate.exercisesModel.workingExercise,
											delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet, str);
							onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
						}
					}
				} //RowLayout

				SetInputField {
					id: txtRestTime
					type: SetInputField.Type.TimeType
					text: delegate.exercisesModel.setRestTime(delegate.exerciseNumber, delegate.exercisesModel.workingSubExercise,
																								delegate.exercisesModel.workingSet)
					availableWidth: listItem.width * 0.9
					editable: delegate.restTimeEditable
					enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && !delegate.setCompleted
					Layout.alignment: Qt.AlignCenter

					onValueChanged: (str) => delegate.exercisesModel.setSetRestTime(delegate.exercisesModel.workingExercise,
											delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet, str);
				}

				SetInputField {
					id: txtNReps
					text: delegate.exercisesModel.setReps(delegate.exerciseNumber, delegate.exercisesModel.workingSubExercise,
																								delegate.exercisesModel.workingSet)
					type: SetInputField.Type.RepType
					availableWidth: listItem.width * 0.9
					enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && !delegate.setCompleted
					Layout.alignment: Qt.AlignCenter

					onValueChanged: (str) => delegate.exercisesModel.setSetReps(delegate.exercisesModel.workingExercise,
											delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet, str);
					onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
				} //txtNReps

				SetInputField {
					id: txtNWeight
					text: delegate.exercisesModel.setWeight(delegate.exerciseNumber, delegate.exercisesModel.workingSubExercise,
																								delegate.exercisesModel.workingSet)
					type: SetInputField.Type.WeightType
					availableWidth: listItem.width * 0.9
					enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && !delegate.setCompleted
					Layout.alignment: Qt.AlignCenter

					onValueChanged: (str) => delegate.exercisesModel.setSetWeight(delegate.exercisesModel.workingExercise,
											delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet, str);
				} //txtNWeight

				SetNotesField {
					id: txtNotes
					info: delegate.exercisesModel.setNotesLabel
					text: delegate.exercisesModel.setNotes(delegate.exerciseNumber, delegate.exercisesModel.workingSubExercise,
																							delegate.exercisesModel.workingSet)
					enabled: cboSetType.currentIndex >= 0 && !delegate.setCompleted
					Layout.minimumWidth: txtNWeight.width
					Layout.maximumWidth: txtNWeight.width
					Layout.alignment: Qt.AlignCenter

					onEditFinished: (new_text) => delegate.exercisesModel.setSetNotes(delegate.exercisesModel.workingExercise,
									delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet, new_text);
				}

				Loader {
					id: setModeLoader
					active: delegate.exercisesModel.isWorkout ? delegate.workoutPageManager.todaysWorkout &&
																							cboSetType.currentIndex >= 0 : false
					asynchronous: true
					Layout.alignment: Qt.AlignCenter
					Layout.maximumWidth: parent.width * 0.55
					Layout.minimumWidth: parent.width * 0.55
					Layout.minimumHeight: AppSettings.itemDefaultHeight
					Layout.maximumHeight: AppSettings.itemDefaultHeight

					property TPButton setModeButton

					sourceComponent: Item {
						TPImage {
							id: imgSetCompleted
							source: "set-completed"
							width: AppSettings.itemDefaultHeight
							height: width
							enabled: delegate.setCompleted

							anchors {
								left: parent.left
								verticalCenter: parent.verticalCenter
							}
						}

						TPButton {
							text: delegate.exercisesModel.setModeLabel(delegate.exerciseNumber,
												delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet)
							width: parent.width - imgSetCompleted.width
							height: parent.height
							enabled: delegate.workoutPageManager.canChangeSetMode(delegate.exerciseNumber,
												delegate.exercisesModel.workingSubExercise, delegate.exercisesModel.workingSet)
							onClicked: delegate.workoutPageManager.setWorkingSetMode();
							Component.onCompleted: setModeLoader.setModeButton = this;

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
			active: delegate.exercisesModel.isWorkout ? delegate.workoutPageManager.todaysWorkout : false
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
					Layout.preferredWidth: AppSettings.itemDefaultHeight
					Layout.preferredHeight: AppSettings.itemDefaultHeight
				}
			} //RowLayout
		} //Loader
	} //contentsLayout

	function changeFields(exercise_number: int, exercise_idx: int, set_number: int, only_settype_dependent_fields: bool): void {
		if (exercise_number === exerciseNumber) {
			if (!only_settype_dependent_fields)
				txtExerciseName.text = exercisesModel.exerciseName(exercise_number, exercise_idx);

			cboSetType.setCurIndex(exercisesModel.setType(exercise_number, exercise_idx, set_number));
			txtRestTime.text = exercisesModel.setRestTime(exercise_number, exercise_idx, set_number);
			subSetsLoader.subSetsNumber = exercisesModel.setSubSets(exercise_number, exercise_idx, set_number);
			txtNReps.text = exercisesModel.setReps(exercise_number, exercise_idx, set_number);
			txtNWeight.text = exercisesModel.setWeight(exercise_number, exercise_idx, set_number);
			txtNotes.text = exercisesModel.setNotes(exercise_number, exercise_idx, set_number);
			if (exercisesModel.isWorkout) {
				setCompleted = exercisesModel.setCompleted(exercise_number, exercise_idx, set_number);
				setModeLoader.setModeButton.enabled = workoutPageManager.canChangeSetMode(exercise_number, exercise_idx, set_number);
			}
		}
	}
} //delegate: ItemDelegate
