import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Frame {
	id: paneSplit
	required property int mesoId
	required property int mesoIdx
	required property string splitLetter

	property bool bCanEditExercise: false
	property int seconds: 0
	property bool bModified: false
	property string filterString: ""

	property string prevMesoName: ""
	property int prevMesoId: -1

	implicitWidth: parent.width
	implicitHeight: splitLayout.height

	padding: 0
	spacing: 0
	Layout.leftMargin: 5

	Timer {
		id: undoTimer
		interval: 1000
		property int idxToRemove

		onTriggered: {
			if ( seconds === 0 ) {
				undoTimer.stop();
				mesoSplitModel.removeExercise(splitLetter, idxToRemove);
				mesoSplitModel.removeSetType(splitLetter, idxToRemove);
				mesoSplitModel.removeSetsNumber(splitLetter, idxToRemove);
				mesoSplitModel.removeReps(splitLetter, idxToRemove);
				mesoSplitModel.removeWeight(splitLetter, idxToRemove);
				if (idxToRemove > 0)
					--idxToRemove;
				if (mesoSplitModel.count === 0)
					appendNewExerciseToDivision();
				mesoSplitModel.setCurrentEntry(splitLetter, idxToRemove);
				bModified = true;
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

	TPBalloonTip {
		id: msgDlgImport
		title: qsTr("Import Exercises Plan?")
		message: qsTr("Import the exercises plan for training division <b>") + splitLetter +
						 qsTr("</b> from <b>") + prevMesoName + "</b>?"
		button1Text: qsTr("Yes")
		button2Text: qsTr("No")
		imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

		onButton1Clicked: appDB.loadSplitFromPreviousMeso(mesoId, prevMesoId, splitLetter);
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
			text: mesoSplitModel.getMuscularGroup(splitLetter, mesoSplitModel.currentEntry(splitLetter))
			font.pixelSize: AppSettings.fontSizeText
			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 20
			font.bold: true

			onTextEdited: {
				mesoSplitModel.changeMuscularGroup(splitLetter, text, mesoSplitModel.currentEntry(splitLetter));
				bModified = true;
				filterString = exercisesListModel.makeFilterString(text);
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
				model = newmodel.nEntries
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
						checked: index === mesoSplitModel.currentEntry(splitLetter)
						Layout.row: 0
						Layout.column: 0
						Layout.columnSpan: 2
						Layout.minimumWidth: parent.width - 40
						Layout.maximumWidth: parent.width - 40
						width: parent.width - 40

						onClicked: {
							mesoSplitModel.setCurrentEntry(splitLetter, index);
						}
					}

					TextField {
						id: txtExerciseName
						text: mesoSplitModel.getExercise(splitLetter, index)
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
							mesoSplitModel.changeExercise(splitLetter, text, index);
							bModified = true;
						}

						onTextChanged: {
							if (bCanEditExercise) {
								if (bottomPane.shown) {
									mesoSplitModel.changeExercise(splitLetter, text, index);
									bModified = true;
								}
							}
							else
								cursorPosition = 0;
						}

						onActiveFocusChanged: {
							if (activeFocus) {
								bottomPane.shown = false;
								cursorPosition = text.length;
							}
							else {
								bCanEditExercise = false;
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
							enabled: index === mesoSplitModel.currentEntry(splitLetter)

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
						currentIndex: mesoSplitModel.getSetType(splitLetter, index);
						Layout.row: 2
						Layout.column: 1
						Layout.rightMargin: 5
						enabled: index === mesoSplitModel.currentEntry(splitLetter)

						onActivated: (index) => {
							mesoSplitModel.changeSetType(splitLetter, index, mesoSplitModel.currentEntry(splitLetter));
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
						text: mesoSplitModel.getSetsNumber(splitLetter, index)
						type: SetInputField.Type.SetType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 3
						Layout.column: 1
						enabled: index === mesoSplitModel.currentEntry(splitLetter)

						onValueChanged: (str, val) => {
							mesoSplitModel.changeSetsNumber(splitLetter, str, index);
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
							text: mesoSplitModel.getReps(splitLetter, 0, index)
							type: SetInputField.Type.RepType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: ["",qsTr("Exercise 1:"),"",""]
							enabled: index === mesoSplitModel.currentEntry(splitLetter)
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								mesoSplitModel.changeReps(splitLetter, 0, str, index);
							}

							onEnterOrReturnKeyPressed: {
								txtNReps2.forceActiveFocus();
							}
						}
						SetInputField {
							id: txtNReps2
							text: mesoSplitModel.getReps(splitLetter, 1, index)
							type: SetInputField.Type.RepType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: ["",qsTr("Exercise 2:"),"",""]
							enabled: index === mesoSplitModel.currentEntry(splitLetter)
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								mesoSplitModel.changeReps(splitLetter, 1, str, index);
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight1.forceActiveFocus();
							}
						}
					} //RowLayout

					SetInputField {
						id: txtNReps
						text: mesoSplitModel.getReps(splitLetter, 0, index)
						type: SetInputField.Type.RepType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 4
						Layout.column: 1
						Layout.rightMargin: 5
						enabled: index === mesoSplitModel.currentEntry(splitLetter)
						visible: cboSetType.currentIndex !== 4

						onValueChanged: (str, val) => {
							mesoSplitModel.changeReps(splitLetter, 0, str, index);
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
							text: mesoSplitModel.getWeight(splitLetter, 0, index)
							type: SetInputField.Type.WeightType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: [qsTr("Exercise 1:"),"","",""]
							enabled: index === mesoSplitModel.currentEntry(splitLetter)
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								mesoSplitModel.changeWeight(splitLetter, 0, str, index);
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight2.forceActiveFocus();
							}
						}
						SetInputField {
							id: txtNWeight2
							text: mesoSplitModel.getWeight(splitLetter, 1, index)
							type: SetInputField.Type.WeightType
							nSetNbr: 0
							availableWidth: listItem.width*0.49
							alternativeLabels: [qsTr("Exercise 2:"),"","",""]
							enabled: index === mesoSplitModel.currentEntry(splitLetter)
							fontPixelSize: AppSettings.fontSizeText * 0.8

							onValueChanged: (str, val) => {
								mesoSplitModel.changeWeight(splitLetter, 1, str, index);
							}
						}
					} //RowLayout

					SetInputField {
						id: txtNWeight
						text: mesoSplitModel.getWeight(splitLetter, index, 0)
						type: SetInputField.Type.WeightType
						nSetNbr: 0
						availableWidth: listItem.width / 3
						showLabel: false
						Layout.row: 5
						Layout.column: 1
						Layout.rightMargin: 5
						enabled: index === mesoSplitModel.currentEntry(splitLetter)
						visible: cboSetType.currentIndex !== 4

						onValueChanged: (str, val) => {
							mesoSplitModel.changeWeight(splitLetter, 0, str, index);
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
						visible: index === mesoSplitModel.nEntries(splitLetter) - 1
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
							lstSplitExercises.positionViewAtIndex(mesoSplitModel.currentEntry(splitLetter), ListView.Beginning);
						}
					} //btnAddExercise
				} //GridLayout

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
					color: mesoSplitModel.currentEntry(splitLetter) === index ? primaryLightColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
				}

				Component.onCompleted: lstSplitExercises.totalHeight += height;

				onClicked:
					mesoSplitModel.setCurrentEntry(splitLetter, index);

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
				appDB.pass_object(mesoSplitModel);
				appDB.updateMesoSplitComplete(mesoIdx, splitLetter);
				bModified = false;
			}
		}
	} //ColumnLayout

	Component.onCompleted: {
		function readyToProceed() {
			appDB.qmlReady.disconnect(readyToProceed);
			lstSplitExercises.setModel(exercisesListModel);
		}

		if (exercisesListModel.count === 0)
		{
			appDB.qmlReady.connect(readyToProceed);
			loadExercises();
		}
		else
			readyToProceed();

		if (mesoSplitModel.nEntries(splitLetter) === 0) {
			prevMesoId = mesocyclesModel.getPreviousMesoId(mesoId);
			if (prevMesoId >= 0) {
				if (appDB.previousMesoHasPlan(prevMesoId, splitLetter)) {
					prevMesoName = mesocyclesModel.getMesoInfo(prevMesoId, DBMesocyclesModel.mesoNameRole);
					msgDlgImport.show((mainwindow.height - msgDlgImport.height) / 2)
					mesoSplitModel.setCurrentEntry(splitLetter, 0);
				}
			}
			else
				appendNewExerciseToDivision();
		}

		if (Qt.platform.os === "android")
			mainwindow.appAboutToBeSuspended.connect(aboutToBeSuspended);
	}

	function appendNewExerciseToDivision() {
		mesoSplitModel.addExercise(splitLetter, qsTr("Choose exercise..."), mesoSplitModel.currentEntry(splitLetter));
		mesoSplitModel.addType(splitLetter, "0", mesoSplitModel.currentEntry(splitLetter));
		mesoSplitModel.addSetsNumber(splitLetter, "4", mesoSplitModel.currentEntry(splitLetter));
		mesoSplitModel.addReps(splitLetter, "12", mesoSplitModel.currentEntry(splitLetter));
		mesoSplitModel.addWeight(splitLetter, "20", mesoSplitModel.currentEntry(splitLetter));
	}

	function changeModel(name1, name2, nsets, nreps, nweight, multiplesel_opt) {
		if (bCanEditExercise) {
			mesoSplitModel.changeExercise(splitLetter, name1 + " - " + name2, mesoSplitModel.currentEntry(splitLetter), multiplesel_opt !== 2);
			mesoSplitModel.changeSetsNumber(splitLetter, nsets, mesoSplitModel.currentEntry(splitLetter), multiplesel_opt !== 2 );
			mesoSplitModel.changeReps(splitLetter, nreps, mesoSplitModel.currentEntry(splitLetter), multiplesel_opt !== 2);
			mesoSplitModel.changeWeight(splitLetter, nweight, mesoSplitModel.currentEntry(splitLetter), multiplesel_opt !== 2);
			bModified = true;
		}
	}

	function editButtonClicked() {
		bottomPane.shown = !bCanEditExercise;
		bCanEditExercise = !bCanEditExercise;
	}

	function aboutToBeSuspended() {
		if (bModified)
			btnSave.clicked();
	}
} //Page
