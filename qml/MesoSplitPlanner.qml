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

	property string filterString: ""

	property string prevMesoName: ""
	property int prevMesoId: -1
	property var setTypesModel: [ { text:qsTr("Regular"), value:0 }, { text:qsTr("Pyramid"), value:1 }, { text:qsTr("Drop Set"), value:2 },
							{ text:qsTr("Cluster Set"), value:3 }, { text:qsTr("Giant Set"), value:4 }, { text:qsTr("Myo Reps"), value:5 } ]

	signal requestSimpleExercisesList(Item requester, var bVisible, int id)

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
			if ( seconds === 0 ) {
				undoTimer.stop();
				splitModel.removeExercise(idxToRemove);
				if (idxToRemove > 0)
					--idxToRemove;
				if (splitModel.count === 0)
					appendNewExerciseToDivision();
				splitModel.setsplitModel.currentRow(idxToRemove);
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
						Layout.minimumWidth: parent.width - 40
						Layout.maximumWidth: parent.width - 40
						width: parent.width - 40

						onClicked: {
							splitModel.currentRow = index;
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

						Keys.onReturnPressed: { //Alphanumeric keyboard
							cboSetType.forceActiveFocus();
						}

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
							requestSimpleExercisesList(paneSplit, !readOnly, 0);
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

							onClicked: {
								txtExerciseName.readOnly = !txtExerciseName.readOnly;
							}
						} //ToolButton btnEditExercise
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
							model: setTypesModel
							currentIndex: setType
							enabled: index === splitModel.currentRow
							Layout.minimumWidth: 110
							Layout.rightMargin: 5

							onActivated: (index) => {
								setType = index;
								txtNSets.forceActiveFocus();
								parentItem.bEnableMultipleSelection = setType === 4;
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

							onValueChanged: (str) => {
								setsNumber = str;
							}

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
						Layout.topMargin: 15
						Layout.bottomMargin: 10

						Label {
							text: qsTr("Exercise 1")
							font.bold: true
							Layout.alignment: Qt.AlignCenter
							Layout.leftMargin: listItem.width/6
						}
						Label {
							text: qsTr("Exercise 2")
							font.bold: true
							Layout.alignment: Qt.AlignRight
							Layout.leftMargin: listItem.width/6
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
						}

						SetInputField {
							id: txtNReps
							text: setsReps1
							type: SetInputField.Type.RepType
							availableWidth: listItem.width/3
							showLabel: false
							enabled: index === splitModel.currentRow
							Layout.rightMargin: 5

							onValueChanged: (str) => {
								setsReps1 = str;
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight.forceActiveFocus();
							}
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

							onValueChanged: (str) => {
								setsReps1 = str;
							}

							onEnterOrReturnKeyPressed: {
								txtNReps2.forceActiveFocus();
							}
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

							onValueChanged: (str) => {
								setsReps2 = str;
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight1.forceActiveFocus();
							}
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

							onValueChanged: (str) => {
								setsWeight1 = str;
							}
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

							onValueChanged: (str) => {
								setsWeight1 = str;
							}

							onEnterOrReturnKeyPressed: {
								txtNWeight2.forceActiveFocus();
							}
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

							onValueChanged: (str) => {
								setsWeight2 = str;
							}
						}
					} //RowLayout

					ToolButton {
						id: btnAddExercise
						height: 25
						text: qsTr("Add exercise")
						font.capitalization: Font.MixedCase
						font.bold: true
						display: AbstractButton.TextBesideIcon
						visible: index === splitModel.count - 1
						Layout.alignment: Qt.AlignCenter
						Layout.minimumWidth: 150
						icon.source: "qrc:/images/"+darkIconFolder+"add-new.png"
						icon.width: 25
						icon.height: 25

						onClicked: {
							if (btnEditExercise.enabled)
								btnEditExercise.clicked();

							appendNewExerciseToDivision();
							lstSplitExercises.currentIndex = splitModel.currentRow;
							lstSplitExercises.positionViewAtIndex(splitModel.currentRow, ListView.Center);
						}
					} //btnAddExercise
				} //ColumnLayout

				contentItem: Rectangle {
					id: listItem
					width: lstSplitExercises.width - 10
					border.color: "transparent"
					color: "transparent"
					radius: 5

					Component.onCompleted: { //Each layout row(9) * 32(height per row) + 32(extra space)
						height = cboSetType.currentIndex !== 4 ? 320 : 420;
					}
				}

				background: Rectangle {
					id:	backgroundColor
					radius: 5
					color: splitModel.currentRow === index ? primaryLightColor : index % 2 === 0 ? listEntryColor1 : listEntryColor2
				}

				Component.onCompleted: lstSplitExercises.totalHeight += height;

				onClicked: {
					splitModel.currentRow = index;
					parentItem.bEnableMultipleSelection = setType === 4;
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
			enabled: splitModel.splitModified

			onClicked: {
				appDB.pass_object(splitModel);
				appDB.updateMesoSplitComplete(splitLetter);
			}
		}
	} //ColumnLayout

	Component.onCompleted: {
		if (Qt.platform.os === "android")
			mainwindow.appAboutToBeSuspended.connect(aboutToBeSuspended);
	}

	function init() {
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
		filterString = exercisesListModel.makeFilterString(txtSplit.text);
	}

	function appendNewExerciseToDivision() {
		splitModel.addExercise(qsTr("Choose exercise..."), 0, "4", "12", "20");
	}

	property int lastAdded: 0
	function changeModel(name1, name2, nsets, nreps, nweight, multiplesel_opt) {
			splitModel.changeExercise(name1, name2, nsets, nreps, nweight, multiplesel_opt);
	}

	function aboutToBeSuspended() {
		if (splitModel.splitModified)
			btnSave.clicked();
	}
} //Page
