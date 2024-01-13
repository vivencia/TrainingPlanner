import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "jsfunctions.js" as JSF

Item {
	id: exerciseItem
	property ListModel exercicesModel
	required property string exerciseName
	required property int tDayId
	required property var stackViewObj
	property bool bFoldPaneOnLoad: false

	property int setType: 0
	property int setNbr: -1
	property var setObjectY: []
	property var suggestedReps: []
	property var suggestedSubSets: []
	property var suggestedWeight: []
	property var suggestedRestTimes: []
	property var setNotes: []
	property var setIds: []
	property var btnFloat: null

	property int thisObjectIdx
	signal exerciseRemoved(int ObjectIdx)
	signal exerciseEdited(int oldId, int newId)
	signal exerciseEdited_SetChanged(int oldexercisename, int newexercisename)
	signal setAdded(bool bnewset, int sety, int setheight, int objidx)
	signal requestHideFloatingButtons(int except_idx)

	property var setObjectList: []
	property var setsInfoList: []

	property string origExerciseName
	property bool bCompositeExercise: false
	property bool bFloatButtonVisible

	Layout.fillWidth: true
	implicitHeight: paneExercise.height

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

	MessageDialog {
		id: msgDlgRemove
		text: qsTr("\n\nRemove Exercise?\n\n")
		informativeText: qsTr("This action cannot be undone.")
		buttons: MessageDialog.Yes | MessageDialog.No

		onButtonClicked: function (button, role) {
			switch (button) {
				case MessageDialog.Yes:
					accept();
					removeAllSets();
					exerciseRemoved(thisObjectIdx);
				break;
				case MessageDialog.No:
					reject();
				break;
			}
		}
	} //MessageDialog

	Frame {
		id: paneExercise
		property bool shown: !bFoldPaneOnLoad
		visible: height > 0
		height: shown ? implicitHeight : txtExerciseName.height
		Behavior on height {
			NumberAnimation {
				easing.type: Easing.InOutQuad
			}
		}
		clip: true
		padding: 0
		z: 0

		background: Rectangle {
			color: thisObjectIdx % 2 === 0 ? primaryLightColor : primaryColor
			border.color: "transparent"
			opacity: 0.4
			radius: 5
		}

		Layout.fillWidth: true
		implicitHeight: layoutMain.implicitHeight + 10
		implicitWidth: parent.width
		width: parent.width

		ColumnLayout {
			id: layoutMain
			anchors.fill: parent
			spacing: 0

			TextField {
				id: txtExerciseName
				text: exerciseName
				font.bold: true
				font.pixelSize: AppSettings.fontSizeText
				readOnly: true
				wrapMode: Text.WordWrap
				width: parent.width - 75
				height: 60
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				Layout.minimumHeight: height
				Layout.maximumHeight: height
				Layout.leftMargin: 5
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
						if (bCompositeExercise) { //Remove the '1: ' from the name
							const idx = exerciseName.indexOf(':');
							exerciseName = exerciseName.substring(idx + 1, exerciseName.length).trim();
						}
					}
					else
						ensureVisible(0);
				}

				onActiveFocusChanged: {
					if (activeFocus)
						closeSimpleExerciseList();
				}

				onEditingFinished: {
					if (!bCompositeExercise)
						exerciseName = text;
					else
						exerciseName = "1: " + text;
					exerciseEdited_SetChanged(origExerciseName, exerciseName);
				}

				ToolButton {
					id: btnFoldIcon
					anchors.left: txtExerciseName.right
					anchors.verticalCenter: parent.verticalCenter
					anchors.rightMargin: -5
					height: 25
					width: 25
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

				ToolButton {
					id: btnRemoveExercise
					padding: 10
					anchors.left: btnFoldIcon.right
					anchors.verticalCenter: parent.verticalCenter
					height: 25
					width: 25
					z: 2
					Image {
						source: "qrc:/images/"+darkIconFolder+"remove.png"
						anchors.fill: parent
						height: 20
						width: 20
					}

					onClicked: {
						msgDlgRemove.open();
					}
				} //btnRemoveExercise

				ToolButton {
					id: btnEditExercise
					padding: 10
					anchors.left: btnRemoveExercise.right
					anchors.verticalCenter: parent.verticalCenter
					height: 25
					width: 25
					z: 2
					Image {
						source: "qrc:/images/"+darkIconFolder+"edit.png"
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
						height: 20
						width: 20
					}

					onClicked: {
						if (txtExerciseName.readOnly) {
							origExerciseName = exerciseName;
							txtExerciseName.readOnly = false;
							//txtExerciseName.forceActiveFocus();
							requestSimpleExerciseList(exerciseItem);
						}
						else {
							txtExerciseName.readOnly = true;
							closeSimpleExerciseList();
						}
						//var exercise = stackViewObj.push("ExercisesDatabase.qml", { bChooseButtonEnabled: true });
						//exercise.exerciseChosen.connect(gotExercise);
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

				ComboBox {
					id: cboSetType
					model: setTypes
					Layout.minimumWidth: 140
					currentIndex: setType
					textRole: "key"
					valueRole: "idx"
				}
				ToolButton {
					id: btnAddSet

					Image {
						source: "qrc:/images/"+darkIconFolder+"add-new.png";
						height: 20
						width: 20
						anchors.verticalCenter: parent.verticalCenter
						anchors.horizontalCenter: parent.horizontalCenter
					}
					onClicked: {
						setType = parseInt(cboSetType.currentValue);
						loadSetType(setType, true);
						requestHideFloatingButtons (thisObjectIdx);
						if (btnFloat === null)
							createFloatingAddSetButton();
						else
							bFloatButtonVisible = true;
					}
				}
			} // RowLayout
		} // ColumnLayout layoutMain

		Component.onCompleted: loadSetsFromDatabase();

		Component.onDestruction: {
			for (var i = 0; i < setObjectList.length; ++i)
				setObjectList[i].Object.destroy();
			delete setObjectList;
			destroyFloatingAddSetButton();
		}
	} //paneExercise

	function createFloatingAddSetButton() {
		var component;
		component = Qt.createComponent("FloatingButton.qml");
		if (component.status === Component.Ready) {
			btnFloat = component.createObject(exerciseItem, {
						text:qsTr("Add set"), image:"add-new.png", comboIndex:setType, nextSetNbr: setNbr + 2
			});
			btnFloat.buttonClicked.connect(addNewSet);
			bFloatButtonVisible = true;
			changeComboModel();
		}
	}

	function changeComboModel() {
		switch (setType) {
			case 0:
			case 1:
			case 2:
				cboSetType.model = [setTypes[0], setTypes[1], setTypes[2]];
			break;
			case 3: cboSetType.model = [setTypes[3]];
			break;
			case 4: cboSetType.model = [setTypes[4]];
			break;
			case 5: cboSetType.model = [setTypes[5]];
			break;
		}
	}

	function destroyFloatingAddSetButton () {
		if (btnFloat !== null) {
			btnFloat.destroy();
			cboSetType.model = setTypes;
		}
		bFloatButtonVisible = false;
	}

	function addNewSet(type) {
		setType = type;
		loadSetType(type, true);
	}

	function loadSetsFromDatabase() {
		setsInfoList = Database.getSetsInfo(tDayId);
		var nset = 0;
		for(var i = 0; i < setsInfoList.length; ++i) {
			if (thisObjectIdx === setsInfoList[i].setExerciseIdx) {
				setNbr = setsInfoList[i].setNumber;
				suggestedReps[nset] = setsInfoList[i].setReps;
				suggestedWeight[nset] = setsInfoList[i].setWeight;
				suggestedSubSets[nset] = setsInfoList[i].setSubSets;
				suggestedRestTimes[nset] = setsInfoList[i].setRestTime;
				setNotes[nset] = setsInfoList[i].setNotes;
				setIds[nset] = setsInfoList[i].setId;
				loadSetType(setsInfoList[i].setType, false);
				nset++;
				setType = setsInfoList[i].setType;
				changeComboModel();
			}
		}
	}

	function updateDayId(newDayId) {
		tDayId = newDayId;
		for(var i = 0; i < setObjectList.length; ++i) {
			setsInfoList[i].setTrainingdDayId = tDayId;
			setObjectList[i].Object.updateTrainingDayId(tDayId);
		}
	}

	function loadSetType(type, bNewSet) {
		const setTypePage = ["SetTypeRegular.qml", "SetTypePyramid.qml",
			"SetTypeDrop.qml", "SetTypeCluster.qml", "SetTypeGiant.qml", "SetTypeMyoReps.qml"];

		var component;
		var sprite;
		component = Qt.createComponent(setTypePage[type]);
		if (component.status === Component.Ready) {
			if (bNewSet) {
				setNbr++;
				calculateSuggestedValues(type);
			}
			sprite = component.createObject(layoutMain, {
								setId:setIds[setNbr], setNumber:setNbr, setReps:suggestedReps[setNbr],
								setWeight:suggestedWeight[setNbr], setSubSets:suggestedSubSets[setNbr],
								setRestTime:suggestedRestTimes[setNbr], setNotes:setNotes[setNbr], exerciseIdx:thisObjectIdx,
								tDayId:tDayId
			});
			setObjectList.push({"Object" : sprite});
			sprite.setRemoved.connect(setRemoved);
			sprite.setChanged.connect(setChanged);

			if (type === 4) { //Giant set
				bCompositeExercise = true;
				sprite.stackViewObj = stackViewObj;
				sprite.secondExerciseNameChanged.connect(compositeSetChanged);
				if (!bNewSet) {
					const sep = exerciseName.indexOf('&');
					var exercise2Name;
					if (sep !== -1)
						exercise2Name = exerciseName.substring(sep + 1, exerciseName.length);
					sprite.exerciseName2 = exercise2Name;
				}
			}
			if (setNbr >= 1) {
				setObjectY[setNbr] = setObjectList[setNbr-1].Object.y + setObjectList[setNbr-1].Object.height;
			}
			else
				setObjectY[setNbr] = 0;
			setAdded(bNewSet, setObjectY[setNbr], sprite.height, thisObjectIdx);
			if (btnFloat !== null)
				btnFloat.nextSetNbr++;
		}
		else
			console.log("not ready");
	}

	function setChanged(nset, nReps, nWeight, nSubSets, restTime, notes) {
		suggestedReps[nset] = nReps;
		suggestedWeight[nset] = nWeight;
		suggestedSubSets[nset] = nSubSets;
		suggestedRestTimes[nset] = restTime;
		setNotes[nset] = notes;
		exerciseEdited_SetChanged("", "");
	}

	function compositeSetChanged(newexercise2name) {
		const oldName = exerciseName;
		const sep = oldName.indexOf('&');
		var exercise1Name;
		if (sep !== -1)
			exercise1Name = oldName.substring(0, sep);
		else
			exercise1Name = oldName;
		exerciseName = exercise1Name + '&' + newexercise2name;
		exerciseEdited_SetChanged(oldName, exerciseName);
	}

	function logSets() {
		for( var i = 0; i < setObjectList.length; ++i )
			setObjectList[i].Object.logSet();
	}

	function removeAllSets() {
		for( var i = 0; i < setObjectList.length; ++i ) {
			setObjectList[i].Object.bIsRemoving = true;
			setObjectList[i].Object.destroy();
		}
		delete setObjectList;
		delete setObjectY;
		let newObjectList = new Array;
		setObjectList = newObjectList;
		let newObjectY = new Array;
		setObjectY = newObjectY;
		destroyFloatingAddSetButton ();
	}

	function setRemoved(nset) {
		let newObjectList = new Array;
		let newObjectY = new Array;
		setObjectList[nset].Object.destroy();
		setNbr--;

		for(var i = 0, x = 0; i < setObjectList.length; ++i) {
			if (i >= nset) {
				setObjectList[i].Object.bIsRemoving = true;
				setObjectList[i].Object.setNumber = setObjectList[i].Object.setNumber - 1;
				if (i < setObjectList.length - 1)
					setObjectY[i+1] = setObjectY[i];
			}
			if (i !== nset) {
				newObjectList[x] = setObjectList[i];
				newObjectY[x] = setObjectY[i];
				x++;
			}
		}
		delete setObjectList;
		delete setObjectY;
		setObjectList = newObjectList;
		setObjectY = newObjectY;
		if (setObjectList.length === 0) { //setNbr === -1
			destroyFloatingAddSetButton ();
		}
		else {
			if (btnFloat !== null)
				btnFloat.nextSetNbr--;
		}
		exerciseEdited_SetChanged("", "");
	}

	function updateSetsExerciseIndex() {
		for(var i = 0, x = 0; i < setObjectList.length; ++i)
			setObjectList[i].Object.exerciseIdx = thisObjectIdx;
	}

	function changeExercise(name1, name2) {
		exerciseName = name1 + ' - ' + name2;
	}

	function calculateSuggestedValues(type) {
		setIds[setNbr] = -1;
		setNotes[setNbr] = setNbr === 0 ? "  " : setNotes[setNbr-1];
		switch (type) {
			case 0: //Regular
			case 4: //Giant
				if (setNbr > 0) {
					suggestedReps[setNbr] = suggestedReps[setNbr-1];
					suggestedWeight[setNbr] = suggestedWeight[setNbr-1];
					suggestedRestTimes[setNbr] = JSF.increaseStringTimeBy(suggestedRestTimes[setNbr-1], "00:30");
				}
				else {
					suggestedReps[0] = 12;
					suggestedWeight[0] = 30;
					suggestedRestTimes[0] = "01:30";
				}
				suggestedSubSets[setNbr] = 0;
			break;
			case 1: //Pyramid
				if (setNbr > 0) {
					suggestedReps[setNbr] = suggestedReps[setNbr-1] - 3;
					suggestedWeight[setNbr] = suggestedWeight[setNbr-1] * 1.2;
					suggestedRestTimes[setNbr] = JSF.increaseStringTimeBy(suggestedRestTimes[setNbr-1], "00:30");
				}
				else {
					suggestedReps[0] = 15;
					suggestedWeight[0] = 20;
					suggestedRestTimes[0] = "01:30";
				}
				suggestedSubSets[setNbr] = 0;
			break;
			case 2: //DropSet
				if (setNbr > 0) {
					suggestedReps[setNbr] = suggestedReps[setNbr-1];
					suggestedWeight[setNbr] = suggestedWeight[setNbr-1];
					suggestedSubSets[setNbr] = suggestedSubSets[setNbr-1];
					suggestedRestTimes[setNbr] = JSF.increaseStringTimeBy(suggestedRestTimes[setNbr-1], "00:30");
				}
				else {
					suggestedReps[0] = "15";
					suggestedWeight[0] = "50";
					suggestedSubSets[0] = 3;
					suggestedRestTimes[0] = "01:30";
				}
			break;
			case 3: //Cluster set
				if (setNbr > 0) {
					suggestedReps[setNbr] = suggestedReps[setNbr-1];
					suggestedWeight[setNbr] = suggestedWeight[setNbr-1];
					suggestedSubSets[setNbr] = suggestedSubSets[setNbr-1];
					suggestedRestTimes[setNbr] = JSF.increaseStringTimeBy(suggestedRestTimes[setNbr-1], "01:00");
				}
				else {
					suggestedReps[0] = 6;
					suggestedWeight[0] = 40;
					suggestedSubSets[0] = 4;
					suggestedRestTimes[0] = "02:00";
				}
			break;
			case 5: //Myo reps
				if (setNbr > 0) {
					suggestedReps[setNbr] = suggestedReps[setNbr-1];
					suggestedWeight[setNbr] = suggestedWeight[setNbr-1];
					suggestedSubSets[setNbr] = suggestedSubSets[setNbr-1] + 1;
					suggestedRestTimes[setNbr] = JSF.increaseStringTimeBy(suggestedRestTimes[setNbr-1], "00:30");
				}
				else {
					suggestedReps[0] = 6;
					suggestedWeight[0] = 100;
					suggestedSubSets[0] = 0;
					suggestedRestTimes[0] = "02:30";
				}
			break;
		}
	}

	function gotExercise(strName1, strName2, sets, reps, weight, bAdd) {
		const oldName = exerciseName;
		exerciseName = strName1 + ' - ' + strName2;
		if (bCompositeExercise) {
			const sep = oldName.indexOf('&');
			var exercise2Name;
			if (sep !== -1)
				exercise2Name = oldName.substring(sep + 1, oldName.length);
			exerciseName = "1: " + strName1 + ' - ' + strName2 + '&' + exercise2Name;
		}
		exerciseEdited_SetChanged(oldName, exerciseName);
	}
} //Item
