import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../ExercisesAndSets"
import "../TPWidgets"
import "../Dialogs"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Frame {
	id: paneSplit
	objectName: "mesoSplitPlanner"
	width: appSettings.pageWidth
	padding: 0
	spacing: 0

	required property DBMesoSplitModel splitModel
	required property SplitManager splitManager
	property PageScrollButtons navButtons: null

	background: Rectangle {
		border.color: "transparent"
		color: "transparent"
		radius: 5
	}

	TPLabel {
		id: lblMain
		text: qsTr("Training Division ") + splitModel.splitLetter()
		font: AppGlobals.largeFont
		width: parent.width
		height: 40
		horizontalAlignment: Text.AlignHCenter

		anchors {
			top: parent.top
			topMargin: 10
			left: parent.left
			bottomMargin: 10
		}
	}

	TPLabel {
		id: lblGroups
		text: qsTr("Muscle groups trained in this division:")
		width: parent.width-10

		anchors {
			top: lblMain.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottomMargin: 5
		}
	}

	TPTextInput {
		id: txtGroups
		text: splitModel.muscularGroup()
		readOnly: true
		suggestedHeight: 50

		anchors {
			top: lblGroups.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 35
		}
	}

	TPButton {
		imageSource: "choose.png"

		anchors {
			left: txtGroups.right
			verticalCenter: txtGroups.verticalCenter
		}

		onClicked: mesocyclesModel.getMesocyclePage(splitManager.mesoIdx());
	}

		ListView {
			id: lstSplitExercises
			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick
			currentIndex: splitModel.currentRow

			anchors {
				top: txtGroups.bottom
				topMargin: 10
				left: parent.left
				leftMargin: 5
				right: parent.right
				rightMargin: 5
				bottom: parent.bottom
				bottomMargin: 10
			}

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

			model: splitModel.count

			delegate: ItemDelegate {
				id: delegate
				spacing: 10
				padding: 0
				implicitWidth: lstSplitExercises.width
				implicitHeight: contentsLayout.implicitHeight

				required property int index

				ColumnLayout {
					id: contentsLayout
					spacing: 10

					Connections {
						target: splitModel

						function onExerciseNameChanged(row: int): void {
							if (splitModel.currentRow === row) {
								if (splitModel.exerciseIsComposite(row))
									cboSetType.currentIndex = 4;
								else {
									if (cboSetType.currentIndex === 4)
										cboSetType.currentIndex = -1;
								}
								txtExerciseName.text = splitModel.exerciseName(row);
								lblExercise1.text = splitModel.exerciseName1(row);
								lblExercise2.text = splitModel.exerciseName2(row);
							}
						}

						function onSetsNumberChanged(row: int): void {
							if (splitModel.currentRow === row) {
								buttonsRepeater.model = splitModel.setsNumber(row);
								lblSetsNumber.text = splitModel.setsNumberLabel + splitModel.setsNumber(index);
							}
						}

						function onWorkingSetChanged(row: int): void {
							if (splitModel.currentRow === row) {
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
					}

					anchors {
						fill: parent
						topMargin: 5
						leftMargin: 5
						rightMargin: 5
						bottomMargin: 5
					}

					Item {
						height: 25
						Layout.fillWidth: true

						TPRadioButton {
							id: optCurrentExercise
							text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>"
							checked: index === splitModel.currentRow
							width: parent.width/2

							anchors {
								left: parent.left
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitModel.currentRow = index;
						}

						TPButton {
							id: btnMoveExerciseUp
							imageSource: "up.png"
							hasDropShadow: false
							height: 20
							width: 20
							visible: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
							enabled: index === splitModel.currentRow ? index > 0 : false

							anchors {
								right: btnMoveExerciseDown.left
								rightMargin: 10
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitManager.moveRow(index, index-1, splitModel);
						}

						TPButton {
							id: btnMoveExerciseDown
							imageSource: "down.png"
							hasDropShadow: false
							height: 20
							width: 20
							visible: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
							enabled: index === splitModel.currentRow ? index < splitModel.count-1 : false

							anchors {
								right: parent.right
								rightMargin: 15
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitManager.moveRow(index, index+1, splitModel);
						}
					}

					ExerciseNameField {
						id: txtExerciseName
						text: splitModel.exerciseName(index)
						enabled: index === splitModel.currentRow
						editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
						width: parent.width
						height: appSettings.pageHeight*0.1
						Layout.preferredWidth: width
						Layout.preferredHeight: height

						Keys.onReturnPressed: cboSetType.forceActiveFocus(); //Alphanumeric keyboard
						onExerciseChanged: (new_text) => splitModel.setExerciseName(index, new_text);
						onItemClicked: splitModel.currentRow = index;
						onRemoveButtonClicked: splitManager.removeExercise();
						onEditButtonClicked: splitManager.simpleExercisesList(splitModel, !readOnly, true, 0);
					} //ExerciseNameField

					SetNotesField {
						info: splitModel.instructionsLabel
						text: splitModel.setsNotes(index)
						editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
						Layout.fillWidth: true

						onEditFinished: (new_text) => splitModel.setSetsNotes(index, new_text);
					}

					GroupBox {
						id: paneSets
						label: TPLabel {
							id: lblSetsNumber
							text: splitModel.setsNumberLabel + splitModel.setsNumber(index)
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
						enabled: index === splitModel.currentRow

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
								Layout.preferredHeight: 30

								TPButton {
									id: btnAddSet
									imageSource: "plus"
									hasDropShadow: false
									imageSize: 30
									visible: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
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
										model: splitModel.setsNumber(index)
										anchors.fill: parent

										TabButton {
											id: tabbutton
											text: qsTr("Set # ") + parseInt(index + 1)
											height: setsTabBar.height
											checkable: true
											checked: index === setsTabBar.currentIndex
											width: paneSets.width*0.22

											contentItem: Label {
												text: tabbutton.text
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
										}
									}
								} //setsTabBar

								TPButton {
									id: btnDelSet
									imageSource: "minus"
									hasDropShadow: false
									imageSize: 30
									visible: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
									z:2

									anchors {
										right: parent.right
										rightMargin: 5
										verticalCenter: parent.verticalCenter
									}

									onClicked: splitModel.delSet(index);
								}
							} //Item

							RowLayout {
								Layout.leftMargin: 5
								Layout.rightMargin: 5
								Layout.topMargin: 5

								TPLabel {
									text: splitModel.typeLabel
									width: listItem.width*0.4
									Layout.preferredWidth: width
								}
								TPComboBox {
									id: cboSetType
									enabled: index === splitModel.workingExercise
									model: AppGlobals.setTypesModel
									editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
									currentIndex: splitModel.setType(index, splitModel.workingSet)
									width: listItem.width*0.50
									Layout.preferredWidth: width

									onActivated: (cboIndex) => splitModel.setSetType(index, splitModel.workingSet, cboIndex);
								}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 2 || cboSetType.currentIndex === 3 || cboSetType.currentIndex === 5
								Layout.leftMargin: 10
								Layout.rightMargin: 10
								Layout.fillWidth: true

								TPLabel {
									text: splitModel.subSetsLabel
									Layout.preferredWidth: listItem.width*0.5
								}
								SetInputField {
									id: txtNSubsets
									text: splitModel.setSubsets(index, splitModel.workingSet)
									editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
									type: SetInputField.Type.SetType
									availableWidth: listItem.width*0.3
									showLabel: false

									onValueChanged: (str) => splitModel.setSetsSubsets(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: {
										if (txtNReps.visible)
											txtNReps.forceActiveFocus();
										else
											txtNReps1.forceActiveFocus();
									}
								}
							}

							RowLayout {
								uniformCellSizes: true
								visible: cboSetType.currentIndex === 4
								spacing: 5
								Layout.fillWidth: true
								Layout.topMargin: 10
								Layout.leftMargin: 5

								TPLabel {
									id: lblExercise1
									text: splitModel.exerciseName1(index)
									wrapMode: Text.WordWrap
									width: listItem.width*0.5
									Layout.preferredWidth: width
									Layout.minimumHeight: _preferredHeight

									MouseArea {
										anchors.fill: parent
										enabled: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
										onClicked: splitManager.simpleExercisesList(splitModel, true, false, 1);
									}
								}

								TPLabel {
									id: lblExercise2
									text: splitModel.exerciseName2(index)
									wrapMode: Text.WordWrap
									width: listItem.width*0.5
									Layout.preferredWidth: width
									Layout.minimumHeight: _preferredHeight

									MouseArea {
										anchors.fill: parent
										enabled: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
										onClicked: splitManager.simpleExercisesList(splitModel, true, false, 2);
									}
								}
							}

							SetInputField {
								id: txtNReps
								text: splitModel.setReps1(index, splitModel.workingSet)
								type: SetInputField.Type.RepType
								availableWidth: listItem.width - 40
								visible: cboSetType.currentIndex !== 4
								editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
								Layout.leftMargin: 10
								Layout.rightMargin: 10

								onValueChanged: (str) => splitModel.setSetReps1(index, splitModel.workingSet, str);
								onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
							}

							Loader {
								id: repsLoader
								active: cboSetType.currentIndex === 4
								asynchronous: true

								property string nReps1: splitModel.setReps1(index, splitModel.workingSet)
								property string nReps2: splitModel.setReps2(index, splitModel.workingSet)

								sourceComponent: RowLayout {
									Layout.fillWidth: true
									Layout.leftMargin: 10

								SetInputField {
									id: txtNReps1
									text: repsLoader.nReps1
									type: SetInputField.Type.RepType
									availableWidth: listItem.width*0.55
									editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())

									onValueChanged: (str) => splitModel.setSetReps1(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
								}

								SetInputField {
									id: txtNReps2
									text: repsLoader.nReps2
									type: SetInputField.Type.RepType
									availableWidth: listItem.width*0.4
									showLabel: false
									editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())

									onValueChanged: (str) => splitModel.setSetReps2(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
								}
							} //RowLayout
							} //Loader

							SetInputField {
								id: txtNWeight
								text: splitModel.setWeight1(index, splitModel.workingSet)
								type: SetInputField.Type.WeightType
								availableWidth: listItem.width - 40
								editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())
								visible: cboSetType.currentIndex !== 4
								Layout.leftMargin: 10
								Layout.rightMargin: 10

								onValueChanged: (str) => splitModel.setSetWeight1(index, splitModel.workingSet, str);
							}

							Loader {
								id: weightsLoader
								active: cboSetType.currentIndex === 4
								asynchronous: true

								property string nWeight1: splitModel.setWeight1(index, splitModel.workingSet)
								property string nWeight2: splitModel.setWeight2(index, splitModel.workingSet)

								sourceComponent: RowLayout {
								visible: cboSetType.currentIndex === 4
								Layout.fillWidth: true
								Layout.leftMargin: 10

								SetInputField {
									id: txtNWeight1
									text: weightsLoader.nWeight1
									type: SetInputField.Type.WeightType
									availableWidth: listItem.width*0.55
									editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())

									onValueChanged: (str) => splitModel.setSetWeight1(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
								}

								SetInputField {
									id: txtNWeight2
									text: weightsLoader.nWeight2
									type: SetInputField.Type.WeightType
									showLabel: false
									availableWidth: listItem.width*0.4
									editable: mesocyclesModel.isOwnMeso(splitManager.mesoIdx())

									onValueChanged: (str) => splitModel.setSetWeight2(index, splitModel.workingSet, str);
								}
							} //RowLayout
							} //Loader
						} //ColumnLayout
					} //Pane
				} //ColumnLayout

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
					color: splitModel.workingExercise === index ? appSettings.primaryColor : index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
				}

				onClicked: splitModel.workingExercise = index;
			} //delegate: ItemDelegate
		} //ListView

	Component.onCompleted: splitModel.modelChanged.connect(reloadModel);

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
		lstSplitExercises.positionViewAtIndex(splitModel.currentRow, ListView.Center);
	}
} //Page
