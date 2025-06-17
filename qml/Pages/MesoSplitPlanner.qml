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

		required property int index
		property alias exerciseNumber: index

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

			Connections {
				target: splitModel

				function onExerciseNameChanged(row: int): void {
					if (splitModel.workingExercise === row) {
						if (splitModel.exerciseIsComposite(row))
							cboSetType.currentIndex = 4;
						else {
							if (cboSetType.currentIndex === 4)
								cboSetType.currentIndex = -1;
						}
						txtExerciseName.text = splitModel.exerciseName(row, 0);
						lblExercise1.text = splitModel.exerciseName(row, 0);
						lblExercise2.text = splitModel.exerciseName(row, 1);
					}
				}

				function onSetsNumberChanged(row: int): void {
					if (splitModel.workingExercise === row) {
						buttonsRepeater.model = splitModel.setsNumber(row);
						lblSetsNumber.text = splitModel.setsNumberLabel + splitModel.setsNumber(index);
					}
				}

				function onWorkingSetChanged(row: int): void {
					if (splitModel.workingExercise === row) {
						const workingSet = splitModel.workingSet;
						cboSetType.currentIndex = splitModel.setType(row, workingSet);
						txtNSubsets.text = splitModel.setSubsets(row, workingSet);
						txtNReps.text = splitModel.setReps1(row, workingSet);
						txtNWeight.text = splitModel.setWeight1(row, workingSet);
						if (repsLoader.active) {
							repsLoader.nReps1 = splitModel.setReps1(row, workingSet);
							repsLoader.nReps2 = splitModel.setReps2(row, workingSet);
							weightsLoader.nWeight1 = splitModel.setWeight1(row, workingSet);
							weightsLoader.nWeight2 = splitModel.setWeight1(row, workingSet);
						}
					}
				}
			} //Connections

			Item {
				height: appSettings.itemDefaultHeight
				Layout.fillWidth: true

				TPRadioButton {
					id: optCurrentExercise
					text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>"
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
						splitModel.addSubExercise(delegate.exerciseNumber);
						subExercisesTabBar.setCurrentIndex(splitModel.workingSubExercise);
					}

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}
				}

				TabBar {
					id: subExercisesTabBar
					implicitWidth: width
					contentWidth: width
					clip: true

					anchors {
						left: btnAddSubExercise.right
						leftMargin: 5
						right: btnDelSubExercise.left
						verticalCenter: parent.verticalCenter
					}

					Repeater {
						id: subExerciseButtonsRepeater
						model: splitModel.subExercisesCount()
						anchors.fill: parent

						required property int index
						property alias exerciseIndex: index
						property int exerciseNumber: delegate.exerciseNumber

						TabButton {
							id: subExercisesTabButton
							text: qsTr("Exercise # ") + parseInt(subExerciseButtonsRepeater.exerciseIndex + 1)
							height: setsTabBar.height
							checkable: true
							checked: subExerciseButtonsRepeater.exerciseIndex === subExercisesTabBar.currentIndex
							width: subExercisesTabBar.width*0.22

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
								color: enabled ? checked ? appSettings.primaryDarkColor : appSettings.primaryColor : "gray"
							}

							onClicked: splitModel.workingSet = index;
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

					onClicked: splitModel.delSubExercise(splitModel.workingExercise, splitModel.workingSubExercise);
				}
			} //Item

			ExerciseNameField {
				id: txtExerciseName
				text: splitModel.exerciseName(index, splitModel.workingExercise)
				showRemoveButton: false
				width: parent.width
				height: appSettings.pageHeight*0.1
				Layout.preferredWidth: width
				Layout.preferredHeight: height

				Keys.onReturnPressed: cboSetType.forceActiveFocus(); //Alphanumeric keyboard
				onExerciseChanged: (new_text) => splitModel.setExerciseName(index, splitModel.workingExercise, new_text);
				onItemClicked: splitModel.workingExercise = index;
				onEditButtonClicked: splitManager.simpleExercisesList(!readOnly, true);
			} //txtExerciseName

			GroupBox {
				id: setsGroup

				label: TPLabel {
					id: lblSetsNumber
					text: splitModel.setNumberLabel + splitModel.setsNumber(index, 0)
					width: parent.width
					horizontalAlignment: Text.AlignHCenter
				}
				Layout.fillWidth: true
				Layout.leftMargin: 0
				Layout.rightMargin: 0
				Layout.bottomMargin: 10
				Layout.topMargin: 0
				padding: 0
				spacing: 0
				height: appSettings.pageHeight*0.3
				enabled: index === splitModel.workingExercise

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
						Layout.minimumWidth: listItem.width
						Layout.maximumWidth: listItem.width
						Layout.preferredHeight: appSettings.itemDefaultHeight*1.1

						TPButton {
							id: btnAddSet
							imageSource: "plus"
							hasDropShadow: false
							imageSize: 30
							z:2

							onClicked: {
								splitModel.addSet(index)
								setsTabBar.setCurrentIndex(splitModel.workingSet);
							}

							anchors {
								left: parent.left
								leftMargin: -5
								verticalCenter: parent.verticalCenter
							}
						}

						TabBar {
							id: setsTabBar
							implicitWidth: width
							contentWidth: width
							clip: true
							z: 1

							anchors {
								left: btnAddSet.right
								leftMargin: 5
								right: btnDelSet.left
								verticalCenter: parent.verticalCenter
							}

							Repeater {
								id: buttonsRepeater
								model: splitModel.setsNumber(index, splitModel.workingSubExercise)
								anchors.fill: parent

								TabButton {
									id: tabButton
									text: qsTr("Set # ") + parseInt(index + 1)
									height: setsTabBar.height
									checkable: true
									checked: index === setsTabBar.currentIndex
									width: setsGroup.width*0.22

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
										color: enabled ? checked ? appSettings.primaryDarkColor : appSettings.primaryColor : "gray"
									}

									onClicked: splitModel.workingSet = index;
								} //tabButton
							} //buttonsRepeater
						} //setsTabBar

						TPButton {
							id: btnDelSet
							imageSource: "minus"
							hasDropShadow: false
							imageSize: 30
							z:2

							anchors {
								right: parent.right
								rightMargin: 5
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitModel.delSet(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet);
						}
					} //Item

					RowLayout {
						Layout.leftMargin: 5
						Layout.rightMargin: 5
						Layout.topMargin: 5

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
							width: listItem.width*0.50
							Layout.preferredWidth: width

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

							onValueChanged: (str) => splitModel.setSetsSubSets(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
							onEnterOrReturnKeyPressed: {
								if (txtNReps.visible)
									txtNReps.forceActiveFocus();
								else
									txtNReps1.forceActiveFocus();
							}
						}
					} //RowLayout

					SetInputField {
						id: txtNReps
						text: splitModel.setReps(splitModel.workingExercise, splitModel.workingSubExercise, splitModel.workingSet)
						type: SetInputField.Type.RepType
						availableWidth: listItem.width - 40
						visible: cboSetType.currentIndex !== 4
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
						availableWidth: listItem.width - 40
						visible: cboSetType.currentIndex !== 4
						Layout.leftMargin: 10
						Layout.rightMargin: 10

						onValueChanged: (str) => splitModel.setSetWeight(splitModel.workingExercise,
															splitModel.workingSubExercise, splitModel.workingSet, str);
					} //txtNWeight

					SetNotesField {
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
