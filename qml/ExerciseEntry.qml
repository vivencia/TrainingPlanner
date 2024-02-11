import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "jsfunctions.js" as JSF

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	required property int thisObjectIdx
	required property string exerciseName
	required property string splitLetter
	required property string exerciseName1
	required property string exerciseName2

	property int tDayId: -1
	property int setType: 0
	property int setNbr: -1
	property var suggestedReps: []
	property var suggestedSubSets: []
	property var suggestedWeight: []
	property var suggestedRestTimes: []
	property var setNotes: []
	property var setObjectList: []
	property var setCreated: []

	property int loadTDayId: -1
	property int loadObjectIdx: -1
	property var btnFloat: null
	property bool bCompositeExercise: false
	property bool bFloatButtonVisible
	property bool bSetsLoaded: false
	property int setBehaviour: 0 //0: do not load sets, 1: load sets from database, 2: load sets from plan

	signal exerciseRemoved(int ObjectIdx)
	signal exerciseEdited(int objidx, string newname)
	signal setAdded(bool bnewset, int objidx, var setObject)
	signal setWasRemoved(int setid)
	signal requestHideFloatingButtons(int except_idx)

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

	Timer {
		id: logSetsTimer
		interval: 200
		running: false
		repeat: true

		onTriggered: {
			if (bSetsLoaded)
				stop();
			const len = setCreated.length;
			var totalReady = 0;
			for(var i = 0; i < len; ++i) {
				switch(setCreated[i]) {
					case 0:
					break;
					case 1:
						setObjectList[i].Object.logSet();
						setCreated[i] = 2;
						bSetsLoaded = true;
					break;
					case 2:
						totalReady++;
					break;
				}
			}
			bSetsLoaded = totalReady === len;
		}
	}

	TPBalloonTip {
		id: msgDlgRemove
		title: qsTr("Remove Exercise")
		message: exerciseName + qsTr("? This action cannot be undone.")
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: {
			removeAllSets();
			exerciseRemoved(thisObjectIdx);
		}
	} //TPBalloonTip

	Frame {
		id: paneExercise
		property bool shown: false
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

		onShownChanged: {
			if (shown) {
				if (!bSetsLoaded)
				{
					bSetsLoaded = true;
					createSets();
				}
			}
		}

		background: Rectangle {
			color: thisObjectIdx % 2 === 0 ? listEntryColor1 : listEntryColor2
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
			anchors.fill: parent
			spacing: 0

			TextField {
				id: txtExerciseName
				text: exerciseName1
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
						if (bCompositeExercise) { //Remove the '1: ' from the name
							const idx = exerciseName1.indexOf(':');
							exerciseName1 = exerciseName1.substring(idx + 1, exerciseName1.length).trim();
						}
						cursorPosition = text.length;
					}
					else {
						cursorPosition = 0;
						ensureVisible(0);
					}
				}

				onActiveFocusChanged: {
					if (activeFocus) {
						closeSimpleExerciseList();
						cursorPosition = text.length;
					}
					else
						cursorPosition = 0;
				}

				onEditingFinished: {
					if (!bCompositeExercise) {
						exerciseName1 = text;
						exerciseName = exerciseName1;
					}
					else {
						exerciseName1 = "1: " + text;
						exerciseName = exerciseName1 + '&' + exerciseName2;
					}
					exerciseEdited(thisObjectIdx, exerciseName);
				}

				Label {
					id: lblExerciseNumber
					text: parseInt(thisObjectIdx + 1) + ":"
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
						if (txtExerciseName.readOnly) {
							txtExerciseName.readOnly = false;
							requestSimpleExerciseList(exerciseItem);
						}
						else {
							txtExerciseName.readOnly = true;
							closeSimpleExerciseList();
						}
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
					model: setTypes
					Layout.minimumWidth: 140
					currentIndex: setType
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
						setType = parseInt(cboSetType.currentValue);
						createSetObject(setType, true);
						requestHideFloatingButtons (thisObjectIdx);
						if (btnFloat === null)
							createFloatingAddSetButton();
						else
							bFloatButtonVisible = true;
					}
				}
			} // RowLayout
		} // ColumnLayout layoutMain

		Component.onDestruction: {
			const len = setObjectList.length;
			for (var i = 0; i < len; ++i)
				setObjectList[i].Object.destroy();
			delete setObjectList;
			destroyFloatingAddSetButton();
		}
	} //paneExercise

	function createFloatingAddSetButton() {
		var component = Qt.createComponent("FloatingButton.qml", Qt.Asynchronous);
		function finishCreation() {
			btnFloat = component.createObject(exerciseItem, {
					text:qsTr("Add set"), image:"add-new.png", comboIndex:setType, nextSetNbr: setNbr + 2
			});
			btnFloat.buttonClicked.connect(addNewSet);
			bFloatButtonVisible = true;
			changeComboModel();
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function changeComboModel() {
		switch (setType) {
			case 0:
			case 1:
			case 2:
				cboSetType.model = [setTypes[0], setTypes[1], setTypes[2]];
				cboSetType.currentIndex = setType;
				return;
			case 3: cboSetType.model = [setTypes[3]];
			break;
			case 4: cboSetType.model = [setTypes[4]];
			break;
			case 5: cboSetType.model = [setTypes[5]];
			break;
		}
		cboSetType.currentIndex = 0;
	}

	function destroyFloatingAddSetButton () {
		if (btnFloat !== null) {
			btnFloat.destroy();
			btnFloat = null;
			cboSetType.model = setTypes;
		}
		bFloatButtonVisible = false;
	}

	function addNewSet(type) {
		setType = type;
		createSetObject(type, true);
	}

	function loadSetsFromDatabase() {
		let setsInfoList = Database.getSetsInfo(loadTDayId);
		const len = setsInfoList.length;
		//console.log("ExerciseEntry::loadSetsFromDatabase - loadObjectIdx = ", loadObjectIdx);
		//console.log("Creating " + len + " sets")
		for(var i = 0; i < len; ++i) {
			if (loadObjectIdx === setsInfoList[i].setExerciseIdx) {
				var bSame = false;
				for(var x = 0; x < i; x++) {
					if (setsInfoList[x].setExerciseIdx === loadObjectIdx) {
						if (setsInfoList[x].setNumber === setsInfoList[i].setNumber) {
							Database.deleteSetFromSetsInfo(setsInfoList[i].setId);
							bSame = true;
							break;
						}
					}
				}
				if (bSame)
					continue;
				setNbr = setsInfoList[i].setNumber;
				setCreated[setNbr] = 0;
				suggestedReps[setNbr] = setsInfoList[i].setReps;
				suggestedWeight[setNbr] = setsInfoList[i].setWeight;
				suggestedSubSets[setNbr] = setsInfoList[i].setSubSets;
				suggestedRestTimes[setNbr] = setsInfoList[i].setRestTime;
				setNotes[setNbr] = setsInfoList[i].setNotes;
				setType = setsInfoList[i].setType;
				createSetObject(setType, false);
			}
		}
		if (setNbr >= 0)
			changeComboModel();
	}

	function loadSetsFromMesoPlan() {
		let plan_info = [];
		switch (splitLetter) {
			case 'A': plan_info = Database.getCompleteDivisionAForMeso(mesoId); break;
			case 'B': plan_info = Database.getCompleteDivisionBForMeso(mesoId); break;
			case 'C': plan_info = Database.getCompleteDivisionCForMeso(mesoId); break;
			case 'D': plan_info = Database.getCompleteDivisionDForMeso(mesoId); break;
			case 'E': plan_info = Database.getCompleteDivisionEForMeso(mesoId); break;
			case 'F': plan_info = Database.getCompleteDivisionFForMeso(mesoId); break;
		}
		const types = plan_info[0].splitSetTypes.split('|');
		const nsets = plan_info[0].splitNSets.split('|');
		const nreps = plan_info[0].splitNReps.split('|');
		const nweights = plan_info[0].splitNWeight.split('|');
		createSetsFromPlan(nsets[loadObjectIdx], types[loadObjectIdx], nreps[loadObjectIdx], nweights[loadObjectIdx]);
	}

	function createSetsFromPlan(nsets, type, nreps, nweight) {
		//console.log("ExerciseEntry::createSetsFromPlan - loadObjectIdx = ", loadObjectIdx);
		//console.log("Creating " + nsets + " sets")
		const _nsets = parseInt(nsets);
		for(var i = 0; i < _nsets; ++i) {
			setCreated[i] = 0;
			setNbr = i;
			setType = parseInt(type);
			calculateSuggestedValues(parseInt(type));
			if (i === 0) {
				suggestedReps[0] = parseInt(nreps);
				suggestedWeight[0] = parseInt(nweight);
				if (setType === 3)//ClusterSet
					suggestedReps[0] /= 4;
			}
			createSetObject(setType, false);
		}
	}

	function createSets() {
		switch (setBehaviour) {
			case 0:
			break;
			case 1: loadSetsFromDatabase(); break;
			case 2: loadSetsFromMesoPlan(); break;
		}
	}

	//sets objects are created on demand(when shown). If they are not created until the day is saved, we need to do it now
	//so that they can log their information for the day. If they were created before saving, they will contain wrong tDayId, i.e.
	//one that was either borrowed from another day or -1 when created from a meso plan. Before logging, we must set tDayId to the value
	//obtained from inserting TrainingDay into the database
	function updateDayId(newDayId) {
		tDayId = newDayId;
		if (setObjectList.length === 0)
			createSets();
		else {
			const len = setObjectList.length;
			for(var i = 0; i < len; ++i)
				setObjectList[i].Object.updateTrainingDayId(tDayId);
		}
	}

	function updateSetsExerciseIndex(newObjIndex) {
		thisObjectIdx = newObjIndex;
		if (setObjectList.length === 0)
			createSets();
		else {
			const len = setObjectList.length;
			for(var i = 0, x = 0; i < len; ++i)
				setObjectList[i].Object.exerciseIdx = thisObjectIdx;
		}
	}

	function createSetObject(type, bNewSet) {
		const setTypePage = ["SetTypeRegular.qml", "SetTypePyramid.qml",
			"SetTypeDrop.qml", "SetTypeCluster.qml", "SetTypeGiant.qml", "SetTypeMyoReps.qml"];

		function generateSetObject(page, setnbr) {
			var component = Qt.createComponent(page, Component.Asynchronous);

			function finishCreation(nset) {
				if (bNewSet) {
					nset++;
					setNbr = nset;
					calculateSuggestedValues(type);
					if (btnFloat !== null)
						btnFloat.nextSetNbr++;
				}
				//console.log("ExerciseEntry::createSetObject # " + nset + " - exerciseIdx = " + thisObjectIdx + "  - tDayId = " + tDayId);
				var sprite = component.createObject(layoutMain, {
								setNumber:nset, setReps:suggestedReps[nset], setWeight:suggestedWeight[nset],
								setSubSets:suggestedSubSets[nset], setRestTime:suggestedRestTimes[nset],
								setNotes:setNotes[nset], exerciseIdx:thisObjectIdx, tDayId:tDayId
				});
				setObjectList.push({ "Object" : sprite });
				sprite.setRemoved.connect(setRemoved);
				sprite.setChanged.connect(setChanged);

				if (nset >= 1)
					setObjectList[nset-1].Object.nextObject = sprite;
				else {
					if (type === 4) { //Giant set
						bCompositeExercise = true;
						sprite.secondExerciseNameChanged.connect(compositeSetChanged);
						if (bNewSet)
							exerciseName2 = qsTr("2: Add exercise");
						sprite.exerciseName2 = exerciseName2;
					}
				}
				setAdded(bNewSet, thisObjectIdx, sprite);
			}

			function checkStatus() {
				if (component.status === Component.Ready)
					finishCreation(setnbr);
			}

			if (component.status === Component.Ready)
				finishCreation(setnbr);
			else
				component.statusChanged.connect(checkStatus);
		}

		generateSetObject(setTypePage[type], setNbr);
	}

	function setChanged(nset, nReps, nWeight, nSubSets, restTime, notes) {
		suggestedReps[nset] = nReps;
		suggestedWeight[nset] = nWeight;
		suggestedSubSets[nset] = nSubSets;
		suggestedRestTimes[nset] = restTime;
		setNotes[nset] = notes;
		exerciseEdited(-1, "");
	}

	function compositeSetChanged(newexercise2name) {
		exerciseName2 = newexercise2name;
		exerciseName = exerciseName1 + '&' + exerciseName2;
		exerciseEdited(thisObjectIdx, exerciseName);
	}

	function logSets() {
		logSetsTimer.start();
	}

	function removeAllSets() {
		const len = setObjectList.length;
		for( var i = 0; i < len; ++i ) {
			var setid = setObjectList[i].Object.setId;
			if (setid !== -1)
				setWasRemoved(setid);
			setObjectList[i].Object.destroy();
		}
		destroyFloatingAddSetButton ();
	}

	function setRemoved(nset) {
		const len = setObjectList.length;
		const new_len = len - 1;
		const setid = setObjectList[nset].Object.setId;
		if (setid !== -1)
			setWasRemoved(setid);
		setObjectList[nset].Object.destroy();
		setNbr--;

		let newObjectList = new Array(new_len);
		let newSuggestedReps = new Array(new_len);
		let newSuggestedSubSets = new Array(new_len);
		let newSuggestedWeight = new Array(new_len);
		let newSuggestedRestTimes = new Array(new_len);
		let newSetNotes = new Array(new_len);

		for(var i = 0, x = 0; i < len; ++i) {
			if (i !== nset) {
				if (i > nset)
					setObjectList[i].Object.setNumber--; // = setObjectList[i].Object.setNumber - 1;

				newObjectList[x] = setObjectList[i];
				newSuggestedReps[x] = suggestedReps[i];
				newSuggestedSubSets[x] = suggestedSubSets[i];
				newSuggestedWeight[x] = suggestedWeight[i];
				newSuggestedRestTimes[x] = suggestedRestTimes[i];
				newSetNotes[x] = setNotes[i];
				x++;
			}
		}
		delete setObjectList;		setObjectList = newObjectList;
		delete suggestedReps;		suggestedReps = newSuggestedReps;
		delete suggestedSubSets;	suggestedSubSets = newSuggestedSubSets;
		delete suggestedWeight;		suggestedWeight = newSuggestedWeight;
		delete setNotes;			setNotes = newSetNotes;

		if (setObjectList.length === 0)
		{
			destroyFloatingAddSetButton ();
			cboSetType.model = setTypes;
			setType = 0;
		}
		else {
			if (btnFloat !== null)
				btnFloat.nextSetNbr--;
		}
		exerciseEdited(-1, ""); //Just to set bModified to true
	}

	function changeExercise(name1, name2) {
		if (!bCompositeExercise) {
			exerciseName1 = name1 + ' - ' + name2;
			exerciseName = exerciseName1;
		}
		else {
			exerciseName1 = "1: " + name1 + ' - ' + name2;
			exerciseName = exerciseName1 + '&' + exerciseName2;
		}
		exerciseEdited(thisObjectIdx, exerciseName);
	}

	function gotExercise(strName1, strName2, sets, reps, weight, bAdd) {
		changeExercise(strName1, strName2);
	}

	function foldUpSets() {
		paneExercise.shown = false;
	}

	function calculateSuggestedValues(type) {
		setNotes[setNbr] = setNbr === 0 ? "  " : setNotes[setNbr-1];
		switch (type) {
			case 0: //Regular
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
					suggestedWeight[setNbr] = Math.floor(suggestedWeight[setNbr-1] * 1.2);
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
					suggestedReps[0] = "15#12#10";
					suggestedWeight[0] = "50#40#30";
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
			case 4: //Giant
				if (setNbr > 0) {
					suggestedReps[setNbr] = suggestedReps[setNbr-1];
					suggestedWeight[setNbr] = suggestedWeight[setNbr-1];
					suggestedRestTimes[setNbr] = JSF.increaseStringTimeBy(suggestedRestTimes[setNbr-1], "00:30");
				}
				else {
					suggestedReps[0] = "12#12";
					suggestedWeight[0] = "30#30";
					suggestedRestTimes[0] = "01:30";
				}
				suggestedSubSets[setNbr] = 0;
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
} //Item
