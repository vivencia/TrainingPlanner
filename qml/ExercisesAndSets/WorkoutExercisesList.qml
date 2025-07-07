import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ListView {
	id: control
	model: workoutModel
	boundsBehavior: Flickable.StopAtBounds
	flickableDirection: Flickable.VerticalFlick
	spacing: 5

	delegate: ItemDelegate {
		id: delegate
		spacing: 10
		padding: 0
		implicitWidth: control.width
		implicitHeight: contentsLayout.implicitHeight * 1.1

		readonly property int exerciseNumber: index
		property int nSubExercises: workoutModel.subExercisesCount(delegate.exerciseNumber)
		property bool enabledRestTime: workoutModel.trackRestTime(exerciseNumber) && !workoutModel.autoRestTime(exerciseNumber)
		property bool setCompleted: workoutModel.setCompleted(exerciseNumber, workoutModel.workingSubExercise, workoutModel.workingSet)

		function exerciseFieldYPos(): int {
			return delegate.mapToItem(mainwindow.contentItem, txtExerciseName.x, txtExerciseName.y).y;
		}

		contentItem: Rectangle {
			id: listItem
			border.color: "transparent"
			color: workingExercise === index ? appSettings.primaryColor :
							(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
			radius: 5
		}

		onClicked: workingExercise = index;

		Connections {
			target: workoutManager

			function onUpdateRestTime(exercise_number: int, rest_time: string) : void {
				if (exercise_number === index) {
					txtRestTime.text = rest_time;
				}
			}
		}

		Connections {
			target: workoutModel

			function onWorkingExerciseChanged(exercise_number: int) : void {
				if (exercise_number === index) {
					const exercise_idx = workoutModel.workingSubExercise;
					subExercisesTabBar.currentIndex = exercise_idx;
					onWorkingSubExerciseChanged(exercise_number, exercise_idx);
				}
			}

			function onWorkingSubExerciseChanged(exercise_number: int, exercise_idx: int) : void {
				const set_number = workoutModel.workingSet;
				setsTabBar.currentIndex = set_number;
				delegate.changeFields(exercise_number, exercise_idx, set_number, false);
			}

			function onWorkingSetChanged(exercise_number: int, exercise_idx: int, set_number: int) : void {
				delegate.changeFields(exercise_number, exercise_idx, set_number, true);
			}

			function onExerciseNameChanged(exercise_number: int, exercise_idx: int) : void {
				if (exercise_number === index) {
					txtExerciseName.text = workoutModel.exerciseName(exercise_number, exercise_idx);
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
							delegate.setCompleted = workoutModel.setCompleted(exercise_number, exercise_idx, set_number);
						break;
					}
				}
			}

			function onSetModeChanged(exercise_number: int, exercise_idx: int, set_number: int, mode: int): void {
				if (exercise_number === index) {
					btnSetMode.text = workoutModel.setModeLabel(exercise_number, exercise_idx, set_number);
				}
			}
		} //Connections

		function changeFields(exercise_number: int, exercise_idx: int, set_number: int, only_settype_dependent_fields: bool): void {
			if (exercise_number === index) {
				if (!only_settype_dependent_fields) {
					txtExerciseName.text = workoutModel.exerciseName(exercise_number, exercise_idx);
					txtNotes.text = workoutModel.setNotes(exercise_number, exercise_idx, set_number);
				}
				cboSetType.currentIndex = workoutModel.setType(exercise_number, exercise_idx, set_number);
				txtRestTime.text = workoutModel.setRestTime(exercise_number, exercise_idx, set_number);
				txtNSubsets.text = workoutModel.setSubSets(exercise_number, exercise_idx, set_number);
				txtNReps.text = workoutModel.setReps(exercise_number, exercise_idx, set_number);
				txtNWeight.text = workoutModel.setWeight(exercise_number, exercise_idx, set_number);
				btnSetMode.enabled = workoutManager.canChangeSetMode(exercise_number, exercise_idx, set_number);
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

				TPRadioButton {
					id: optCurrentExercise
					text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>" + (delegate.nSubExercises > 1 ? qsTr(" - Giant sets") : "")
					checked: index === workoutModel.workingExercise
					width: parent.width * 0.7

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}

					onClicked: workoutModel.workingExercise = index;
				} //optCurrentExercise

				TPButton {
					id: btnDelExercise
					imageSource: "remove"
					width: appSettings.itemDefaultHeight
					height: width
					enabled: index === workoutModel.workingExercise

					anchors {
						right: btnMoveExerciseUp.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: workoutManager.removeExercise(index);
				} //btnDelExercise

				TPButton {
					id: btnMoveExerciseUp
					imageSource: "up.png"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight * 0.9
					height: width
					enabled: index === workoutModel.workingExercise ? (delegate.exerciseNumber >= 1) : false

					anchors {
						right: btnMoveExerciseDown.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: workoutModel.moveExercise(index, index-1);
				} //btnMoveExerciseUp

				TPButton {
					id: btnMoveExerciseDown
					imageSource: "down.png"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight * 0.9
					height: width
					enabled: index === workoutModel.workingExercise ? (delegate.exerciseNumber < workoutModel.count) : false

					anchors {
						right: parent.right
						rightMargin: 20
						verticalCenter: parent.verticalCenter
					}

					onClicked: sworkoutModel.moveExercise(index, index+1);
				} //btnMoveExerciseDown
			} //Item

			Item {
				enabled: index === workoutModel.workingExercise
				Layout.minimumWidth: listItem.width
				Layout.maximumWidth: listItem.width
				Layout.preferredHeight: appSettings.itemDefaultHeight*1.1

				TPButton {
					id: btnAddSubExercise
					imageSource: "plus"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight
					height: width

					onClicked: {
						nSubExercises = workoutModel.addSubExercise(delegate.exerciseNumber) + 1;
						subExercisesTabBar.setCurrentIndex(workoutModel.workingSubExercise);
					}

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}
				}

				StackLayout{
					id: subExercisesStack
					currentIndex: delegate.nSubExercises > 0 ? 1 : 0
					height: appSettings.itemDefaultHeight*1.2

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
						Layout.minimumHeight: appSettings.itemDefaultHeight * 2
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

							TabButton {
								id: subExercisesTabButton
								text: workoutModel.exerciseName(delegate.exerciseNumber, index)
								checkable: true
								checked: index === workoutModel.workingSubExercise
								height: subExercisesTabBar.height * 0.95

								contentItem: Label {
									text: subExercisesTabButton.text
									elide: Text.ElideRight
									horizontalAlignment: Qt.AlignHCenter
									verticalAlignment: Qt.AlignVCenter
									font.pixelSize: appSettings.smallFontSize
									color: appSettings.fontColor
								}

								background: Rectangle {
									border.color: appSettings.fontColor
									opacity: 0.8
									color: enabled ? (checked ? appSettings.primaryDarkColor : appSettings.primaryColor) : appSettings.disabledFontColor
								}

								onClicked: {
									workoutModel.workingSubExercise = index;
									if (text.startsWith(qsTr("Choose")))
										workoutManager.simpleExercisesList(true);
								}
							} //subExercisesTabButton
						} //subExerciseButtonsRepeater
					} //subExercisesTabBar
				} //StackLayout

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
						workoutModel.delSubExercise(workoutModel.workingExercise, workoutModel.workingSubExercise);
						delegate.nSubExercises--;
					}
				}
			} //Item

			ExerciseNameField {
				id: txtExerciseName
				text: workoutModel.exerciseName(index, workoutModel.workingSubExercise)
				showRemoveButton: false
				editable: delegate.nSubExercises > 0 && index === workoutModel.workingExercise
				Layout.preferredWidth: parent.width
				Layout.preferredHeight: appSettings.pageHeight * 0.1

				onExerciseChanged: (new_text) => workoutModel.setExerciseName(workoutModel.workingExercise, workoutModel.workingSubExercise, new_text);
				onItemClicked: workoutModel.workingExercise = index;
				onShowExercisesListButtonClicked: workoutManager.simpleExercisesList(true);
			} //txtExerciseName

			Row {
				id: trackRestTimeRow
				enabled: index === workoutModel.workingExercise
				spacing: 0
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10

				TPCheckBox {
					id: chkTrackRestTime
					text: workoutModel.trackRestTimeLabel
					checked: workoutModel.trackRestTime(index)
					width: listItem.width * 0.5

					onClicked: {
						workoutModel.setTrackRestTime(index, checked);
						delegate.enabledRestTime = checked;
					}
				}

				TPCheckBox {
					id: chkAutoRestTime
					text: workoutModel.autoRestTimeLabel
					checked: workoutModel.autoRestTime(index)
					width: listItem.width * 0.5

					onClicked: {
						workoutModel.setAutoRestTime(index, checked);
						delegate.enabledRestTime = !checked;
					}
				}
			}

			GroupBox {
				id: setsGroup
				padding: 0
				spacing: 0
				enabled: delegate.nSubExercises > 0 && index === workoutModel.workingExercise
				Layout.maximumWidth: parent.width
				Layout.minimumWidth: parent.width

				property int nSets: workoutModel.setsNumber(delegate.exerciseNumber, workoutModel.workingSubExercise)

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
						Layout.preferredHeight: appSettings.itemDefaultHeight*1.1

						TPButton {
							id: btnAddSet
							imageSource: "plus"
							hasDropShadow: false
							width: appSettings.itemDefaultHeight
							height: width

							onClicked: {
								setsGroup.nSets = workoutModel.addSet(workoutModel.workingExercise, workoutModel.workingSubExercise) + 1;
								setsTabBar.setCurrentIndex(workoutModel.workingSet);
							}

							anchors {
								left: parent.left
								verticalCenter: parent.verticalCenter
							}
						}

						StackLayout{
							id: setsStack
							currentIndex: setsGroup.nSets > 0 ? 1 : 0
							height: appSettings.itemDefaultHeight*1.2

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

									TabButton {
										id: tabButton
										text: qsTr("Set # ") + parseInt(index + 1)
										checkable: true
										checked: index === setsTabBar.currentIndex
										width: setsGroup.width * 0.22
										height: setsTabBar.height * 0.95

										contentItem: Label {
											text: tabButton.text
											horizontalAlignment: Qt.AlignHCenter
											verticalAlignment: Qt.AlignVCenter
											font.pixelSize: appSettings.smallFontSize
											color: appSettings.fontColor
										}

										background: Rectangle {
											border.color: appSettings.fontColor
											opacity: 0.8
											color: enabled ? (checked ? appSettings.primaryDarkColor : appSettings.primaryColor) : appSettings.disabledFontColor
										}

										onClicked: workoutModel.workingSet = index;
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
								verticalCenter: parent.verticalCenter
							}

							onClicked: {
								workoutModel.delSet(workoutModel.workingExercise, workoutModel.workingSubExercise, workoutModel.workingSet);
								setsGroup.nSets--;
							}
						}
					} //Item

					Row {
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width * 0.9
						Layout.minimumHeight: appSettings.itemDefaultHeight
						padding: 10
						enabled: setsGroup.nSets > 0

						TPLabel {
							text: workoutModel.setTypeLabel
							width: listItem.width * 0.36
						}

						TPComboBox {
							id: cboSetType
							enabled: index === workoutModel.workingExercise
							model: AppGlobals.setTypesModel
							currentIndex: workoutModel.setType(index, workoutModel.workingSubExercise, workoutModel.workingSet)
							width: listItem.width * 0.45

							onActivated: (cboIndex) => workoutModel.setSetType(index, workoutModel.workingSubExercise, workoutModel.workingSet, cboIndex);
						}
					} //RowLayout

					RowLayout {
						visible: setsGroup.nSets > 0 && cboSetType.currentIndex >= 3
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width * 0.9

						TPLabel {
							text: workoutModel.setTotalSubsets
							Layout.preferredWidth: listItem.width * 0.63
						}

						SetInputField {
							id: txtNSubsets
							text: workoutModel.setSubSets(index, workoutModel.workingSubExercise, workoutModel.workingSet)
							type: SetInputField.Type.SetType
							availableWidth: listItem.width * 0.25
							showLabel: false

							onValueChanged: (str) => workoutModel.setSetSubSets(workoutModel.workingExercise,
															workoutModel.workingSubExercise, workoutModel.workingSet, str);
							onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
						}
					} //RowLayout

					SetInputField {
						id: txtRestTime
						type: SetInputField.Type.TimeType
						text: workoutModel.setRestTime(index, workoutModel.workingSubExercise, workoutModel.workingSet)
						availableWidth: listItem.width * 0.9
						enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0 && delegate.enabledRestTime
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => workoutModel.setSetRestTime(workoutModel.workingExercise,
															workoutModel.workingSubExercise, workoutModel.workingSet, str);
					}

					SetInputField {
						id: txtNReps
						text: workoutModel.setReps(index, workoutModel.workingSubExercise, workoutModel.workingSet)
						type: SetInputField.Type.RepType
						availableWidth: listItem.width * 0.9
						enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => workoutModel.setSetReps(workoutModel.workingExercise,
															workoutModel.workingSubExercise, workoutModel.workingSet, str);
						onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
					} //txtNReps

					SetInputField {
						id: txtNWeight
						text: workoutModel.setWeight(index, workoutModel.workingSubExercise, workoutModel.workingSet)
						type: SetInputField.Type.WeightType
						availableWidth: listItem.width * 0.9
						enabled: setsGroup.nSets > 0 && cboSetType.currentIndex >= 0
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => workoutModel.setSetWeight(workoutModel.workingExercise,
															workoutModel.workingSubExercise, workoutModel.workingSet, str);
					} //txtNWeight

					SetNotesField {
						id: txtNotes
						info: workoutModel.setNotesLabel
						text: workoutModel.setNotes(index, workoutModel.workingSubExercise, workoutModel.workingSet)
						enabled: cboSetType.currentIndex >= 0
						Layout.fillWidth: true

						onEditFinished: (new_text) => workoutModel.setSetNotes(workoutModel.workingExercise,
														workoutModel.workingSubExercise, workoutModel.workingSet, new_text);
					}

					Item {
						id: setModeLayout
						visible: workoutModel ? (workoutModel.isWorkout && cboSetType.currentIndex >= 0) : false
						width: parent.width * 0.55
						Layout.alignment: Qt.AlignCenter
						Layout.maximumWidth: width
						Layout.minimumWidth: width
						Layout.minimumHeight: appSettings.itemDefaultHeight
						Layout.maximumHeight: appSettings.itemDefaultHeight

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
							id: btnSetMode
							text: workoutModel.setModeLabel(index, workoutModel.workingSubExercise, workoutModel.workingSet)
							width: parent.width - imgSetCompleted.width
							height: parent.height
							enabled: workoutManager.canChangeSetMode(index, workoutModel.workingSubExercise, workoutModel.workingSet)
							onClicked: workoutManager.setWorkingSetMode();

							anchors {
								left: imgSetCompleted.right
								verticalCenter: parent.verticalCenter
							}
						}
					} //setModeLayout
				} //setsItemsLayout
			} //paneSets
		} //contentsLayout
	} //delegate: ItemDelegate

	function reloadModel(): void {
		control.model = 0;
		control.model = workoutModel.count;
		txtGroups.text = workoutModel.muscularGroup();
	}

	function setScrollBarPosition(pos: int): void {
		if (pos === 0)
			vBar.setPosition(0);
		else
			vBar.setPosition(pos - vBar.size/2);
	}

	function updateTxtGroups(musculargroup: string): void {
		txtGroups.text = musculargroup;
	}

	function appendNewExerciseToDivision(): void {
		workoutManager.addExercise();
		control.currentIndex = workoutModel.workingExercise;
		control.positionViewAtIndex(workoutModel.workingExercise, ListView.Contain);
	}

	function exerciseNameFieldYPosition(): int {
		if (itemAtIndex(workoutModel.workingExercise))
			return itemAtIndex(workoutModel.workingExercise).exerciseFieldYPos();
		else
			return 0;
	}
} //ListView
