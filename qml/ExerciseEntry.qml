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
	property int setNbr: -1

	property var btnFloat: null
	property bool bFloatButtonVisible

	signal setAdded(int objidx, var setObject)
	signal requestHideFloatingButtons(int except_idx)
	signal requestSimpleExercisesList(Item requester, var bVisible, int id)

	property var setTypesModel: [ { text:qsTr("Regular"), value:0 }, { text:qsTr("Pyramid"), value:1 }, { text:qsTr("Drop Set"), value:2 },
							{ text:qsTr("Cluster Set"), value:3 }, { text:qsTr("Giant Set"), value:4 }, { text:qsTr("Myo Reps"), value:5 } ]

	onBFloatButtonVisibleChanged: {
		if (bFloatButtonVisible) {
			if (btnFloat !== null)
				btnFloat.visible = true;
			cboSetType.enabled = false;
			btnAddSet.enabled = false;
		}
		else {
			if (btnFloat !== null)
				btnFloat.visible = false;
			cboSetType.enabled = true;
			btnAddSet.enabled = true;
		}
	}

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
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				Layout.minimumHeight: height
				Layout.maximumHeight: height
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
					requestSimpleExercisesList(paneSplit, !readOnly, 1);
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
					onClicked: paneExercise.shown = !paneExercise.shown
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

					onClicked: {
						txtExerciseName.readOnly = !txtExerciseName.readOnly;
					}
				}

				MouseArea {
					anchors.left: txtExerciseName.left
					anchors.right: txtExerciseName.right
					anchors.top: txtExerciseName.top
					anchors.bottom: txtExerciseName.bottom
					onClicked: paneExercise.shown = !paneExercise.shown
					enabled: txtExerciseName.readOnly
					z:1
				}
			} //txtExerciseName

			RowLayout {
				Layout.fillWidth: true
				Layout.leftMargin: 5
				Layout.rightMargin: 5
				spacing: 1

				Label {
					text: qsTr("Set type: ")
				}

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
						console.log(setNbr);
						return;
						tDayModel.setSetType(0, cboSetType.currentValue, exerciseIdx);
						createSetObject(cboSetType.currentIndex);
						requestHideFloatingButtons (exerciseIdx);
						if (btnFloat === null)
							createFloatingAddSetButton();
						else
							bFloatButtonVisible = true;
					}
				}
			} // RowLayout
		} // ColumnLayout layoutMain
		Component.onDestruction: {
			destroyFloatingAddSetButton();
		}
	} //paneExercise

	function createFloatingAddSetButton() {
		var component = Qt.createComponent("FloatingButton.qml", Qt.Asynchronous);
		function finishCreation() {
			btnFloat = component.createObject(exerciseItem, {
					text:qsTr("Add set"), image:"add-new.png", comboIndex:tDayModel.setType(setNbr, exerciseIdx), nextSetNbr: setNbr + 1
			});
			btnFloat.buttonClicked.connect(createSetObject);
			bFloatButtonVisible = true;
			changeComboModel();
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function changeComboModel() {
		switch (tDayModel.setType(0, exerciseIdx)) {
			case 0:
			case 1:
			case 2:
				cboSetType.model = [setTypesModel[0], setTypesModel[1], setTypesModel[2]];
				cboSetType.currentIndex = tDayModel.setType(0, exerciseIdx);
				return;
			case 3: cboSetType.model = [setTypesModel[3]];
			break;
			case 4: cboSetType.model = [setTypesModel[4]];
			break;
			case 5: cboSetType.model = [setTypesModel[5]];
			break;
		}
		cboSetType.currentIndex = 0;
	}

	function destroyFloatingAddSetButton () {
		if (btnFloat !== null) {
			btnFloat.destroy();
			btnFloat = null;
			cboSetType.model = setTypesModel;
		}
		bFloatButtonVisible = false;
	}

	function changeExercise(newname)
	{
		txtExerciseName.text = newname;
		tDayModel.setExerciseName1(newname, exerciseIdx);
	}

	function createSetObject(type) {
		function setObjectCreated(object) {
			appDB.getItem.disconnect(setObjectCreated);
			setAdded(exerciseIdx, object);
			if (btnFloat !== null)
				btnFloat.nextSetNbr++;
		}

		setNbr++;
		appDB.getItem.connect(setObjectCreated);
		appDB.createSetObject(cboSetType.currentIndex, setNbr, exerciseIdx);
	}

	function foldUpSets() {
		paneExercise.shown = false;
	}

	function requestExercisesList(requester, visible) {
		requestSimpleExercisesList(requester, visible);
	}
} //Item
