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
			if (navButtons) {
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

		contentItem: Rectangle {
			id: listItem
			width: lstSplitExercises.width
			border.color: "transparent"
			color: "transparent"
			radius: 5
		}

		background: Rectangle {
			id:	backgroundColor
			radius: 6
			color: splitModel.workingExercise === index ? appSettings.primaryColor :
							(index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
		}

		onClicked: splitModel.workingExercise = index;

		Connections {
			target: splitModel

			function onWorkingSubExerciseChanged(exercise_number: int, exercise_idx: int): void {
				delegate.changeFields(exercise_number, exercise_idx, splitModel.workingSet);
			}

			function onWorkingSetChanged(exercise_number: int, exercise_idx: int, set_number: int): void {
				delegate.changeFields(exercise_number, exercise_idx, set_number);
			}

			function onExerciseNameChanged(exercise_number: int, exercise_idx: int): void {
				txtExerciseName.text = splitModel.exerciseName(exercise_number, exercise_idx);
			}
		} //Connections

		function changeFields(exercise_number: int, exercise_idx: int, set_number: int): void {
			txtExerciseName.text = splitModel.exerciseName(exercise_number, exercise_idx);
			cboSetType.currentIndex = splitModel.setType(exercise_number, exercise_idx, set_number);
			txtNSubsets.text = splitModel.setSubSets(exercise_number, exercise_idx, set_number);
			txtNReps.text = splitModel.setReps(exercise_number, exercise_idx, set_number);
			txtNWeight.text = splitModel.setWeight(exercise_number, exercise_idx, set_number);
			txtNotes.text = splitModel.setNotes(exercise_number, exercise_idx, set_number);
			setsGroup.nSets = splitModel.setsNumber(exercise_number, exercise_idx);
			setsTabBar.setCurrentIndex(splitModel.workingSet);
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
					text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>" +
							(splitModel.exerciseIsComposite(delegate.exerciseNumber) ? qsTr("Giant sets") : "")
					checked: index === splitModel.workingExercise
					width: parent.width/2

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
					height: 20
					width: 20
					enabled: index === splitModel.workingExercise ? index > 0 : false

					anchors {
						right: btnMoveExerciseDown.left
						rightMargin: 10
						verticalCenter: parent.verticalCenter
					}

					onClicked: splitManager.moveRow(index, index-1, splitModel);
				} //btnMoveExerciseUp

				TPButton {
					id: btnMoveExerciseDown
					imageSource: "down.png"
					hasDropShadow: false
					height: 20
					width: 20
					enabled: index === splitModel.workingExercise ? index < splitModel.count-1 : false

					anchors {
						right: parent.right
						rightMargin: 15
						verticalCenter: parent.verticalCenter
					}

					onClicked: splitManager.moveRow(index, index+1, splitModel);
				} //btnMoveExerciseDown
			} //Item

			Item {
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
						text: qsTr(" <<-- Add some exercise")
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
								text: qsTr("Exercise # ") + parseInt(index + 1)
								checkable: true
								checked: index === splitModel.workingSubExercise
								width: subExercisesTabBar.width*0.22
								height: subExercisesTabBar.height*0.95

								contentItem: Label {
									text: subExercisesTabButton.text
									leftPadding: 2
									rightPadding: 2
									horizontalAlignment: Qt.AlignHCenter
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
				text: splitModel.exerciseName(index, splitModel.workingExercise)
				showRemoveButton: false
				enabled: delegate.nSubExercises > 0
				Layout.preferredWidth: parent.width
				Layout.preferredHeight: appSettings.pageHeight*0.1

				onExerciseChanged: (new_text) => splitModel.setExerciseName(splitModel.workingExercise, splitModel.workingSubExercise, new_text);
				onItemClicked: splitModel.workingExercise = index;
				onShowExercisesListButtonClicked: splitManager.simpleExercisesList(true, true);
			} //txtExerciseName

			GroupBox {
				id: setsGroup
				padding: 0
				spacing: 0
				enabled: delegate.nSubExercises > 0
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

				label: TPLabel {
					id: lblSetsNumber
					text: splitModel.setNumberLabel + String(setsGroup.nSets)
					width: parent.width
					horizontalAlignment: Text.AlignHCenter
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
											leftPadding: 2
											rightPadding: 2
											horizontalAlignment: Qt.AlignHCenter
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

					RowLayout {
						Layout.leftMargin: 5
						Layout.rightMargin: 5
						Layout.topMargin: 5
						enabled: setsGroup.nSets > 0

						TPLabel {
							text: splitModel.setTypeLabel
							width: listItem.width*0.4
							Layout.preferredWidth: width
						}

						TPComboBox {
							id: cboSetType
							enabled: index === splitModel.workingExercise
							model: AppGlobals.setTypesModel
							currentIndex: splitModel.setType(index, splitModel.workingSubExercise, splitModel.workingSet)
							Layout.preferredWidth: listItem.width*0.50

							onActivated: (cboIndex) => splitModel.setSetType(index, splitModel.workingSubExercise, splitModel.workingSet, cboIndex);
						}
					} //RowLayout

					RowLayout {
						visible: cboSetType.currentIndex >= 3
						Layout.leftMargin: 10
						Layout.rightMargin: 10
						Layout.fillWidth: true

						TPLabel {
							text: splitModel.setTotalSubsets
							Layout.preferredWidth: listItem.width*0.5
						}

						SetInputField {
							id: txtNSubsets
							text: splitModel.setSubSets(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet)
							type: SetInputField.Type.SetType
							availableWidth: listItem.width*0.3
							showLabel: false

							onValueChanged: (str) => splitModel.setSetSubSets(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
							onEnterOrReturnKeyPressed: txtNReps.forceActiveFocus();
						}
					} //RowLayout

					SetInputField {
						id: txtNReps
						text: splitModel.setReps(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet)
						type: SetInputField.Type.RepType
						availableWidth: listItem.width*0.9
						enabled: setsGroup.nSets > 0
						Layout.leftMargin: 10
						Layout.rightMargin: 10

						onValueChanged: (str) => splitModel.setSetReps(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
						onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
					} //txtNReps

					SetInputField {
						id: txtNWeight
						text: splitModel.setWeight(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet)
						type: SetInputField.Type.WeightType
						availableWidth: listItem.width*0.9
						enabled: setsGroup.nSets > 0
						Layout.leftMargin: 10
						Layout.rightMargin: 10

						onValueChanged: (str) => splitModel.setSetWeight(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
					} //txtNWeight

					SetNotesField {
						id: txtNotes
						info: splitModel.setNotesLabel
						text: splitModel.setNotes(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet)
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
} //Page
