import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import com.vivenciasoftware.qmlcomponents

Frame {
	id: paneSplit
	objectName: "mesoSplitPlanner"

	required property int mesoId
	required property int mesoIdx
	required property DBMesoSplitModel splitModel

	property bool bAlreadyLoaded: false
	property int removalSecs: 0
	property string prevMesoName: ""
	property int prevMesoId: -1
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

	Timer {
		id: undoTimer
		interval: 1000
		property int idxToRemove

		onTriggered: {
			if ( removalSecs === 0 ) {
				undoTimer.stop();
				removeExercise(idxToRemove);
			}
			else {
				removalSecs = removalSecs - 1000;
				start();
			}
		}

		function init(idxtoremove) {
			idxToRemove = idxtoremove;
			start();
		}
	} //Timer

	TPBalloonTip {
		id: msgDlgImport
		title: qsTr("Import Exercises Plan?")
		message: qsTr("Import the exercises plan for training division <b>") + splitModel.splitLetter +
						 qsTr("</b> from <b>") + prevMesoName + "</b>?"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: appDB.loadSplitFromPreviousMeso(prevMesoId, splitModel);
	} //TPBalloonTip

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise?")
		message: exerciseName + qsTr("This action cannot be undone.")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		onButton1Clicked: removeExercise(idxToRemove);

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
		text: qsTr("Training Division ") + splitModel.splitLetter
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
		text: splitModel.muscularGroup
		width: parent.width - 20

		anchors {
			top: lblGroups.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			bottomMargin: 10
		}

		onEditingFinished: {
			splitModel.muscularGroup = text;
			exercisesListModel.makeFilterString(text);
			swappableLetter = appDB.checkIfSplitSwappable(splitModel.splitLetter);
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
			}

			model: splitModel

			delegate: SwipeDelegate {
				id: delegate
				spacing: 0
				padding: 0
				implicitWidth: lstSplitExercises.width
				implicitHeight: listItem.height

				ColumnLayout {
					id: contentsLayout
					anchors.fill: parent
					spacing: 2

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
								rightMargin: -10
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
								rightMargin: 20
								verticalCenter: parent.verticalCenter
							}

							onClicked: splitModel.moveRow(index,index+1);
						}
					}

					ExerciseNameField {
						id: txtExerciseName
						text: exerciseName
						Layout.leftMargin: 5
						Layout.minimumWidth: parent.width - 20
						Layout.maximumWidth: parent.width - 20

						//Alphanumeric keyboard
						Keys.onReturnPressed: cboSetType.forceActiveFocus();
						onExerciseChanged: (new_text) => exerciseName = new_text;
						onItemClicked: splitModel.currentRow = index;

						onRemoveButtonClicked: {
							splitModel.currentRow = index;
							msgDlgRemove.init(index, 0);
						}

						onEditButtonClicked: {
							splitModel.currentRow = index;
							requestSimpleExercisesList(paneSplit, !readOnly, setType === 4, 0);
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
					} //ExerciseNameField

					RowLayout {
						Layout.leftMargin: 5
						Layout.topMargin: 5
						Layout.fillWidth: true

						Label {
							text: qsTr("Set Type:")
							wrapMode: Text.WordWrap
							Layout.minimumWidth: listItem.width/2
						}
						TPComboBox {
							id: cboSetType
							currentIndex: setType
							enabled: index === splitModel.currentRow
							Layout.rightMargin: 5

							onActivated: (index) => {
								setListItemHeight(lstSplitExercises.currentItem, index);
								setType = index;
								txtNSets.forceActiveFocus();
								if (setType !== 4)
									exerciseName = (qsTr("Choose exercise..."));
								else
									exerciseName = (qsTr("Choose exercises..."));
							}
						}
					}

					RowLayout {
						Layout.leftMargin: 5
						Layout.fillWidth: true

						Label {
							text: qsTr("Number of Sets:")
							wrapMode: Text.WordWrap
							Layout.minimumWidth: listItem.width/2
						}
						SetInputField {
							id: txtNSets
							text: setsNumber
							type: SetInputField.Type.SetType
							availableWidth: listItem.width / 3
							showLabel: false
							enabled: index === splitModel.currentRow

							onValueChanged: (str) => setsNumber = str;

							onEnterOrReturnKeyPressed: {
								if (txtNSubsets.visible)
									txtNSubsets.forceActiveFocus();
								else
									txtNReps.forceActiveFocus();
							}
						}
					}

					RowLayout {
						visible: setType === 2 || setType === 3 || setType === 5
						Layout.leftMargin: 5
						Layout.fillWidth: true

						Label {
							text: qsTr("Number of Subsets:")
							wrapMode: Text.WordWrap
							Layout.minimumWidth: listItem.width/2
						}
						SetInputField {
							id: txtNSubsets
							text: setsSubsets
							type: SetInputField.Type.SetType
							availableWidth: listItem.width / 3
							showLabel: false
							enabled: index === splitModel.currentRow

							onValueChanged: (str) => setsSubsets = str;

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
						Layout.leftMargin: 5
						Layout.fillWidth: true
						Layout.topMargin: 10
						Layout.bottomMargin: 10

						Label {
							text: exerciseName1
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
							text: exerciseName2
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

					RowLayout {
						visible: cboSetType.currentIndex !== 4
						Layout.leftMargin: 5
						Layout.fillWidth: true

						Label {
							text: qsTr("Baseline number of reps:")
							wrapMode: Text.WordWrap
							Layout.maximumWidth: listItem.width/2
							Layout.minimumWidth: listItem.width/2
						}

						SetInputField {
							id: txtNReps
							text: setsReps1
							type: SetInputField.Type.RepType
							availableWidth: listItem.width/3
							showLabel: false
							enabled: index === splitModel.currentRow
							Layout.rightMargin: 5

							onValueChanged: (str) => setsReps1 = str;
							onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Baseline number of reps:")
						wrapMode: Text.WordWrap
						visible: cboSetType.currentIndex === 4
						Layout.leftMargin: 5
					}

					RowLayout {
						visible: cboSetType.currentIndex === 4

						SetInputField {
							id: txtNReps1
							text: setsReps1
							type: SetInputField.Type.RepType
							availableWidth: listItem.width/3
							showLabel: false
							enabled: index === splitModel.currentRow
							Layout.alignment: Qt.AlignCenter
							Layout.leftMargin: listItem.width/6

							onValueChanged: (str) => setsReps1 = str;
							onEnterOrReturnKeyPressed: txtNReps2.forceActiveFocus();
						}

						SetInputField {
							id: txtNReps2
							text: setsReps2
							type: SetInputField.Type.RepType
							availableWidth: listItem.width/3
							showLabel: false
							enabled: index === splitModel.currentRow
							Layout.alignment: Qt.AlignRight
							Layout.rightMargin: listItem.width/6

							onValueChanged: (str) => setsReps2 = str;
							onEnterOrReturnKeyPressed: txtNWeight1.forceActiveFocus();
						}
					} //RowLayout

					RowLayout {
						visible: cboSetType.currentIndex !== 4
						Layout.leftMargin: 5
						Layout.fillWidth: true

						Label {
							text: qsTr("Baseline weight ") + AppSettings.weightUnit + ":"
							wrapMode: Text.WordWrap
							Layout.minimumWidth: listItem.width/2
						}

						SetInputField {
							id: txtNWeight
							text: setsWeight1
							type: SetInputField.Type.WeightType
							availableWidth: listItem.width / 3
							showLabel: false
							enabled: index === splitModel.currentRow
							visible: cboSetType.currentIndex !== 4

							onValueChanged: (str) => setsWeight1 = str;
						}
					}

					Label {
						text: qsTr("Baseline weight ") + AppSettings.weightUnit + ":"
						wrapMode: Text.WordWrap
						visible: cboSetType.currentIndex === 4
					}

					RowLayout {
						Layout.row: 8
						Layout.column: 0
						Layout.columnSpan: 2
						visible: cboSetType.currentIndex === 4

						SetInputField {
							id: txtNWeight1
							text: setsWeight1
							type: SetInputField.Type.WeightType
							availableWidth: listItem.width/3
							showLabel: false
							enabled: index === splitModel.currentRow
							Layout.alignment: Qt.AlignCenter
							Layout.leftMargin: listItem.width/6

							onValueChanged: (str) => setsWeight1 = str;
							onEnterOrReturnKeyPressed: txtNWeight2.forceActiveFocus();
						}

						SetInputField {
							id: txtNWeight2
							text: setsWeight2
							type: SetInputField.Type.WeightType
							availableWidth: listItem.width/3
							showLabel: false
							enabled: index === splitModel.currentRow
							Layout.alignment: Qt.AlignRight
							Layout.rightMargin: listItem.width/6

							onValueChanged: (str) => setsWeight2 = str;
						}
					} //RowLayout

					SetNotesField {
						info: qsTr("Set instructions:")
						text: setsNotes
						Layout.leftMargin: 5
						Layout.fillWidth: true

						onEditFinished: (new_text) => setsNotes = new_text;
					}
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
					radius: 5
					color: splitModel.currentRow === index ? AppSettings.primaryLightColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
				}

				Component.onCompleted: lstSplitExercises.totalHeight += height;
				onClicked: splitModel.currentRow = index;

				swipe.right: Rectangle {
					id: rec
					width: parent.width
					height: parent.height
					color: SwipeDelegate.pressed ? "#555" : "#666"
					radius: 5
					opacity: Math.abs(delegate.swipe.position)
					z: 2

					Image {
						source: "qrc:/images/"+AppSettings.iconFolder+"remove.png"
						anchors.left: parent.left
						anchors.leftMargin: 10
						anchors.verticalCenter: parent.verticalCenter
						width: 20
						height: 20
						opacity: 2 * -delegate.swipe.position
						z:3
					}

					Label {
						text: qsTr("Removing in " + removalSecs/1000 + "s")
						color: AppSettings.fontColor
						padding: 5
						anchors.fill: parent
						anchors.leftMargin: 40
						horizontalAlignment: Qt.AlignLeft
						verticalAlignment: Qt.AlignVCenter
						opacity: delegate.swipe.complete ? 1 : 0
						Behavior on opacity { NumberAnimation { } }
						z:2
					}

					SwipeDelegate.onClicked: delegate.swipe.close();
					SwipeDelegate.onPressedChanged: undoTimer.stop();
				} //swipe.right

				swipe.onCompleted: {
					removalSecs = 4000;
					undoTimer.init(index);
				}
			} //delegate: SwipeDelegate
		} //ListView

	function init() {
		if (!bAlreadyLoaded) {
			if (splitModel.count === 0) {
				prevMesoId = mesocyclesModel.getPreviousMesoId(mesoId);
				if (prevMesoId >= 0) {
					if (appDB.mesoHasPlan(prevMesoId, splitModel.splitLetter)) {
						prevMesoName = mesocyclesModel.getMesoInfo(prevMesoId, 1);
						msgDlgImport.show((mainwindow.height - msgDlgImport.height) / 2)
						splitModel.currentRow = 0;
					}
					else
						appendNewExerciseToDivision();
				}
				else
					appendNewExerciseToDivision();
			}
			exercisesListModel.makeFilterString(txtGroups.text);
			bAlreadyLoaded = true;
			swappableLetter = appDB.checkIfSplitSwappable(splitModel.splitLetter);
			bCanSwapPlan = swappableLetter !== "";
		}
	}

	//Each layout row(10) * 32(height per row) + 30(extra space)
	function setListItemHeight(item, settype) {
		item.height = settype !== 4 ? 360 : 450;
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
