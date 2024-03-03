import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import com.vivenciasoftware.qmlcomponents

FocusScope {
	id: exerciseItem
	Layout.fillWidth: true
	implicitHeight: paneExercise.height

	required property int thisObjectIdx
	required property DBTrainingDayModel tDayModel

	property var parentLayout
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
	signal exerciseEdited(int objidx)
	signal setAdded(bool bnewset, int objidx, var setObject)
	signal setWasRemoved(int setid)
	signal requestHideFloatingButtons(int except_idx)
	signal requestTimerDialogSignal(Item requester, var args, var tdaydate)

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
		message: tDayModel.exerciseName(thisObjectIdx) + qsTr("? This action cannot be undone.")
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
		property bool shown: tDayModel.setsNumber(thisObjectIdx) === 0
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
				text: tDayModel.exerciseName1(thisObjectIdx)
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
					if (!readOnly)
						cursorPosition = text.length;
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

				onEditingFinished:{
					tDayModel.setExerciseName1(text, thisObjectIdx);
					exerciseEdited(thisObjectIdx);
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
					model: setTypesModel
					Layout.minimumWidth: 140
					currentIndex: tDayModel.setType(0, thisObjectIdx)
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
						tDayModel.setSetType(0, cboSetType.currentValue, thisObjectIdx);
						createSetObject(cboSetType.currentIndex);
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

		Component.onCompleted: {
			const setTypePage = ["SetTypeRegular.qml", "SetTypePyramid.qml",
				"SetTypeDrop.qml", "SetTypeCluster.qml", "SetTypeGiant.qml", "SetTypeMyoReps.qml"];
			var component;
			for (var i = 0; i < tDayModel.setsNumber(thisObjectIdx); ++i) {

				function finishCreation() {
					var sprite = component.createObject(layoutMain, { tDayModel:tDayModel, exerciseIdx:thisObjectIdx, setNumber:setNbr });
					setObjectList.push({ "Object" : sprite });
					sprite.setRemoved.connect(setRemoved);

					if (setNbr >= 1)
						setObjectList[setNbr-1].Object.nextObject = sprite;
					setAdded(true, thisObjectIdx, sprite);
				}

				setNbr++;
				component = Qt.createComponent(setTypePage[tDayModel.setType(setNbr, thisObjectIdx)]);
				if (btnFloat !== null)
					btnFloat.nextSetNbr++;
				if (component.status === Component.Ready)
					finishCreation();
			}
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
		switch (tDayModel.setType(0, thisObjectIdx)) {
			case 0:
			case 1:
			case 2:
				cboSetType.model = [setTypesModel[0], setTypesModel[1], setTypesModel[2]];
				cboSetType.currentIndex = setType;
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

	function addNewSet(type) {
		setType = type;
		createSetObject(type);
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
		switch (tDayModel.splitLetter()) {
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

	function createSetObject(type) {
		const setTypePage = ["SetTypeRegular.qml", "SetTypePyramid.qml",
			"SetTypeDrop.qml", "SetTypeCluster.qml", "SetTypeGiant.qml", "SetTypeMyoReps.qml"];
		var component;

		function finishCreation() {
			var sprite = component.createObject(layoutMain, { tDayModel:tDayModel, exerciseIdx:thisObjectIdx, setNumber:setNbr });
			setObjectList.push({ "Object" : sprite });
			sprite.setRemoved.connect(setRemoved);

			if (setNbr >= 1)
				setObjectList[setNbr-1].Object.nextObject = sprite;
			else {
				if (type === 4) { //Giant set
					bCompositeExercise = true;
					sprite.secondExerciseNameChanged.connect(compositeSetChanged);
					sprite.exerciseName2 = qsTr("Add exercise");
				}
			}
			setAdded(true, thisObjectIdx, sprite);
		}

		function checkStatus() {
			if (component.status === Component.Ready)
				finishCreation();
		}

		component = Qt.createComponent(setTypePage[type], Component.Asynchronous);
		setNbr++;
		if (btnFloat !== null)
			btnFloat.nextSetNbr++;
		tDayModel.newSet(thisObjectIdx, setNbr, type);
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(checkStatus);
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

	function requestTimer(requester, message, mins, secs) {
		var args = [message, mins, secs];
		requestTimerDialogSignal(requester, args, tDayModel.date());
	}
} //Item
