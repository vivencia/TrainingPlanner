import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../ExercisesAndSets"
import "../TPWidgets"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Frame {
	id: paneSplit
	objectName: "mesoSplitPlanner"

	required property TPPage parentItem
	required property DBMesoSplitModel splitModel
	required property SplitManager splitManager

	property bool bCanSwapPlan
	property string swappableLetter
	property string prevMesoName
	property int prevMesoId

	width: appSettings.pageWidth
	padding: 0
	spacing: 0

	background: Rectangle {
		border.color: "transparent"
		radius: 5
	}

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise?")
		message: splitModel.exerciseName(splitModel.currentRow) + qsTr("\nThis action cannot be undone.")
		imageSource: "remove"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		onButton1Clicked: {
			splitModel.removeExercise(splitModel.currentRow);
			lstSplitExercises.positionViewAtIndex(splitModel.currentRow, ListView.Center);
		}
		parentPage: parentItem
	} //TPBalloonTip

	TPLabel {
		id: lblMain
		text: qsTr("Training Division ") + splitModel.splitLetter()
		font: AppGlobals.extraLargeFont
		width: parent.width
		fontColor: "black"
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
		singleLine: true
		color: "black"

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
		width: parent.width - 20

		anchors {
			top: lblGroups.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottomMargin: 10
		}

		onEditingFinished: {
			splitManager.changeMuscularGroup(text, splitModel);
			exercisesModel.makeFilterString(text);
		}
	}

		ListView {
			id: lstSplitExercises
			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

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
					if (parentItem.navButtons) {
						if (lstSplitExercises.contentY <= 50) {
							parentItem.navButtons.showUpButton = false;
							parentItem.navButtons.showDownButton = true;
						}
						else if (lstSplitExercises.contentHeight - lstSplitExercises.contentY - vBar.height <= 50) {
							parentItem.navButtons.showUpButton = true;
							parentItem.navButtons.showDownButton = false;
						}
						else {
							parentItem.navButtons.showUpButton = true;
							parentItem.navButtons.showDownButton = true;
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

						function onExerciseNameChanged(row) {
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

						function onSetsNumberChanged(row) {
							if (splitModel.currentRow === row)
								buttonsRepeater.model = splitModel.setsNumber(row);
						}
					}

					anchors {
						fill: parent
						topMargin: 5
						leftMargin: 5
						rightMargin: 5
						bottomMargin: 5
					}

					TPRadioButton {
						id: optCurrentExercise
						text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>"
						textColor: "black"
						checked: index === splitModel.currentRow
						width: parent.width
						Layout.preferredWidth: width

						onClicked: splitModel.currentRow = index;

						TPButton {
							id: btnMoveExerciseUp
							imageSource: "up"
							hasDropShadow: false
							height: 30
							width: 30
							enabled: index === splitModel.currentRow ? index > 0 : false

							anchors {
								right: btnMoveExerciseDown.left
								rightMargin: -5
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitModel.moveRow(index,index-1);
						}

						TPButton {
							id: btnMoveExerciseDown
							imageSource: "down"
							hasDropShadow: false
							height: 30
							width: 30
							enabled: index === splitModel.currentRow ? index < splitModel.count-1 : false

							anchors {
								right: parent.right
								rightMargin: 15
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitModel.moveRow(index,index+1);
						}
					}

					ExerciseNameField {
						id: txtExerciseName
						text: splitModel.exerciseName(index)
						enabled: index === splitModel.currentRow
						width: parent.width
						height: appSettings.pageHeight*0.1
						Layout.preferredWidth: width
						Layout.preferredHeight: height

						//Alphanumeric keyboard
						Keys.onReturnPressed: cboSetType.forceActiveFocus();
						onExerciseChanged: (new_text) => splitModel.setExerciseName(index, new_text);
						onItemClicked: splitModel.currentRow = index;

						onRemoveButtonClicked: msgDlgRemove.show(0);
						onEditButtonClicked: splitManager.simpleExercisesList(splitModel, !readOnly, true, 0);

						onMousePressed: (mouse) => {
							if (!readOnly) {
								mouse.accepted = true;
								forceActiveFocus();
							}
							else
								mouse.accepted = false; //relay the signal to the delegate
						}

						onMousePressAndHold: (mouse) => mouse.accepted = false;
					} //ExerciseNameField

					SetNotesField {
						info: splitModel.instructionsLabel
						text: splitModel.setsNotes(index)
						Layout.fillWidth: true

						onEditFinished: (new_text) => splitModel.setSetsNotes(index, new_text);
					}

					Pane {
						id: paneSets
						Layout.fillWidth: true
						Layout.leftMargin: -20
						Layout.rightMargin: 0
						Layout.bottomMargin: 0
						Layout.topMargin: -20
						height: appSettings.pageHeight*0.3
						enabled: index === splitModel.currentRow

						background: Rectangle {
							color: "transparent"
						}

						ColumnLayout {
							id: setsItemsLayout
							anchors.fill: parent
							spacing: 5

							Frame {
								Layout.minimumWidth: listItem.width
								Layout.maximumWidth: listItem.width
								clip: true

								background: Rectangle {
									border.width: 0
									color: "transparent"
								}

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
												font.pixelSize: appSettings.smallFontSize
												color: appSettings.fontColor
											}

											background: Rectangle {
												border.color: appSettings.fontColor
												radius: 6
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
									z:2

									anchors {
										right: parent.right
										rightMargin: -10
										verticalCenter: parent.verticalCenter
									}

									onClicked: splitModel.delSet(index);
								}
							} //Frame


							RowLayout {
								Layout.leftMargin: 20
								Layout.rightMargin: 20

								TPLabel {
									text: splitModel.typeLabel
									fontColor: "black"
									width: listItem.width*0.4
									Layout.preferredWidth: width
								}
								TPComboBox {
									id: cboSetType
									enabled: index === splitModel.currentRow
									model: AppGlobals.setTypesModel
									currentIndex: splitModel.setType(index, splitModel.workingSet)
									width: listItem.width*0.55
									Layout.rightMargin: 5

									onActivated: (cboIndex) => splitModel.setSetType(index, splitModel.workingSet, cboIndex);

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													currentIndex = splitModel.setType(row, splitModel.workingSet)
											//}
										});
									}
								}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 2 || cboSetType.currentIndex === 3 || cboSetType.currentIndex === 5
								Layout.leftMargin: 10
								Layout.rightMargin: 10
								Layout.fillWidth: true

								TPLabel {
									text: splitModel.subSetsLabel
									fontColor: "black"
									Layout.preferredWidth: listItem.width*0.5
								}
								SetInputField {
									id: txtNSubsets
									text: splitModel.setSubsets(index, splitModel.workingSet)
									type: SetInputField.Type.SetType
									availableWidth: listItem.width*0.3
									showLabel: false

									onValueChanged: (str) => splitModel.setSetsSubsets(index, splitModel.workingSet, str);

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													text = splitModel.setSubsets(row, splitModel.workingSet);
											//}
										});
									}

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
								Layout.fillWidth: true
								Layout.topMargin: 10
								Layout.leftMargin: 10

								TPLabel {
									id: lblExercise1
									text: splitModel.exerciseName1(index)
									wrapMode: Text.WordWrap
									fontColor: "black"
									width: listItem.width*0.5
									Layout.preferredWidth: width
									Layout.preferredHeight: _preferredHeight
									Layout.alignment: Qt.AlignHCenter

									MouseArea {
										anchors.fill: parent
										onClicked: {
											splitModel.currentRow = index;
											splitManager.simpleExercisesList(splitModel, true, false, 1);
										}
									}
								}

								TPLabel {
									id: lblExercise2
									text: splitModel.exerciseName2(index)
									wrapMode: Text.WordWrap
									fontColor: "black"
									width: listItem.width*0.5
									Layout.preferredWidth: width
									Layout.preferredHeight: _preferredHeight
									Layout.alignment: Qt.AlignRight

									MouseArea {
										anchors.fill: parent
										onClicked: {
											splitModel.currentRow = index;
											splitManager.simpleExercisesList(splitModel, true, false, 2);
										}
									}
								}
							}

							SetInputField {
								id: txtNReps
								text: splitModel.setReps1(index, splitModel.workingSet)
								type: SetInputField.Type.RepType
								availableWidth: listItem.width - 40
								visible: cboSetType.currentIndex !== 4
								Layout.leftMargin: 20
								Layout.rightMargin: 20

								onValueChanged: (str) => splitModel.setSetReps1(index, splitModel.workingSet, str);
								onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
								Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													text = splitModel.setReps1(row, splitModel.workingSet);
											//}
										});
									}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 4
								Layout.fillWidth: true
								Layout.leftMargin: 10

								SetInputField {
									id: txtNReps1
									text: splitModel.setReps1(index, splitModel.workingSet)
									type: SetInputField.Type.RepType
									availableWidth: listItem.width*0.55

									onValueChanged: (str) => splitModel.setSetReps1(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													text = splitModel.setReps1(row, splitModel.workingSet);
											//}
										});
									}
								}

								SetInputField {
									id: txtNReps2
									text: splitModel.setReps2(index, splitModel.workingSet)
									type: SetInputField.Type.RepType
									availableWidth: listItem.width*0.4
									showLabel: false

									onValueChanged: (str) => splitModel.setSetReps2(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													text = splitModel.setReps2(row, splitModel.workingSet);
											//}
										});
									}
								}
							} //RowLayout

							SetInputField {
								id: txtNWeight
								text: splitModel.setWeight1(index, splitModel.workingSet)
								type: SetInputField.Type.WeightType
								availableWidth: listItem.width - 40
								visible: cboSetType.currentIndex !== 4
								Layout.leftMargin: 20
								Layout.rightMargin: 20

								onValueChanged: (str) => splitModel.setSetWeight1(index, splitModel.workingSet, str);

								Component.onCompleted: {
									splitModel.workingSetChanged.connect(function(row) {
										//if (visible) {
											if (index === row)
												text = splitModel.setWeight1(row, splitModel.workingSet);
										//}
									});
								}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 4
								Layout.fillWidth: true
								Layout.leftMargin: 10

								SetInputField {
									id: txtNWeight1
									text: splitModel.setWeight1(index, splitModel.workingSet)
									type: SetInputField.Type.WeightType
									availableWidth: listItem.width*0.55

									onValueChanged: (str) => splitModel.setSetWeight1(index, splitModel.workingSet, str);
									onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													text = splitModel.setWeight1(row, splitModel.workingSet);
											//}
										});
									}
								}

								SetInputField {
									id: txtNWeight2
									text: splitModel.setWeight2(index, splitModel.workingSet)
									type: SetInputField.Type.WeightType
									showLabel: false
									availableWidth: listItem.width*0.4

									onValueChanged: (str) => splitModel.setSetWeight2(index, splitModel.workingSet, str);

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function(row) {
											//if (visible) {
												if (index === row)
													text = splitModel.setWeight2(row, splitModel.workingSet);
											//}
										});
									}
								}
							} //RowLayout
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
					color: splitModel.currentRow === index ? appSettings.primaryLightColor : index % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
				}

				onClicked: splitModel.currentRow = index;
			} //delegate: ItemDelegate
		} //ListView

	Component.onCompleted: {
		exercisesModel.makeFilterString(txtGroups.text);
		lstSplitExercises.currentIndex = splitModel.currentRow;
		lstSplitExercises.positionViewAtIndex(0, ListView.Center);
	}

	function setScrollBarPosition(pos): void {
		if (pos === 0)
			vBar.setPosition(0);
		else
			vBar.setPosition(pos - vBar.size/2);
	}

	function updateTxtGroups(musculargroup: string): void {
		txtGroups.text = musculargroup;
		exercisesModel.makeFilterString(musculargroup);
	}

	function appendNewExerciseToDivision(): void {
		splitModel.appendExercise();
		lstSplitExercises.currentIndex = splitModel.currentRow;
		lstSplitExercises.positionViewAtIndex(splitModel.currentRow, ListView.Center);
	}

	TPBalloonTip {
		id: msgDlgImport
		title: qsTr("Import Exercises Plan?")
		message: qsTr("Import the exercises plan for training division <b>") + splitModel.splitLetter() +
						 qsTr("</b> from <b>") + prevMesoName + "</b>?"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "remove"
		parentPage: parentItem

		onButton1Clicked: splitManager.loadSplitFromPreviousMeso(splitModel);
	} //TPBalloonTip

	function showImportFromPreviousMesoMessage(): void
	{
		msgDlgImport.show(-1);
	}
} //Page
