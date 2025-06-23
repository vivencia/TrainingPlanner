import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../ExercisesAndSets"
import "../TPWidgets"
import "../Dialogs"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ListView {
	id: lstSplitExercises
	objectName: "mesoSplitPlanner"
	model: splitModel
	boundsBehavior: Flickable.StopAtBounds
	flickableDirection: Flickable.VerticalFlick
	currentIndex: splitModel.workingExercise
	width: appSettings.pageWidth
	spacing: 5

	required property DBMesoSplitModel splitModel
	required property SplitManager splitManager
	property PageScrollButtons navButtons: null

	ScrollBar.vertical: ScrollBar {
		id: vBar
		policy: ScrollBar.AsNeeded
		active: ScrollBar.AsNeeded
		visible: lstSplitExercises.contentHeight > lstSplitExercises.height

		onPositionChanged: {
			if (navButtons.visible) {
				if (lstSplitExercises.contentY <= 50) {
					navButtons.showUpButton = false;
					navButtons.showDownButton = true;
				}
				else if (lstSplitExercises.contentHeight - lstSplitExercises.contentY - vBar.height <= 50) {
					navButtons.showUpButton = true;
					navButtons.showDownButton = false;
				}
				else {
					navButtons.showUpButton = true;
					navButtons.showDownButton = true;
				}
			}
		}
	}

	delegate: ItemDelegate {
		id: delegate
		spacing: 10
		padding: 0
		implicitWidth: lstSplitExercises.width
		implicitHeight: contentsLayout.implicitHeight

		readonly property int exerciseNumber: index
		property int nSubExercises: splitModel.subExercisesCount(delegate.exerciseNumber)

		function exerciseFieldYPos(): int {
			return delegate.mapToItem(mainwindow.contentItem, txtExerciseName.x, txtExerciseName.y).y;
		}

		contentItem: Rectangle {
			id: listItem
			width: lstSplitExercises.width
			border.color: "transparent"
			color: "transparent"
			radius: 5
		}

		background: Rectangle {
			id:	backgroundColor
			color: splitModel.workingExercise === index ? appSettings.primaryColor :
							(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
		}

		onClicked: splitModel.workingExercise = index;

		Connections {
			target: splitModel

			function onWorkingSubExerciseChanged(exercise_number: int, exercise_idx: int): void {
				delegate.changeFields(exercise_number, exercise_idx, splitModel.workingSet, false);
			}

			function onWorkingSetChanged(exercise_number: int, exercise_idx: int, set_number: int): void {
				delegate.changeFields(exercise_number, exercise_idx, set_number, true);
			}

			function onExerciseNameChanged(exercise_number: int, exercise_idx: int): void {
				if (exercise_number === index) {
					txtExerciseName.text = splitModel.exerciseName(exercise_number, exercise_idx);
					subExerciseButtonsRepeater.itemAt(exercise_idx).text = txtExerciseName.text;
				}
			}

			function onExerciseModified(exercise_number: int, exercise_idx: int, set_number: int, field: int): void {
				if (exercise_number === index) {
					switch (field) {
						case 5: //EXERCISES_COL_TRACKRESTTIMES
						case 6: //EXERCISES_COL_AUTORESTTIMES
							txtRestTime.enabled = cboSetType.currentIndex >= 0 && splitModel.trackRestTime(index) && !splitModel.autoRestTime(index);
						break;
						case 7: //EXERCISES_COL_SETTYPES
							changeFields(index, exercise_idx, set_number, true);
						break;
					}
				}
			}
		} //Connections

		function changeFields(exercise_number: int, exercise_idx: int, set_number: int, only_settype_dependent_fields: bool): void {
			if (exercise_number === index)
			{
				if (!only_settype_dependent_fields) {
					txtExerciseName.text = splitModel.exerciseName(exercise_number, exercise_idx);
					cboSetType.currentIndex = splitModel.setType(exercise_number, exercise_idx, set_number);
					txtNotes.text = splitModel.setNotes(exercise_number, exercise_idx, set_number);
				}
				txtRestTime.text = splitModel.setRestTime(exercise_number, exercise_idx, set_number);
				txtRestTime.enabled = cboSetType.currentIndex >= 0 && splitModel.trackRestTime(exercise_number) && !splitModel.autoRestTime(exercise_number);
				txtNSubsets.text = splitModel.setSubSets(exercise_number, exercise_idx, set_number);
				txtNReps.text = splitModel.setReps(exercise_number, exercise_idx, set_number);
				txtNWeight.text = splitModel.setWeight(exercise_number, exercise_idx, set_number);
				setsGroup.nSets = splitModel.setsNumber(exercise_number, exercise_idx);
				//setsTabBar.setCurrentIndex(splitModel.workingSet);
			}
		}

		ColumnLayout {
			id: contentsLayout
			spacing: 10

			anchors {
				fill: parent
				topMargin: 5
				leftMargin: 5
				rightMargin: 5
				bottomMargin: 5
			}

			Item {
				height: appSettings.itemDefaultHeight
				Layout.fillWidth: true

				TPRadioButton {
					id: optCurrentExercise
					text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>" + (delegate.nSubExercises > 1 ? qsTr(" - Giant sets") : "")
					checked: index === splitModel.workingExercise
					width: parent.width*0.7

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}

					onClicked: splitModel.workingExercise = index;
				} //optCurrentExercise

				TPButton {
					id: btnMoveExerciseUp
					imageSource: "up.png"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight*0.9
					height: width
					enabled: index === splitModel.workingExercise ? (delegate.exerciseNumber >= 1) : false

					anchors {
						right: btnMoveExerciseDown.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: splitModel.moveExercise(index, index-1);
				} //btnMoveExerciseUp

				TPButton {
					id: btnMoveExerciseDown
					imageSource: "down.png"
					hasDropShadow: false
					width: appSettings.itemDefaultHeight*0.9
					height: width
					enabled: index === splitModel.workingExercise ? (delegate.exerciseNumber < (splitModel.count-1)) : false

					anchors {
						right: parent.right
						rightMargin: 20
						verticalCenter: parent.verticalCenter
					}

					onClicked: ssplitModel.moveExercise(index, index+1);
				} //btnMoveExerciseDown
			} //Item

			Item {
				enabled: index === splitModel.workingExercise
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
						nSubExercises = splitModel.addSubExercise(delegate.exerciseNumber) + 1;
						subExercisesTabBar.setCurrentIndex(splitModel.workingSubExercise);
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
						horizontalAlignment: Text.AlignHCenter
						Layout.fillWidth: true
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
								text: splitModel.exerciseName(delegate.exerciseNumber, index)
								checkable: true
								checked: index === splitModel.workingSubExercise
								height: subExercisesTabBar.height*0.95

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

								onClicked: splitModel.workingSubExercise = index;
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
						splitModel.delSubExercise(splitModel.workingExercise, splitModel.workingSubExercise);
						delegate.nSubExercises--;
					}
				}
			} //Item

			ExerciseNameField {
				id: txtExerciseName
				text: splitModel.exerciseName(index, splitModel.workingSubExercise)
				showRemoveButton: false
				editable: delegate.nSubExercises > 0 && index === splitModel.workingExercise
				Layout.preferredWidth: parent.width
				Layout.preferredHeight: appSettings.pageHeight*0.1

				onExerciseChanged: (new_text) => splitModel.setExerciseName(splitModel.workingExercise, splitModel.workingSubExercise, new_text);
				onItemClicked: splitModel.workingExercise = index;
				onShowExercisesListButtonClicked: splitManager.simpleExercisesList(true, true);
			} //txtExerciseName

			Row {
				id: trackRestTimeRow
				enabled: index === splitModel.workingExercise
				spacing: 0
				Layout.fillWidth: true
				Layout.leftMargin: 10
				Layout.rightMargin: 10

				TPCheckBox {
					id: chkTrackRestTime
					text: splitModel.trackRestTimeLabel
					checked: splitModel.trackRestTime(index)
					width: listItem.width*0.5

					onClicked: splitModel.setTrackRestTime(index, checked);
				}

				TPCheckBox {
					id: chkAutoRestTime
					text: splitModel.autoRestTimeLabel
					checked: splitModel.autoRestTime(index)
					width: listItem.width*0.5

					onClicked: splitModel.setAutoRestTime(index, checked);
				}
			}

			GroupBox {
				id: setsGroup
				padding: 0
				spacing: 0
				enabled: delegate.nSubExercises > 0 && index === splitModel.workingExercise
				width: parent.width
				Layout.preferredWidth: width
				Layout.preferredHeight: height
				Layout.leftMargin: 0
				Layout.rightMargin: 0
				Layout.bottomMargin: 10
				Layout.topMargin: 0

				property int nSets: splitModel.setsNumber(delegate.exerciseNumber, splitModel.workingSubExercise)

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
								setsGroup.nSets = splitModel.addSet(splitModel.workingExercise, splitModel.workingSubExercise) + 1;
								setsTabBar.setCurrentIndex(splitModel.workingSet);
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
										width: setsGroup.width*0.22
										height: setsTabBar.height*0.95

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

										onClicked: splitModel.workingSet = index;
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
								splitModel.delSet(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet);
								setsGroup.nSets--;
							}
						}
					} //Item

					Row {
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width*0.9
						padding: 10
						enabled: setsGroup.nSets > 0

						TPLabel {
							text: splitModel.setTypeLabel
							width: listItem.width*0.36
						}

						TPComboBox {
							id: cboSetType
							enabled: index === splitModel.workingExercise
							model: AppGlobals.setTypesModel
							currentIndex: splitModel.setType(index, splitModel.workingSubExercise, splitModel.workingSet)
							width: listItem.width*0.45

							onActivated: (cboIndex) => splitModel.setSetType(index, splitModel.workingSubExercise, splitModel.workingSet, cboIndex);
						}
					} //RowLayout

					RowLayout {
						visible: cboSetType.currentIndex >= 3
						Layout.alignment: Qt.AlignCenter
						Layout.preferredWidth: listItem.width*0.9

						TPLabel {
							text: splitModel.setTotalSubsets
							Layout.preferredWidth: listItem.width*0.63
						}

						SetInputField {
							id: txtNSubsets
							text: splitModel.setSubSets(index, splitModel.workingSubExercise, splitModel.workingSet)
							type: SetInputField.Type.SetType
							availableWidth: listItem.width*0.25
							showLabel: false

							onValueChanged: (str) => splitModel.setSetSubSets(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
							onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
						}
					} //RowLayout

					SetInputField {
						id: txtRestTime
						type: SetInputField.Type.TimeType
						text: splitModel.setRestTime(index, splitModel.workingSubExercise, splitModel.workingSet)
						availableWidth: listItem.width*0.9
						enabled: cboSetType.currentIndex >= 0 && splitModel.trackRestTime(index) && !splitModel.autoRestTime(index)
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => splitModel.setSetRestTime(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
					}

					SetInputField {
						id: txtNReps
						text: splitModel.setReps(index, splitModel.workingSubExercise, splitModel.workingSet)
						type: SetInputField.Type.RepType
						availableWidth: listItem.width*0.9
						enabled: cboSetType.currentIndex >= 0
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => splitModel.setSetReps(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
						onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
					} //txtNReps

					SetInputField {
						id: txtNWeight
						text: splitModel.setWeight(index, splitModel.workingSubExercise, splitModel.workingSet)
						type: SetInputField.Type.WeightType
						availableWidth: listItem.width*0.9
						enabled: cboSetType.currentIndex >= 0
						Layout.alignment: Qt.AlignCenter

						onValueChanged: (str) => splitModel.setSetWeight(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
					} //txtNWeight

					SetNotesField {
						id: txtNotes
						info: splitModel.setNotesLabel
						text: splitModel.setNotes(index, splitModel.workingSubExercise, splitModel.workingSet)
						enabled: cboSetType.currentIndex >= 0
						Layout.fillWidth: true

						onEditFinished: (new_text) => splitModel.setSetNotes(splitModel.workingExercise,
														splitModel.workingSubExercise, splitModel.workingSet, new_text);
					}
				} //setsItemsLayout
			} //paneSets
		} //contentsLayout
	} //delegate: ItemDelegate

	function reloadModel(): void {
		lstSplitExercises.model = 0;
		lstSplitExercises.model = splitModel.count;
		txtGroups.text = splitModel.muscularGroup();
	}

	function setScrollBarPosition(pos): void {
		if (pos === 0)
			vBar.setPosition(0);
		else
			vBar.setPosition(pos - vBar.size/2);
	}

	function updateTxtGroups(musculargroup: string): void {
		txtGroups.text = musculargroup;
	}

	function appendNewExerciseToDivision(): void {
		splitManager.addExercise();
		lstSplitExercises.currentIndex = splitModel.workingExercise;
		lstSplitExercises.positionViewAtIndex(splitModel.workingExercise, ListView.Center);
	}

	function exerciseNameFieldYPosition(): int {
		if (itemAtIndex(splitModel.workingExercise))
			return itemAtIndex(splitModel.workingExercise).exerciseFieldYPos();
		else
			return 0;
	}
} //Page
