import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	property DBTrainingDayModel tDayModel
	property int exerciseIdx

	property int setNbr: 0
	property string nSets
	property string nReps
	property string nWeight
	property bool bListRequestForExercise1: false
	property bool bListRequestForExercise2: false

	signal setAdded(int objidx, var setObject)
	signal requestSimpleExercisesList(Item requester, var bVisible, var bMultipleSelection, int id)
	signal requestFloatingButton(var exerciseidx, var settype)

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise")
		message: tDayModel.exerciseName(exerciseIdx) + qsTr("? This action cannot be undone.")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: itemManager.removeExerciseObject(exerciseIdx);
	} //TPBalloonTip

	Frame {
		id: paneExercise
		property bool shown: tDayModel.setsNumber(exerciseIdx) === 0
		visible: height > 0
		height: shown ? implicitHeight : txtExerciseName.height + 15
		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutBack
			}
		}
		clip: true
		padding: 0
		z: 0

		background: Rectangle {
			color: exerciseIdx % 2 === 0 ? listEntryColor1 : listEntryColor2
			border.color: "transparent"
			opacity: 0.8
			radius: 5
		}

		Layout.fillWidth: true
		implicitHeight: layoutMain.implicitHeight + 10
		implicitWidth: parent.width
		width: windowWidth - 10

		RoundButton {
			id: btnMoveExerciseUp
			anchors.left: parent.left
			anchors.leftMargin: 0
			anchors.top: parent.top
			anchors.topMargin: -15
			height: 30
			width: 30
			padding: 5
			enabled: exerciseIdx > 0

			Image {
				source: "qrc:/images/"+darkIconFolder+"up.png"
				asynchronous: true
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter
				height: 25
				width: 25
			}

			onClicked: moveExercise(true, true);
		}
		RoundButton {
			id: btnMoveExerciseDown
			anchors.left: parent.left
			anchors.leftMargin: 20
			anchors.top: parent.top
			anchors.topMargin: -15
			height: 30
			width: 30
			padding: 5
			enabled: exerciseIdx < tDayModel.exerciseCount-1

			Image {
				source: "qrc:/images/"+darkIconFolder+"down.png"
				asynchronous: true
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenter: parent.horizontalCenter
				height: 25
				width: 25
			}

			onClicked: moveExercise(false, true);
		}

		ColumnLayout {
			id: layoutMain
			objectName: "exerciseSetsLayout"
			anchors.fill: parent
			spacing: 0

			TextField {
				id: txtExerciseName
				text: tDayModel.exerciseName(exerciseIdx)
				font.bold: true
				font.pointSize: AppSettings.fontSizeText
				readOnly: true
				wrapMode: Text.WordWrap
				width: windowWidth - 110
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				height: 60
				Layout.leftMargin: 45
				Layout.rightMargin: 5
				Layout.topMargin: 0
				z: 1

				background: Rectangle {
					color: txtExerciseName.readOnly ? "transparent" : "white"
					border.color: txtExerciseName.readOnly ? "transparent" : "black"
					radius: 5
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					btnEditExercise.clicked();
					cboSetType.forceActiveFocus();
				}

				onTextChanged: {
					if (readOnly)
						ensureVisible(0);
				}

				onActiveFocusChanged: {
					if (activeFocus)
						cursorPosition = text.length;
					else {
						readOnly = false;
						tDayModel.setExerciseName1(text, exerciseIdx);
						cursorPosition = 0;
						ensureVisible(0);
					}
				}

				Label {
					id: lblExerciseNumber
					text: parseInt(exerciseIdx + 1) + ":"
					font.pointSize: AppSettings.fontSizeText
					anchors.right: txtExerciseName.left
					anchors.verticalCenter: txtExerciseName.verticalCenter
					width: 20
					padding: 2
				}

				RoundButton {
					id: btnFoldIcon
					anchors.right: lblExerciseNumber.left
					anchors.verticalCenter: txtExerciseName.verticalCenter
					anchors.topMargin: 20
					height: 25
					width: 25
					padding: 5

					Image {
						source: paneExercise.shown ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
						asynchronous: true
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
						height: 20
						width: 20
					}
					onClicked: paneExerciseShowHide()
					z: 1
				}

				RoundButton {
					id: btnRemoveExercise
					anchors.left: txtExerciseName.right
					anchors.top: txtExerciseName.top
					height: 25
					width: 25
					padding: 5
					z: 2

					Image {
						source: "qrc:/images/"+darkIconFolder+"remove.png"
						asynchronous: true
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
						height: 20
						width: 20
					}

					onClicked: msgDlgRemove.show(exerciseItem.y)
				} //btnRemoveExercise

				RoundButton {
					id: btnEditExercise
					anchors.left: btnRemoveExercise.right
					anchors.top: txtExerciseName.top
					height: 25
					width: 25
					padding: 5
					visible: cboSetType.currentIndex !== 4
					z: 2

					Image {
						source: "qrc:/images/"+darkIconFolder+"edit.png"
						asynchronous: true
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
						height: 20
						width: 20
					}

					onClicked: {
						txtExerciseName.readOnly = !txtExerciseName.readOnly;
						requestSimpleExercisesList(exerciseItem, !txtExerciseName.readOnly, cboSetType.currentIndex === 4, 1);
					}
				}

				RoundButton {
					id: btnClearText
					anchors.left: btnEditExercise.left
					anchors.top: btnEditExercise.bottom
					height: 20
					width: 20
					visible: !txtExerciseName.readOnly

					Image {
						source: "qrc:/images/"+darkIconFolder+"edit-clear.png"
						asynchronous: true
						anchors.fill: parent
						height: 20
						width: 20
					}
					onClicked: {
						txtExerciseName.clear();
						txtExerciseName.forceActiveFocus();
					}
				}

				MouseArea {
					anchors.left: txtExerciseName.left
					anchors.right: txtExerciseName.right
					anchors.top: txtExerciseName.top
					anchors.bottom: txtExerciseName.bottom
					onClicked: paneExerciseShowHide()
					enabled: txtExerciseName.readOnly
					z:1
				}
			} //txtExerciseName

			SetInputField {
				id: txtNReps
				text: nReps
				Layout.topMargin: 10
				type: SetInputField.Type.RepType
				availableWidth: layoutMain.width / 2
				backColor: "transparent"
				borderColor: "transparent"

				onValueChanged:(str)=> nReps = str;
				onEnterOrReturnKeyPressed: txtNWeight.forceActiveFocus();
			}

			SetInputField {
				id: txtNWeight
				text: nWeight
				type: SetInputField.Type.WeightType
				availableWidth: layoutMain.width / 2
				backColor: "transparent"
				borderColor: "transparent"

				onValueChanged:(str)=> nWeight = str;
			}

			Label {
				text: qsTr("Set type: ")
				font.bold: true
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				Layout.bottomMargin: 10
				spacing: 1

				TPComboBox {
					id: cboSetType
					currentIndex: tDayModel.setType(0, exerciseIdx)

					onActivated: (index)=> {
						switch(index) {
							case 2: nSets = "1"; break; //DropSet
							case 3: nSets = "2"; break; //ClusterSet
							case 5: nSets = "3"; break; //MyoReps
							default: break;
						}
					}
				}

				SetInputField {
					id: txtNSets
					text: nSets
					type: SetInputField.Type.SetType
					availableWidth: layoutMain.width / 3
					alternativeLabels: ["","","",qsTr("sets #:")]
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str)=> nSets = str;
				}

				RoundButton {
					id: btnAddSet
					width: 30
					height: 30
					Layout.leftMargin: 20

					Image {
						source: "qrc:/images/"+darkIconFolder+"add-new.png";
						asynchronous: true
						height: 20
						width: 20
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
					}
					onClicked: {
						createSetObject(cboSetType.currentIndex, parseInt(txtNSets.text), nReps, nWeight);
						requestFloatingButton(exerciseIdx, cboSetType.currentIndex);
					}
				}
			} // RowLayout
		} // ColumnLayout layoutMain
	} //paneExercise

	function changeExercise(newname)
	{
		if (bListRequestForExercise1) {
			itemManager.changeSetsExerciseLabels(exerciseIdx, 1, newname);
			bListRequestForExercise1 = false;
		}
		else if (bListRequestForExercise2) {
			itemManager.changeSetsExerciseLabels(exerciseIdx, 2, newname);
			bListRequestForExercise2 = false;
		}
		else
			tDayModel.setExerciseName(newname, exerciseIdx);
		txtExerciseName.text = tDayModel.exerciseName(exerciseIdx);
		requestSimpleExercisesList(null, false, false, 1);
	}

	function changeExercise1() {
		bListRequestForExercise1 = true;
		requestSimpleExercisesList(exerciseItem, true, false, 1);
	}

	function changeExercise2() {
		bListRequestForExercise2 = true;
		requestSimpleExercisesList(exerciseItem, true, false, 1);
	}

	function moveExercise(up: bool, cxx_cal: bool) {
		if (cxx_cal)
			itemManager.moveExercise(exerciseIdx, up ? --exerciseIdx : ++exerciseIdx);
		else {
			if (up) --exerciseIdx
			else ++exerciseIdx;
		}

		lblExerciseNumber.text = parseInt(exerciseIdx + 1) + ":";
		exerciseItem.Layout.row = exerciseIdx;
	}

	function createSetObject(type: int, n: int, nreps: string, nweight: string) {
		var nsets_created = 0;
		function setObjectCreated(object, id) {
			if (id === 140) {
				if (++nsets_created === n)
					itemManager.itemReady.disconnect(setObjectCreated);
				setAdded(exerciseIdx, object);
			}
		}

		itemManager.itemReady.connect(setObjectCreated);
		itemManager.createSetObjects(exerciseIdx, setNbr, setNbr + n, type, nreps, nweight);
		setNbr += n;
	}

	function paneExerciseShowHide() {
		paneExercise.shown = !paneExercise.shown
		if (paneExercise.shown)
			itemManager.createSetObjects(exerciseIdx);
	}
} //Item
