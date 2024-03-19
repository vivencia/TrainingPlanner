import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	required property DBTrainingDayModel tDayModel
	required property int exerciseIdx

	property int tDayId: -1
	property int setNbr: 0

	signal setAdded(int objidx, var setObject)
	signal requestSimpleExercisesList(Item requester, var bVisible, int id)
	signal requestFloatingButton(var exerciseidx)

	readonly property var setTypesModel: [ { text:qsTr("Regular"), value:0 }, { text:qsTr("Pyramid"), value:1 }, { text:qsTr("Drop Set"), value:2 },
							{ text:qsTr("Cluster Set"), value:3 }, { text:qsTr("Giant Set"), value:4 }, { text:qsTr("Myo Reps"), value:5 } ]

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise")
		message: tDayModel.exerciseName(exerciseIdx) + qsTr("? This action cannot be undone.")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: {
			appDB.removeExerciseObject(exerciseIdx);
		}
	} //TPBalloonTip

	Frame {
		id: paneExercise
		property bool shown: tDayModel.setsNumber(exerciseIdx) === 0
		visible: height > 0
		height: shown ? implicitHeight : txtExerciseName.height
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
		width: parent.width

		ColumnLayout {
			id: layoutMain
			objectName: "exerciseSetsLayout"
			anchors.fill: parent
			spacing: 0

			TextField {
				id: txtExerciseName
				text: tDayModel.exerciseName1(exerciseIdx)
				font.bold: true
				font.pixelSize: AppSettings.fontSizeText
				readOnly: true
				wrapMode: Text.WordWrap
				width: parent.width - 100
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

				onReadOnlyChanged: {
					if (!readOnly) {
						cursorPosition = text.length;
						tDayModel.setExerciseName1(text, exerciseIdx);
					}
					else {
						cursorPosition = 0;
						ensureVisible(0);
					}
					requestSimpleExercisesList(exerciseItem, !readOnly, 1);
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
						cursorPosition = 0;
					}
				}

				Label {
					id: lblExerciseNumber
					text: parseInt(exerciseIdx + 1) + ":"
					font.pixelSize: AppSettings.fontSizeText
					anchors.right: txtExerciseName.left
					anchors.verticalCenter: txtExerciseName.verticalCenter
					width: 20
					padding: 2
				}

				RoundButton {
					id: btnFoldIcon
					anchors.right: lblExerciseNumber.left
					anchors.verticalCenter: txtExerciseName.verticalCenter
					height: 25
					width: 25
					padding: 5
					Image {
						source: paneExercise.shown ? "qrc:/images/"+darkIconFolder+"fold-up.png" : "qrc:/images/"+darkIconFolder+"fold-down.png"
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
					anchors.verticalCenter: txtExerciseName.verticalCenter
					height: 25
					width: 25
					padding: 5
					z: 2
					Image {
						source: "qrc:/images/"+darkIconFolder+"remove.png"
						anchors.fill: parent
						height: 20
						width: 20
					}

					onClicked: msgDlgRemove.show(exerciseItem.y)
				} //btnRemoveExercise

				RoundButton {
					id: btnEditExercise
					anchors.left: btnRemoveExercise.right
					anchors.verticalCenter: parent.verticalCenter
					height: 25
					width: 25
					padding: 5
					z: 2
					Image {
						source: "qrc:/images/"+darkIconFolder+"edit.png"
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
						height: 20
						width: 20
					}

					onClicked: txtExerciseName.readOnly = !txtExerciseName.readOnly;
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

			Label {
				text: qsTr("Set type: ")
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				spacing: 1

				TPComboBox {
					id: cboSetType
					model: setTypesModel
					Layout.minimumWidth: 140
					currentIndex: tDayModel.setType(0, exerciseIdx)
				}

				RoundButton {
					id: btnAddSet

					Image {
						source: "qrc:/images/"+darkIconFolder+"add-new.png";
						height: 20
						width: 20
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
					}
					onClicked: {
						tDayModel.setSetType(0, cboSetType.currentValue, exerciseIdx);
						createSetObject(cboSetType.currentIndex, parseInt(txtNSets.text));
						requestFloatingButton(exerciseIdx);
					}
				}

				SetInputField {
					id: txtNSets
					text: "1"
					type: SetInputField.Type.SetType
					availableWidth: layoutMain.width / 3
					showLabel: false
					backColor: "transparent"
					borderColor: "transparent"

					onValueChanged: (str)=> text = str;
				}
			} // RowLayout
		} // ColumnLayout layoutMain
	} //paneExercise

	function changeExercise(newname)
	{
		txtExerciseName.text = newname;
		tDayModel.setExerciseName1(newname, exerciseIdx);
	}

	function createSetObject(type, n) {
		function setObjectCreated(object) {
			appDB.getItem.disconnect(setObjectCreated);
			setAdded(exerciseIdx, object);
		}

		for(var i = setNbr; i < setNbr + n; ++i)
		{
			appDB.getItem.connect(setObjectCreated);
			appDB.createSetObject(type, i, exerciseIdx);
		}
		setNbr += n;
	}

	function paneExerciseShowHide() {
		paneExercise.shown = !paneExercise.shown
		if (paneExercise.shown)
			appDB.createSetObjects(exerciseIdx);
	}

	function requestExercisesList(requester, visible) {
		requestSimpleExercisesList(requester, visible);
	}
} //Item
