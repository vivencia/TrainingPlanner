import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import com.vivenciasoftware.qmlcomponents

Frame {
	id: paneSplit
	required property int mesoId
	required property int mesoIdx
	required property string splitLetter
	required property DBMesoSplitModel splitModel
	required property Item parentItem

	property bool bAlreadyLoaded: false
	property int removalSecs: 0
	property string prevMesoName: ""
	property int prevMesoId: -1
	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false
	property bool bCanSwapPlan: false
	property string swappableLetter

	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)

	implicitWidth: windowWidth
	implicitHeight: splitLayout.height

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
				splitModel.removeExercise(idxToRemove);
				if (idxToRemove > 0)
					--idxToRemove;
				if (splitModel.count === 0)
					appendNewExerciseToDivision();
				splitModel.setCurrentRow(idxToRemove);
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
		message: qsTr("Import the exercises plan for training division <b>") + splitLetter +
						 qsTr("</b> from <b>") + prevMesoName + "</b>?"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: {
			appDB.pass_object(splitModel);
			appDB.loadSplitFromPreviousMeso(prevMesoId, splitLetter);
		}
	} //TPBalloonTip

	background: Rectangle {
		border.color: "transparent"
		radius: 5
	}

	ColumnLayout {
		id: splitLayout
		anchors.fill: parent

		Label {
			id: lblMain
			text: qsTr("Training Division ") + splitLetter
			Layout.alignment: Qt.AlignHCenter|Qt.AlignTop
			Layout.topMargin: 10
			Layout.maximumWidth: parent.width - 20
			font.bold: true
			font.pixelSize: AppSettings.fontSizeTitle
		}// Label lblMain

		Label {
			text: qsTr("Muscle groups trained in this division:")
			Layout.leftMargin: 5
			Layout.topMargin: 10
		}

		TextField {
			id: txtSplit
			text: mesoSplitModel.get(mesoIdx, splitLetter.charCodeAt(0) - "A".charCodeAt(0) + 2)
			font.pixelSize: AppSettings.fontSizeText
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 20
			font.bold: true

			onTextEdited: {
				mesoSplitModel.set(mesoIdx, splitLetter.charCodeAt(0) - "A".charCodeAt(0) + 2, text);
				exercisesListModel.makeFilterString(text);
				swappableLetter = appDB.checkIfSplitSwappable(splitLetter);
				bCanSwapPlan = swappableLetter !== "";
			}
		}

		ListView {
			id: lstSplitExercises
			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick
			contentHeight: totalHeight * 1.1 + 20//contentHeight: Essencial for the ScrollBars to work.
			property int totalHeight
			width: parent.width

			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.rightMargin: 10
			Layout.leftMargin: 5
			Layout.minimumHeight: 200

			ScrollBar.vertical: ScrollBar {
				id: vBar
				policy: ScrollBar.AsNeeded
				active: true; visible: lstSplitExercises.totalHeight > lstSplitExercises.height
			}

			model: splitModel

			delegate: SwipeDelegate {
				id: delegate
				spacing: 0
				padding: 0
				implicitWidth: Math.max(lstSplitExercises.width, listItem.implicitWidth)
				implicitHeight: listItem.height

				ColumnLayout {
					id: contentsLayout
					anchors.fill: parent
					spacing: 2

					TPRadioButton {
						id: optCurrentExercise
						text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>"
						textColor: "black"
						indicatorColor: "black"
						checked: index === splitModel.currentRow
						width: parent.width

						onClicked: splitModel.currentRow = index;

						RoundButton {
							id: btnMoveExerciseUp
							anchors.right: btnMoveExerciseDown.left
							anchors.verticalCenter: parent.verticalCenter
							height: 30
							width: 30
							padding: 5
							enabled: index > 0

							Image {
								source: "qrc:/images/"+darkIconFolder+"up.png"
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								height: 25
								width: 25
							}

							onClicked: splitModel.moveRow(index,index-1);
						}
						RoundButton {
							id: btnMoveExerciseDown
							anchors.right: parent.right
							anchors.verticalCenter: parent.verticalCenter
							height: 30
							width: 30
							padding: 5
							enabled: index < splitModel.count-1

							Image {
								source: "qrc:/images/"+darkIconFolder+"down.png"
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								height: 25
								width: 25
							}

							onClicked: splitModel.moveRow(index,index+1);
						}
					}

					TextField {
						id: txtExerciseName
						text: exerciseName
						wrapMode: Text.WordWrap
						readOnly: true
						Layout.leftMargin: 5
						Layout.minimumWidth: parent.width - 40
						Layout.maximumWidth: parent.width - 40
						width: parent.width - 40
						focus: true

						background: Rectangle {
							color: txtExerciseName.readOnly ? "transparent" : "white"
							border.color: txtExerciseName.readOnly ? "transparent" : "black"
							radius: 5
						}

						//Alphanumeric keyboard
						Keys.onReturnPressed: cboSetType.forceActiveFocus();

						onPressed: (mouse) => {
							if (!readOnly) {
								mouse.accepted = true;
								forceActiveFocus();
							}
							else
								mouse.accepted = false; //relay the signal to the delegate
						}

						onPressAndHold: (mouse) => {
							mouse.accepted = false;
						}

						onReadOnlyChanged: {
							if (!readOnly) {
								cursorPosition = text.length;
								exerciseName = text;
							}
							else {
								cursorPosition = 0;
								ensureVisible(0);
							}
							requestSimpleExercisesList(paneSplit, !readOnly, setType === 4, 0);
						}

						onActiveFocusChanged: {
							if (activeFocus) {
								cursorPosition = text.length;
							}
							else {
								readOnly = true;
								cursorPosition = 0;
							}
						}

						RoundButton {
							id: btnEditExercise
							padding: 10
							anchors.left: txtExerciseName.right
							anchors.top: txtExerciseName.top
							height: 25
							width: 25
							enabled: index === splitModel.currentRow

							Image {
								source: "qrc:/images/"+darkIconFolder+"edit.png"
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								fillMode: Image.PreserveAspectFit
								height: 20
								width: 20
							}

							onClicked: txtExerciseName.readOnly = !txtExerciseName.readOnly;
						} //ToolButton btnEditExercise

						RoundButton {
							id: btnClearText
							anchors.left: btnEditExercise.left
							anchors.top: btnEditExercise.bottom
							height: 20
							width: 20
							visible: !txtExerciseName.readOnly

							Image {
								source: "qrc:/images/"+darkIconFolder+"edit-clear.png"
								anchors.fill: parent
								height: 20
								width: 20
							}
							onClicked: {
								txtExerciseName.clear();
								txtExerciseName.forceActiveFocus();
							}
						}
					} //TextField

					RowLayout {
						Layout.leftMargin: 5
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
								if (txtExerciseName.text === qsTr("Choose exercise..."))
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
							width: listItem.width/2
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
							width: listItem.width/2
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
					color: splitModel.currentRow === index ? primaryLightColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
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
						source: "qrc:/images/"+lightIconFolder+"remove.png"
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
						color: "white"
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
	} //ColumnLayout

	function init() {
		if (!bAlreadyLoaded) {
			if (splitModel.count === 0) {
				prevMesoId = mesocyclesModel.getPreviousMesoId(mesoId);
				if (prevMesoId >= 0) {
					if (appDB.mesoHasPlan(prevMesoId, splitLetter)) {
						prevMesoName = mesocyclesModel.getMesoInfo(prevMesoId, DBMesocyclesModel.mesoNameRole);
						msgDlgImport.show((mainwindow.height - msgDlgImport.height) / 2)
						splitModel.currentRow = 0;
					}
				}
				else
					appendNewExerciseToDivision();
			}
			exercisesListModel.makeFilterString(txtSplit.text);
			bAlreadyLoaded = true;
			swappableLetter = appDB.checkIfSplitSwappable(splitLetter);
			bCanSwapPlan = swappableLetter !== "";
		}
	}

	//Each layout row(9) * 32(height per row) + 32(extra space)
	function setListItemHeight(item, settype) {
		item.height = settype !== 4 ? 320 : 450;
	}

	function changeExercise(name, nsets, nreps, nweight, multiplesel_opt) {
		if (bListRequestForExercise1) {
			splitModel.exerciseName1 = name;
			splitModel.setsReps1 = nreps;
			splitModel.setsWeight1 = nweight;
			bListRequestForExercise1 = false;
			requestSimpleExercisesList(null, false, false, 0);
		}
		else if (bListRequestForExercise2) {
			splitModel.exerciseName2 = name;
			splitModel.setsReps2 = nreps;
			splitModel.setsWeight2 = nweight;
			bListRequestForExercise2 = false;
			requestSimpleExercisesList(null, false, false, 0);
		}
		else
			splitModel.changeExercise(name, nsets, nreps, nweight, multiplesel_opt);
	}

	function appendNewExerciseToDivision() {
		splitModel.addExercise(qsTr("Choose exercise..."), 0, "4", "12", "20");
		lstSplitExercises.currentIndex = splitModel.currentRow;
		lstSplitExercises.positionViewAtIndex(splitModel.currentRow, ListView.Center);
	}
} //Page
