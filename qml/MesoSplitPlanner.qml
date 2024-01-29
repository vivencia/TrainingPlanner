import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "jsfunctions.js" as JSF

Frame {
	id: paneSplit
	required property int divisionId
	required property int mesoId
	required property string splitLetter
	required property string splitText
	property string splitExercises
	property string splitSetTypes
	property string splitNSets
	property string splitNReps
	property string splitNWeight

	property bool bCanEditExercise: false
	property int currentModelIndex: 0
	property int seconds: 0
	property bool bModified: false
	property string filterString: ""

	implicitWidth: parent.width
	implicitHeight: splitLayout.height

	padding: 0
	spacing: 0
	Layout.leftMargin: 5

	ListModel {
		id: exercisesListModel
	}

	Timer {
		id: undoTimer
		interval: 1000
		property int idxToRemove

		onTriggered: {
			if ( seconds === 0 ) {
				undoTimer.stop();
				removeExercise(idxToRemove);
			}
			else {
				seconds = seconds - 1000;
				start();
			}
		}

		function init(idxtoremove) {
			idxToRemove = idxtoremove;
			start();
		}
	} //Timer

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
			text: splitText
			font.pixelSize: AppSettings.fontSizeText
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 20
			font.bold: true

			onTextEdited: {
				splitText = text;
				bModified = true;
				filterString = JSF.makeFilterString(splitText, AppSettings.appLocale);
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

			function setModel(newmodel) {
				model = newmodel;
			}

			delegate: SwipeDelegate {
				id: delegate
				spacing: 0
				padding: 0
				implicitWidth: Math.max(lstSplitExercises.width, listItem.implicitWidth)
				implicitHeight: listItem.height

				GridLayout {
					id: contentsLayout
					anchors.fill: parent
					rows: 9
					columns: 2
					rowSpacing: 2
					columnSpacing: 2

					TPRadioButton {
						id: optCurrentExercise
						text: qsTr("Exercise #") + "<b>" + (index + 1) + "</b>"
						textColor: "black"
						indicatorColor: "black"
						checked: index === currentModelIndex
						Layout.row: 0
						Layout.column: 0
						Layout.columnSpan: 2
						Layout.minimumWidth: parent.width - 40
						Layout.maximumWidth: parent.width - 40
						width: parent.width - 40

						onClicked: {
							currentModelIndex = index;
						}
					}

					TextField {
						id: txtExerciseName
						text: getExerciseName()
						wrapMode: Text.WordWrap
						readOnly: !bCanEditExercise
						Layout.row: 1
						Layout.column: 0
						Layout.columnSpan: 2
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

						Keys.onReturnPressed: { //Alphanumeric keyboard
							bCanEditExercise = false;
							cboSetType.forceActiveFocus();
						}

						onPressed: (mouse) => { //relay the signal to the delegate
							if (bCanEditExercise) {
								mouse.accepted = true;
								forceActiveFocus();
							}
							else
								mouse.accepted = false;
						}

						onPressAndHold: (mouse) => {
							mouse.accepted = false;
						}

						onEditingFinished: {
							exercisesListModel.setProperty(index, "exerciseName", text);
							bModified = true;
						}

						onTextChanged: {
							if (bCanEditExercise) {
								if (bottomPane.shown) {
									exercisesListModel.setProperty(index, "exerciseName", text);
									bModified = true;
								}
							}
						}

						onActiveFocusChanged: {
							if (activeFocus) {
								bottomPane.shown = false;
								cursorPosition = text.length;
							}
							else
								bCanEditExercise = false;
						}

						RoundButton {
							id: btnEditExercise
							padding: 10
							anchors.left: txtExerciseName.right
							anchors.top: txtExerciseName.top
							height: 25
							width: 25
							enabled: index === currentModelIndex

							Image {
								source: "qrc:/images/"+darkIconFolder+"edit.png"
								anchors.verticalCenter: parent.verticalCenter
								anchors.horizontalCenter: parent.horizontalCenter
								fillMode: Image.PreserveAspectFit
								height: 20
								width: 20
							}

							onClicked: {
								editButtonClicked();
							}
						} //ToolButton btnEditExercise
					} //TextField

					Label {
						text: qsTr("Set Type:")
						Layout.row: 2
						Layout.column: 0
						Layout.leftMargin: 5
						width: listItem.width/3
						wrapMode: Text.WordWrap
					}
					TPComboBox {
						id: cboSetType
						model: setTypes
						Layout.minimumWidth: 110
						currentIndex: parseInt(setType);
						Layout.row: 2
						Layout.column: 1
						Layout.rightMargin: 5
						enabled: index === currentModelIndex

						onActivated: (index) => {
							exercisesListModel.setProperty(currentModelIndex, "setType", currentValue.toString());
							txtNSets.forceActiveFocus();
							bModified = true;
						}
					}

					Label {
						text: qsTr("Number of Sets:")
						Layout.row: 3
						Layout.column: 0
						Layout.leftMargin: 5
						width: listItem.width/2
						Layout.maximumWidth: width
						wrapMode: Text.WordWrap
					}
					SetInputField {
						id: txtNSets
						text: setsNumber
						type: SetInputField.Type.SetType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 3
						Layout.column: 1
						enabled: index === currentModelIndex

						onValueChanged: (str, val) => {
							exercisesListModel.setProperty(index, "setsNumber", str);
							bModified = true;
						}

						onEnterOrReturnKeyPressed: {
							if (txtNReps.visible)
								txtNReps.forceActiveFocus();
							else
								txtNReps1.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Baseline number of reps:")
						Layout.row: 4
						Layout.column: 0
						Layout.leftMargin: 5
						width: cboSetType.currentIndex !== 4 ? listItem.width/2 : listItem.width
						Layout.maximumWidth: width
						wrapMode: Text.WordWrap
					}

					RowLayout {
						Layout.row: 5
						Layout.column: 0
						Layout.columnSpan: 2
						visible: cboSetType.currentIndex === 4

						SetInputField {
							id: txtNReps1
							text: getReps(1)
							type: SetInputField.Type.RepType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: ["",qsTr("Exercise 1:"),"",""]
							enabled: index === currentModelIndex
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								changeRep(1, str);
							}

							onEnterOrReturnKeyPressed: {
								txtNReps2.forceActiveFocus();
							}
						}
						SetInputField {
							id: txtNReps2
							text: getReps(2)
							type: SetInputField.Type.RepType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: ["",qsTr("Exercise 2:"),"",""]
							enabled: index === currentModelIndex
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								changeRep(1, str);
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight1.forceActiveFocus();
							}
						}
					} //RowLayout

					SetInputField {
						id: txtNReps
						text: getReps(0)
						type: SetInputField.Type.RepType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 4
						Layout.column: 1
						Layout.rightMargin: 5
						enabled: index === currentModelIndex
						visible: cboSetType.currentIndex !== 4

						onValueChanged: (str, val) => {
							exercisesListModel.setProperty(index, "repsNumber", str);
							bModified = true;
						}

						onEnterOrReturnKeyPressed: {
							txtNWeight.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Baseline weight ") + AppSettings.weightUnit + ":"
						Layout.row: cboSetType.currentIndex !== 4 ? 5 : 6
						Layout.column: 0
						Layout.leftMargin: 5
						width: listItem.width/2
						Layout.maximumWidth: width
						wrapMode: Text.WordWrap
					}

					RowLayout {
						Layout.row: 7
						Layout.column: 0
						Layout.columnSpan: 2
						visible: cboSetType.currentIndex === 4

						SetInputField {
							id: txtNWeight1
							text: getWeight(1)
							type: SetInputField.Type.WeightType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: [qsTr("Exercise 1:"),"","",""]
							enabled: index === currentModelIndex
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								changeWeight(1, str);
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight2.forceActiveFocus();
							}
						}
						SetInputField {
							id: txtNWeight2
							text: getWeight(2)
							type: SetInputField.Type.WeightType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: [qsTr("Exercise 2:"),"","",""]
							enabled: index === currentModelIndex
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								changeWeight(2, str);
							}
						}
					} //RowLayout

					SetInputField {
						id: txtNWeight
						text: getWeight(0)
						type: SetInputField.Type.WeightType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 5
						Layout.column: 1
						Layout.rightMargin: 5
						enabled: index === currentModelIndex
						visible: cboSetType.currentIndex !== 4

						onValueChanged: (str, val) => {
							exercisesListModel.setProperty(index, "weightValue", str);
							bModified = true;
						}
					}

					ToolButton {
						id: btnAddExercise
						height: 25
						text: qsTr("Add exercise")
						font.capitalization: Font.MixedCase
						font.bold: true
						display: AbstractButton.TextBesideIcon
						visible: index === exercisesListModel.count - 1
						Layout.row: cboSetType.currentIndex !== 4 ? 6 : 8
						Layout.column: 0
						Layout.columnSpan: 2
						Layout.alignment: Qt.AlignCenter
						Layout.minimumWidth: 150
						icon.source: "qrc:/images/"+darkIconFolder+"add-new.png"
						icon.width: 25
						icon.height: 25

						onClicked: {
							if (btnEditExercise.enabled)
								btnEditExercise.clicked();

							appendNewExerciseToDivision();
							lstSplitExercises.positionViewAtIndex(currentModelIndex, ListView.Beginning);
						}
					} //btnAddExercise
				} //GridLayout

				function getReps(n) {
					var idx = repsNumber.indexOf('#')
					switch (n) {
						case 0:
							if (idx !== -1) //Only for a DropSet that was converted from a TrainingDay
								return repsNumber.substring(0, idx);
							else
								return repsNumber;
						case 1:
							return repsNumber.substring(0, idx);
						case 2:
							return repsNumber.substring(idx+1, repsNumber.length);
					}
				}

				function getWeight(n) {
					var idx = weightValue.indexOf('#')
					switch (n) {
						case 0:
							if (idx !== -1) //Only for a DropSet that was converted from a TrainingDay
								return weightValue.substring(0, idx);
							else
								return weightValue;
						case 1:
							return weightValue.substring(0, idx);
						case 2:
							return weightValue.substring(idx+1, weightValue.length);
					}
				}

				function changeRep(n, newRep) {
					const reps = repsNumber.split('#');
					if (reps[n-1] !== newRep) {
						reps[n-1] = newRep;
						exercisesListModel.setProperty(index, "repsNumber", reps[0] + '#' + reps[1]);
						bModified = true;
					}
				}

				function changeWeight(n, newWeight) {
					const weights = weightValue.split('#');
					if (weights[n-1] !== newWeight) {
						weights[n-1] = newWeight;
						exercisesListModel.setProperty(index, "weightValue", weights[0] + '#' + weights[1]);
						bModified = true;
					}
				}

				function getExerciseName() {
					return exerciseName.replace('&', " + ");
				}

				contentItem: Rectangle {
					id: listItem
					width: lstSplitExercises.width - 10
					height: (contentsLayout.rows + 1) * 40
					border.color: "transparent"
					color: "transparent"
					radius: 5
				}

				background: Rectangle {
					id:	backgroundColor
					radius: 5
					color: currentModelIndex === index ? primaryLightColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
				}

				Component.onCompleted: lstSplitExercises.totalHeight += height;

				onClicked: {
					if (currentModelIndex !== index)
						currentModelIndex = index;
				}

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
						text: qsTr("Removing in " + seconds/1000 + "s")
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
					seconds = 4000;
					undoTimer.init(index);
				}
			} //delegate: SwipeDelegate
		} //ListView

		ButtonFlat {
			id: btnSave
			text: qsTr("Save Plan for division ") + splitLetter
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 10
			enabled: bModified

			onClicked: {
				if (bCanEditExercise)
					editButtonClicked();
				saveMesoDivisionPlan();
			}
		}
	} //ColumnLayout

	Component.onCompleted: {
		lstSplitExercises.setModel(exercisesListModel);
		filterString = JSF.makeFilterString(splitText, AppSettings.appLocale);

		if (splitExercises.length === 0) {
			appendNewExerciseToDivision();
			return;
		}

		const exercises = splitExercises.split('|');
		const types = splitSetTypes.split('|');
		const nsets = splitNSets.split('|');
		const nreps = splitNReps.split('|');
		const nweights = splitNWeight.split('|');
		var i = 0;

		while (i < exercises.length) {
			exercisesListModel.append ({ "exerciseName":exercises[i], "setType":types[i],
			"setsNumber":nsets[i], "repsNumber":nreps[i], "weightValue":nweights[i] });
			i++;
		}
		currentModelIndex = exercisesListModel.count - 1;

		if (Qt.platform.os === "android")
			mainwindow.appAboutToBeSuspended.connect(aboutToBeSuspended);
	}

	function appendNewExerciseToDivision() {
		currentModelIndex = exercisesListModel.count;
		exercisesListModel.append ( {"exerciseName":qsTr("Choose exercise..."), "setType":"0",
			"setsNumber":"4", "repsNumber":"12", "weightValue":"20" } );
	}

	function changeModel(name1, name2, nsets, nreps, nweight, multiplesel_opt) {
		if (bCanEditExercise) {
			switch (multiplesel_opt) {
				case 0: exercisesListModel.setProperty(currentModelIndex, "exerciseName", name1 + " - " + name2);
				break;
				case 1: removeExerciseFromName(currentModelIndex, name1 + " - " + name2);
				break;
				case 2: appendExerciseToName(currentModelIndex, name1 + " - " + name2);
				break;
			}
			exercisesListModel.setProperty(currentModelIndex, "setsNumber", nsets.toString());
			exercisesListModel.setProperty(currentModelIndex, "repsNumber", nreps.toString());
			exercisesListModel.setProperty(currentModelIndex, "weightValue", nweight.toString());
			bModified = true;
		}
	}

	function removeExerciseFromName(index, name) {
		var names = exercisesListModel.get(index).exerciseName.split('+');
		var exercisesnames = "";
		for (var i = 0; i < names.length; ++i) {
			names[i] = names[i].trim();
			if (names[i] === name)
				continue;
			exercisesnames += names[i] + " + ";
		}
		exercisesListModel.setProperty(index, "exerciseName", exercisesnames.slice(0, -3)); //remove the last ' + '
	}

	function appendExerciseToName(index, name) {
		var exercisesnames = exercisesListModel.get(index).exerciseName;
		if (exercisesnames.indexOf("...") !== -1)
			exercisesnames = name;
		else
			exercisesnames += " + " + name;
		exercisesListModel.setProperty(index, "exerciseName", exercisesnames);
	}

	function editButtonClicked() {
		bottomPane.shown = !bCanEditExercise;
		bCanEditExercise = !bCanEditExercise;
	}

	function saveMesoDivisionPlan() {
		var exercises = "", types = "", nsets = "", nreps = "",nweights = "";
		const searchRegExp = /\ \+\ /g;
		const replaceWith = '&';
		for (var i = 0; i < exercisesListModel.count; ++i) {
			//Visually, the + sign is better than &. So we work with it on the GUI. We save with & because that's
			//what's required by TrainingDayInfo. If not instances of ' + ' are found, the unmodified string is returned
			exercisesListModel.get(i).exerciseName = exercisesListModel.get(i).exerciseName.replace(searchRegExp, replaceWith);
			exercises += exercisesListModel.get(i).exerciseName + '|';
			types += exercisesListModel.get(i).setType + '|';
			nsets += exercisesListModel.get(i).setsNumber + '|';
			nreps += exercisesListModel.get(i).repsNumber + '|';
			nweights += exercisesListModel.get(i).weightValue + '|';
		}
		Database.updateMesoDivisionComplete(divisionId, splitLetter, splitText,	exercises.slice(0, -1),
					types.slice(0, -1), nsets.slice(0, -1), nreps.slice(0, -1), nweights.slice(0, -1));
		bModified = false;
	}

	function removeExercise(idx) {
		exercisesListModel.remove(idx);
		if (idx === exercisesListModel.count) {
			if (idx > 0)
				--idx;
			else //No more items
				appendNewExerciseToDivision();
		}
		currentModelIndex = idx;
		bModified = true;
	}

	function aboutToBeSuspended() {
		if (bModified)
			btnSave.clicked();
	}
} //Page
