import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../"
import "../ExercisesAndSets"
import "../TPWidgets"

import com.vivenciasoftware.qmlcomponents

Frame {
	id: paneSplit
	objectName: "mesoSplitPlanner"

	required property TPPage parentItem
	required property DBMesoSplitModel splitModel

	property bool bAlreadyLoaded: false
	property int removalSecs: 0
	property string prevMesoName: ""
	property int prevMesoId: -2
	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false
	property bool bCanSwapPlan: false
	property string swappableLetter: ""

	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)

	width: windowWidth
	height: windowHeight - 55
	padding: 0
	spacing: 0
	Layout.leftMargin: 5

	TPBalloonTip {
		id: msgDlgImport
		title: qsTr("Import Exercises Plan?")
		message: qsTr("Import the exercises plan for training division <b>") + splitModel.splitLetter() +
						 qsTr("</b> from <b>") + prevMesoName + "</b>?"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "remove.png"
		parentPage: parentItem

		onButton1Clicked: appDB.loadSplitFromPreviousMeso(prevMesoId, splitModel);
	} //TPBalloonTip


	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise?")
		message: exerciseName + qsTr("This action cannot be undone.")
		imageSource: "remove.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		onButton1Clicked: removeExercise(idxToRemove);
		parentPage: parentItem

		property int idxToRemove
		property string exerciseName

		function init(idxtoremove: int, pos: int) {
			idxToRemove = idxtoremove;
			splitModel.currentRow = idxtoremove;
			exerciseName = splitModel.exerciseName;
			show(pos);
		}
	} //TPBalloonTip

	background: Rectangle {
		border.color: "transparent"
		radius: 5
	}

	Label {
		id: lblMain
		text: qsTr("Training Division ") + splitModel.splitLetter()
		horizontalAlignment: Text.AlignHCenter
		width: parent.width
		font.bold: true
		font.pointSize: AppSettings.fontSizeTitle

		anchors {
			top: parent.top
			topMargin: 10
			left: parent.left
			bottomMargin: 10
		}
	}// Label lblMain

	Label {
		id: lblGroups
		text: qsTr("Muscle groups trained in this division:")
		width: parent.width - 20
		anchors {
			top: lblMain.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
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
			bottomMargin: 10
		}

		onEditingFinished: {
			splitModel.setMuscularGroup(text);
			exercisesListModel.makeFilterString(text);
			swappableLetter = appDB.checkIfSplitSwappable(splitModel.splitLetter());
			bCanSwapPlan = swappableLetter !== "";
			itemManager.changeMuscularGroup(splitModel);
		}
	}

		ListView {
			id: lstSplitExercises
			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick
			contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
			property int totalHeight

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
				active: true; visible: lstSplitExercises.contentHeight > lstSplitExercises.height

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

			delegate: SwipeDelegate {
				id: delegate
				spacing: 10
				padding: 0
				implicitWidth: lstSplitExercises.width
				implicitHeight: listItem.height

				ColumnLayout {
					id: contentsLayout
					anchors.fill: parent
					spacing: 5

					TPRadioButton {
						id: optCurrentExercise
						text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>"
						textColor: "black"
						checked: index === splitModel.currentRow
						width: parent.width

						onClicked: splitModel.currentRow = index;

						TPRoundButton {
							id: btnMoveExerciseUp
							imageName: "up.png"
							height: 30
							width: 30
							padding: 5
							enabled: index > 0
							anchors {
								right: btnMoveExerciseDown.left
								rightMargin: -5
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitModel.moveRow(index,index-1);
						}
						TPRoundButton {
							id: btnMoveExerciseDown
							imageName: "down.png"
							height: 30
							width: 30
							padding: 0
							enabled: index < splitModel.count-1
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
						Layout.leftMargin: 5
						Layout.minimumWidth: parent.width - 20
						Layout.maximumWidth: parent.width - 20
						Layout.maximumHeight: 70

						//Alphanumeric keyboard
						Keys.onReturnPressed: cboSetType.forceActiveFocus();
						onExerciseChanged: (new_text) => {
							splitModel.setExerciseName(index, new_text);
							if (cboSetType.currentIndex === 4) {
								lblExercise1.text = splitModel.exerciseName1(index);
								lblExercise2.text = splitModel.exerciseName2(index);
							}
						}
						onItemClicked: splitModel.currentRow = index;

						onRemoveButtonClicked: {
							splitModel.currentRow = index;
							msgDlgRemove.init(index, 0);
						}

						onEditButtonClicked: {
							splitModel.currentRow = index;
							requestSimpleExercisesList(paneSplit, !readOnly, cboSetType.currentIndex === 4, 0);
						}

						onMousePressed: (mouse) => {
							if (!readOnly) {
								mouse.accepted = true;
								forceActiveFocus();
							}
							else
								mouse.accepted = false; //relay the signal to the delegate
						}

						onMousePressAndHold: (mouse) => mouse.accepted = false;

						Component.onCompleted: {
							splitModel.exerciseNameChanged.connect(function () { text = splitModel.exerciseName(index); } );
						}
					} //ExerciseNameField

					SetNotesField {
						info: splitModel.columnLabel(2)
						text: splitModel.setsNotes(index)
						Layout.leftMargin: 5
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
						height: 256
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

								TPRoundButton {
									id: btnAddSet
									imageName: "plus.png"
									height: 30
									width: 30
									z:2

									onClicked: splitModel.addSet(index)

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
										anchors.fill: parent
										model: splitModel.nbrSets === 1 ? splitModel.setsNumber(index) : splitModel.setsNumber(index)

										TabButton {
											id: tabbutton
											text: qsTr("Set # ") + parseInt(index + 1)
											height: setsTabBar.height
											checkable: true
											checked: index === setsTabBar.currentIndex
											width: 70

											contentItem: IconLabel {
												spacing: tabbutton.spacing
												display: AbstractButton.TextOnly
												text: tabbutton.text
												font.pointSize: AppSettings.fontSizeLists
												color: AppSettings.fontColor
											}

											background: Rectangle {
												border.color: AppSettings.fontColor
												radius: 6
												opacity: 0.8
												color: enabled ? checked ? AppSettings.primaryDarkColor : AppSettings.primaryColor : "gray"
											}

											onClicked: splitModel.workingSet = index;
										}
									}
								} //setsTabBar

								TPRoundButton {
									id: btnDelSet
									imageName: "minus.png"
									height: 30
									width: 30
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

								Label {
									text: splitModel.columnLabel(3)
									wrapMode: Text.WordWrap
									Layout.minimumWidth: listItem.width/2 - 40
									Layout.maximumWidth: listItem.width/2 - 40
								}
								TPComboBox {
									id: cboSetType
									enabled: index === splitModel.currentRow
									currentIndex: splitModel.workingSet === setsTabBar.currentIndex ? splitModel.setType(index) : splitModel.setType(index)
									Layout.rightMargin: 5

									onActivated: (cboIndex) => {
										setListItemHeight(lstSplitExercises.currentItem, cboIndex);
										splitModel.setSetType(index, cboIndex);
										if (cboIndex !== 4)
											txtExerciseName.text = (qsTr("Choose exercise..."));
										else
											txtExerciseName.text = (qsTr("Choose exercises..."));
									}
								}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 2 || cboSetType.currentIndex === 3 || cboSetType.currentIndex === 5
								Layout.leftMargin: 20
								Layout.rightMargin: 20
								Layout.fillWidth: true

								Label {
									text: splitModel.columnLabel(4)
									wrapMode: Text.WordWrap
									Layout.minimumWidth: listItem.width/2
								}
								SetInputField {
									id: txtNSubsets
									text: splitModel.setsSubsets(index)
									type: SetInputField.Type.SetType
									availableWidth: listItem.width / 3
									showLabel: false

									onValueChanged: (str) => splitModel.setSetsSubsets(index, str);

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function() {
											if (!visible) return;
											if (index === splitModel.currentRow) {
												text = splitModel.setsSubsets(index);
											}
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
								visible: cboSetType.currentIndex === 4
								Layout.leftMargin: 20
								Layout.fillWidth: true
								Layout.topMargin: 5
								Layout.bottomMargin: 5

								Label {
									id: lblExercise1
									text: splitModel.exerciseName1(index)
									font.bold: true
									wrapMode: Text.WordWrap
									width: listItem.width*0.5-10
									Layout.alignment: Qt.AlignCenter
									Layout.maximumWidth: width
									Layout.minimumWidth: width

									MouseArea {
										anchors.fill: parent
										onClicked: {
											splitModel.currentRow = index;
											bListRequestForExercise1 = true;
											requestSimpleExercisesList(paneSplit, true, false, 0);
										}
									}
								}

								Label {
									id: lblExercise2
									text: splitModel.exerciseName2(index)
									font.bold: true
									wrapMode: Text.WordWrap
									width: listItem.width*0.5-10
									Layout.alignment: Qt.AlignCenter
									Layout.maximumWidth: width
									Layout.minimumWidth: width

									MouseArea {
										anchors.fill: parent
										onClicked: {
											splitModel.currentRow = index;
											bListRequestForExercise2 = true;
											requestSimpleExercisesList(paneSplit, true, false, 0);
										}
									}
								}
							}

							SetInputField {
								id: txtNReps
								text: splitModel.setsReps1(index)
								type: SetInputField.Type.RepType
								availableWidth: listItem.width - 40
								visible: cboSetType.currentIndex !== 4
								Layout.leftMargin: 20
								Layout.rightMargin: 20

								onValueChanged: (str) => splitModel.setSetsReps1(index, str);
								onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
								Component.onCompleted: {
										splitModel.workingSetChanged.connect(function() {
											if (!visible) return;
											if (index === splitModel.currentRow) {
												text = splitModel.setsReps1(index);
											}
										});
									}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 4

								SetInputField {
									id: txtNReps1
									text: splitModel.setsReps1(index)
									type: SetInputField.Type.RepType
									availableWidth: listItem.width/2 + 10
									Layout.alignment: Qt.AlignLeft
									Layout.leftMargin: 20

									onValueChanged: (str) => splitModel.setSetsReps1 (index, str);
									onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function() {
											if (!visible) return;
											if (index === splitModel.currentRow) {
												text = splitModel.setsReps1(index);
											}
										});
									}
								}

								SetInputField {
									id: txtNReps2
									text: splitModel.setsReps2(index)
									type: SetInputField.Type.RepType
									availableWidth: listItem.width/3
									showLabel: false

									onValueChanged: (str) => splitModel.setSetsReps2(index, str);
									onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function() {
											if (!visible) return;
											if (index === splitModel.currentRow) {
												text = splitModel.setsReps2(index);
											}
										});
									}
								}
							} //RowLayout

							SetInputField {
								id: txtNWeight
								text: splitModel.setsWeight1(index)
								type: SetInputField.Type.WeightType
								availableWidth: listItem.width - 40
								visible: cboSetType.currentIndex !== 4
								Layout.leftMargin: 20
								Layout.rightMargin: 20

								onValueChanged: (str) => splitModel.setSetsWeight1(index, str);

								Component.onCompleted: {
									splitModel.workingSetChanged.connect(function() {
										if (!visible) return;
										if (index === splitModel.currentRow) {
											text = splitModel.setsWeight1(index);
										}
									});
								}
							}

							RowLayout {
								visible: cboSetType.currentIndex === 4
								Layout.fillWidth: true

								SetInputField {
									id: txtNWeight1
									text: splitModel.setsWeight1(index)
									type: SetInputField.Type.WeightType
									availableWidth: listItem.width/2 + 10
									Layout.alignment: Qt.AlignCenter
									Layout.leftMargin: 20

									onValueChanged: (str) => splitModel.setSetsWeight1(index, str);
									onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function() {
											if (!visible) return;
											if (index === splitModel.currentRow) {
												text = splitModel.setsWeight1(index);
											}
										});
									}
								}

								SetInputField {
									id: txtNWeight2
									text: splitModel.setsWeight2(index)
									type: SetInputField.Type.WeightType
									showLabel: false
									availableWidth: listItem.width/3
									Layout.alignment: Qt.AlignRight
									Layout.rightMargin: listItem.width/6

									onValueChanged: (str) => splitModel.setSetsWeight2(index, str);

									Component.onCompleted: {
										splitModel.workingSetChanged.connect(function() {
											if (!visible) return;
											if (index === splitModel.currentRow) {
												text = splitModel.setsWeight2(index);
											}
										});
									}
								}
							} //RowLayout
						} //ColumnLayout
					} //Pane
				} //ColumnLayout

				contentItem: Rectangle {
					id: listItem
					width: lstSplitExercises.width - 10
					border.color: "transparent"
					color: "transparent"
					radius: 5
					Component.onCompleted: setListItemHeight(this, cboSetType.currentIndex);
				}

				background: Rectangle {
					id:	backgroundColor
					radius: 6
					color: splitModel.currentRow === index ? AppSettings.primaryLightColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
				}

				Component.onCompleted: lstSplitExercises.totalHeight += height;
				onClicked: splitModel.currentRow = index;
			} //delegate: SwipeDelegate
		} //ListView

	function init() {
		if (!bAlreadyLoaded) {
			if (prevMesoId == -1) //splitModel is empty and there is no previous mesocycle
				appendNewExerciseToDivision();
			else if (prevMesoId >= 0) { //splitModel is empty and there is a previous mesocycle. Check if it has a plan for the letter
				if (appDB.mesoHasPlan(prevMesoId, splitModel.splitLetter())) {
					prevMesoName = mesocyclesModel.getMesoInfo(prevMesoId, 1);
					msgDlgImport.show((mainwindow.height - msgDlgImport.height) / 2)
				}
				else
					appendNewExerciseToDivision();
			}
			splitModel.currentRow = 0;
			exercisesListModel.makeFilterString(txtGroups.text);
			bAlreadyLoaded = true;
			swappableLetter = appDB.checkIfSplitSwappable(splitModel.splitLetter());
			bCanSwapPlan = swappableLetter !== "";
		}
	}

	function setScrollBarPosition(pos) {
		if (pos === 0)
			vBar.setPosition(0);
		else
			vBar.setPosition(pos - vBar.size/2);
	}

	//Each layout row(10) * 32(height per row) + 20(extra space)
	function setListItemHeight(item, settype) {
		var nheight;
		switch (settype) {
			case 4:
				nheight = 360; break;
			case 3:
			case 5:
				nheight = 340; break;
			default:
				nheight = 300; break;
		}
		item.height = nheight;
	}

	function removeExercise(idx: int) {
		splitModel.removeExercise(idx);
		if (idx > 0)
			--idx;
		if (splitModel.count === 0)
			appendNewExerciseToDivision();
		splitModel.setCurrentRow(idx);
	}

	function changeExercise(fromList: bool) {
		if (bListRequestForExercise1) {
			splitModel.exerciseName1 = exercisesListModel.selectedEntriesValue(0, 1) + " - " + exercisesListModel.selectedEntriesValue(0, 2);
			splitModel.setsNumber = exercisesListModel.selectedEntriesValue(0, 4);
			splitModel.setsReps1 = exercisesListModel.selectedEntriesValue(0, 5);
			splitModel.setsWeight1 = exercisesListModel.selectedEntriesValue(0, 6);
			bListRequestForExercise1 = false;
			requestSimpleExercisesList(null, false, false, 0);
		}
		else if (bListRequestForExercise2) {
			splitModel.exerciseName2 = exercisesListModel.selectedEntriesValue(0, 1) + " - " + exercisesListModel.selectedEntriesValue(0, 2);
			splitModel.setsNumber = exercisesListModel.selectedEntriesValue(0, 4);
			splitModel.setsReps2 = exercisesListModel.selectedEntriesValue(0, 5);
			splitModel.setsWeight2 = exercisesListModel.selectedEntriesValue(0, 6);
			bListRequestForExercise2 = false;
			requestSimpleExercisesList(null, false, false, 0);
		}
		else
			splitModel.changeExercise(exercisesListModel);
	}

	function appendNewExerciseToDivision() {
		splitModel.addExercise(qsTr("Choose exercise..."), 0, "4", "12", "20");
		lstSplitExercises.currentIndex = splitModel.currentRow;
		lstSplitExercises.positionViewAtIndex(splitModel.currentRow, ListView.Center);
	}
} //Page
