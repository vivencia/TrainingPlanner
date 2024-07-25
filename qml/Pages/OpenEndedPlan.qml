import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vivenciasoftware.qmlcomponents

import "../"
import "../jsfunctions.js" as JSF
import "../Dialogs"
import "../TPWidgets"

Page {
	id: openEndedPage
	objectName: "openEndedPage"
	width: windowWidth
	height: windowHeight

	required property int mesoId
	required property int mesoIdx

	property string mesoSplit: mesocyclesModel.get(mesoIdx, 6);
	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date fixedMesoEndDate //Used on newMeso to revert data to the original value gathered from HomePage
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

	property bool bMesoSplitOK: false
	property bool bStartDateChanged: false
	property bool bEndDateChanged: false

	property bool bNewMeso: mesoId === -1

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	header: ToolBar {
		height: 45
		enabled: !bNewMeso

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			text: qsTr("Calendar")
			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 20
			}
			imageSource: "edit-mesocycle.png"

			onClicked: appDB.getMesoCalendar(true);
		}
	}

	ScrollView {
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

		ColumnLayout {
			id: colMain
			anchors.fill: parent
			spacing: 15

			Label {
				text: mesocyclesModel.columnLabel(2)
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.topMargin: 10
				color: AppSettings.fontColor
			}

			TPTextInput {
				id: txtMesoStartDate
				text: runCmd.formatDate(mesocyclesModel.getDate(mesoIdx, 2))
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				readOnly: true
				ToolTip.text: qsTr("A start date is necessary for record keeping")

				CalendarDialog {
					id: caldlg
					showDate: calendarStartDate
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate

					onOpenedChanged: {
						if (bNewMeso && !opened)
							txtMesoStartDate.ToolTip.visible = !bStartDateChanged;
					}

					onDateSelected: function(date) {
						bStartDateChanged = mesocyclesModel.setDate(mesoIdx, 2, date);
						if (bStartDateChanged) {
							txtMesoStartDate.text = runCmd.formatDate(date);
						}
						if (bNewMeso)
							caldlg2.open();
					}
				}

				TPRoundButton {
					id: btnStartDate
					anchors.left: txtMesoStartDate.right
					anchors.verticalCenter: txtMesoStartDate.verticalCenter
					width: 40
					height: 40
					imageName: "calendar.png"

					onClicked: caldlg.open();
				}
			}

			Label {
				text: mesocyclesModel.columnLabel(6)
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: AppSettings.fontColor
			}
			RegularExpressionValidator {
				id: regEx
				regularExpression: new RegExp(/[A-FR]+/);
			}
			TPTextInput {
				id: txtMesoSplit
				text: mesocyclesModel.get(mesoIdx, 6)
				validator: regEx
				width: txtMesoStartDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				ToolTip.text: qsTr("There should be at least one rest day(R) in any program")

				TPRoundButton {
					id: btnTrainingSplit
					width: 40
					height: 40
					anchors {
						left: txtMesoSplit.right
						verticalCenter: txtMesoSplit.verticalCenter
						leftMargin: 10
					}
					imageName: paneTrainingSplit.visible ? "fold-up.png" : "fold-down.png"
					onClicked: paneTrainingSplit.shown = !paneTrainingSplit.shown
				}

				onTextEdited: {
					bMesoSplitOK = text.indexOf('R') !== -1;
					ToolTip.visible = !bMesoSplitOK;
				}

				onEditingFinished: {
					if (bMesoSplitOK) {
						bMesoSplitOK = mesocyclesModel.set(mesoIdx, 6, text);
						if (bMesoSplitOK) {
							mesoSplit = text;
							JSF.checkWhetherCanCreatePlan();
						}
					}
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (!paneTrainingSplit.shown)
						btnTrainingSplit.clicked();
					JSF.moveFocusToNextField('0');
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (!paneTrainingSplit.shown)
						btnTrainingSplit.clicked();
					JSF.moveFocusToNextField('0');
				}
			}

			Pane {
				id: paneTrainingSplit
				Layout.fillWidth: true
				Layout.leftMargin: 20
				property bool shown: false
				visible: height > 0
				height: shown ? implicitHeight : 0
				Behavior on height {
					NumberAnimation {
						duration: 300
						easing.type: Easing.InOutBack
					}
				}
				padding: 0

				background: Rectangle {
					color: "transparent"
				}

				GridLayout {
					anchors.fill: parent
					columns: 2
					rows: 7
					Label {
						text: qsTr("Day A: ")
						font.bold: true
						Layout.row: 0
						Layout.column: 0
						color: AppSettings.fontColor
						visible: txtSplitA.visible
					}
					TPTextInput {
						id: txtSplitA
						text: mesoSplitModel.get(mesoIdx, 2)
						Layout.row: 0
						Layout.column: 1
						Layout.fillWidth: true
						Layout.rightMargin: 20
						visible: mesoSplit.indexOf('A') !== -1

						onEditingFinished: {
							if (mesoSplitModel.set(mesoIdx, 2, text))
								JSF.checkWhetherCanCreatePlan();
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('A');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('A');
						}
					}

					Label {
						text: qsTr("Day B: ")
						font.bold: true
						Layout.row: 1
						Layout.column: 0
						color: AppSettings.fontColor
						visible: txtSplitB.visible
					}
					TPTextInput {
						id: txtSplitB
						text: mesoSplitModel.get(mesoIdx, 3)
						Layout.row: 1
						Layout.column: 1
						Layout.fillWidth: true
						Layout.rightMargin: 20
						visible: mesoSplit.indexOf('B') !== -1

						onEditingFinished: {
							if (mesoSplitModel.set(mesoIdx, 3, text))
								JSF.checkWhetherCanCreatePlan();
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('B');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('B');
						}
					}

					Label {
						text: qsTr("Day C: ")
						font.bold: true
						Layout.row: 2
						Layout.column: 0
						color: AppSettings.fontColor
						visible: txtSplitC.visible
					}
					TPTextInput {
						id: txtSplitC
						text: mesoSplitModel.get(mesoIdx, 4)
						Layout.row: 2
						Layout.column: 1
						Layout.fillWidth: true
						Layout.rightMargin: 20
						visible: mesoSplit.indexOf('C') !== -1

						onEditingFinished: {
							if (mesoSplitModel.set(mesoIdx, 4, text))
								JSF.checkWhetherCanCreatePlan();
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('C');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('C');
						}
					}

					Label {
						text: qsTr("Day D: ")
						font.bold: true
						Layout.row: 3
						Layout.column: 0
						color: AppSettings.fontColor
						visible: txtSplitD.visible
					}
					TPTextInput {
						id: txtSplitD
						text: mesoSplitModel.get(mesoIdx, 5)
						Layout.row: 3
						Layout.column: 1
						Layout.fillWidth: true
						Layout.rightMargin: 20
						visible: mesoSplit.indexOf('D') !== -1

						onEditingFinished: {
							if (mesoSplitModel.set(mesoIdx, 5, text))
								JSF.checkWhetherCanCreatePlan();
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('D');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('D');
						}
					}

					Label {
						text: qsTr("Day E: ")
						font.bold: true
						Layout.row: 4
						Layout.column: 0
						color: AppSettings.fontColor
						visible: txtSplitE.visible
					}
					TPTextInput {
						id: txtSplitE
						text: mesoSplitModel.get(mesoIdx, 6)
						Layout.row: 4
						Layout.column: 1
						Layout.fillWidth: true
						Layout.rightMargin: 20
						visible: mesoSplit.indexOf('E') !== -1

						onEditingFinished: {
							if (mesoSplitModel.set(mesoIdx, 6, text))
								JSF.checkWhetherCanCreatePlan();
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('E');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('E');
						}
					}

					Label {
						text: qsTr("Day F: ")
						font.bold: true
						Layout.row: 5
						Layout.column: 0
						color: AppSettings.fontColor
						visible: txtSplitF.visible
					}
					TPTextInput {
						id: txtSplitF
						text: mesoSplitModel.get(mesoIdx, 7)
						Layout.row: 5
						Layout.column: 1
						Layout.fillWidth: true
						Layout.rightMargin: 20
						visible: mesoSplit.indexOf('F') !== -1

						onEditingFinished: {
							if (mesoSplitModel.set(mesoIdx, 7, text))
								JSF.checkWhetherCanCreatePlan();
						}

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('F');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('F');
						}
					}

					TPButton {
						id: btnCreateExercisePlan
						text: qsTr("Exercises Planner")
						Layout.row: 6
						Layout.column: 0
						Layout.columnSpan: 2
						Layout.alignment: Qt.AlignCenter

						onClicked: appDB.createExercisesPlannerPage();
					} //TPButton
				} //GridLayout
			} //Pane

			Label {
				text: qsTr("Notes about this program")
				font.bold: true
				Layout.leftMargin: 5
				color: AppSettings.fontColor
			}
			Flickable {
				Layout.fillWidth: true
				Layout.rightMargin: 20
				Layout.leftMargin: 10
				height: Math.min(contentHeight, 60)
				width: parent.width - 20
				contentHeight: txtMesoNotes.implicitHeight

				TextArea.flickable: TextArea {
					id: txtMesoNotes
					text: mesocyclesModel.get(mesoIdx, 4)
					color: AppSettings.fontColor

					onEditingFinished: mesocyclesModel.set(mesoIdx, 4, text);
				}

				Component.onCompleted: vBar2.position = 0
				ScrollBar.vertical: ScrollBar { id: vBar2 }
				ScrollBar.horizontal: ScrollBar {}
			}
		} //ColumnLayout
	} //ScrollView

	Component.onCompleted: {
		JSF.checkWhetherCanCreatePlan();
		openEndedPage.StackView.onDeactivating.connect(pageDeActivation);
		openEndedPage.StackView.activating.connect(pageActivation);
		mesocyclesModel.modifiedChanged.connect(saveMeso);
		mesoSplitModel.modifiedChanged.connect(saveMeso);
	}

	function changeMuscularGroup(splitletter: string, description: string) {
		switch (splitletter) {
			case 'A': txtSplitA.text = description; break;
			case 'B': txtSplitB.text = description; break;
			case 'C': txtSplitC.text = description; break;
			case 'D': txtSplitD.text = description; break;
			case 'E': txtSplitE.text = description; break;
			case 'F': txtSplitF.text = description; break;
		}
		appDB.updateMesoSplit(txtSplitA.text, txtSplitB.text, txtSplitC.text, txtSplitD.text, txtSplitE.text, txtSplitF.text);
	}

	function pageDeActivation() {
		if (bNewMeso)
			appDB.removeMesocycle(mesoIdx);
	}

	function pageActivation() {
		appDB.setWorkingMeso(mesoIdx);
	}

	function saveMeso() {
		if (mesocyclesModel.modified || mesoSplitModel.modified) {
			appDB.saveMesocycle(bNewMeso);
			bNewMeso = false;
		}
	}
} //Page
