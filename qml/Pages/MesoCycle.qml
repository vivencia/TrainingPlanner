import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vivenciasoftware.qmlcomponents

import "../jsfunctions.js" as JSF
import "../"
import "../Dialogs"
import "../TPWidgets"

TPPage {
	id: mesoPropertiesPage
	objectName: "mesoPage"

	required property int mesoId
	required property int mesoIdx

	property string mesoSplit: mesocyclesModel.get(mesoIdx, 6);
	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date fixedMesoEndDate //Used on newMeso to revert data to the original value gathered from HomePage
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

	property bool bMesoNameOK: false
	property bool bNewMeso: mesoId === -1

	property var calendarChangeDlg: null
	property bool bChangeStartDate
	property bool bPreserveOldCalendar: false
	property bool bPreserveOldCalendarUntilYesterday: false
	property bool bChangedCalendar: false

	Component.onCompleted: {
		JSF.checkWhetherCanCreatePlan();
		if (bNewMeso)
			txtMesoName.forceActiveFocus();
		mesocyclesModel.modifiedChanged.connect(function () { saveMeso(false); });
		mesoSplitModel.modifiedChanged.connect(function () { saveMeso(false); });
	}

	onPageActivated: {
		appDB.setWorkingMeso(mesoIdx);
	}

	onPageDeActivated: {
		if (bNewMeso)
			appDB.scheduleMesocycleRemoval(mesoIdx);
	}

	header: ToolBar {
		height: headerHeight
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
			spacing: 5

			Label {
				text: mesocyclesModel.columnLabel(1)
				font.bold: true
				Layout.alignment: Qt.AlignHCenter
				Layout.topMargin: 10
				color: AppSettings.fontColor
			}
			TPTextInput {
				id: txtMesoName
				text: mesocyclesModel.get(mesoIdx, 1);
				Layout.alignment: Qt.AlignHCenter
				Layout.minimumWidth: parent.width / 2
				Layout.maximumWidth: parent.width - 20
				wrapMode: Text.WordWrap
				ToolTip.text: qsTr("Mesocycle name too short")

				onTextEdited: {
					bMesoNameOK = text.length >= 5;
					ToolTip.visible = !bMesoNameOK;
				}

				onEditingFinished: {
					if (bMesoNameOK)
						bMesoNameOK = mesocyclesModel.set(mesoIdx, 1, text);
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (bNewMeso)
						caldlg.open();
					else
						txtMesoSplit.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (bNewMeso)
						caldlg.open();
					else
						txtMesoSplit.forceActiveFocus();
				}
			}

			Label {
				text: mesocyclesModel.columnLabel(2)
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: AppSettings.fontColor
			}

			TPTextInput {
				id: txtMesoStartDate
				text: runCmd.formatDate(mesocyclesModel.getDate(mesoIdx, 2))
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				readOnly: true

				CalendarDialog {
					id: caldlg
					showDate: calendarStartDate
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: function() {
						bChangeStartDate = true;
						showCalendarChangedDialog();
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
				text: mesocyclesModel.columnLabel(3)
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: AppSettings.fontColor
			}
			TPTextInput {
				id: txtMesoEndDate
				text: runCmd.formatDate(mesocyclesModel.getDate(mesoIdx, 3))
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				readOnly: true

				CalendarDialog {
					id: caldlg2
					showDate: mesocyclesModel.getDate(mesoIdx, 3)
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: function(date) {
						bChangeStartDate = false;
						showCalendarChangedDialog();
						txtMesoSplit.forceActiveFocus();
					}
				}

				TPRoundButton {
					id: btnEndDate
					anchors.left: txtMesoEndDate.right
					anchors.verticalCenter: txtMesoEndDate.verticalCenter
					width: 40
					height: 40
					imageName: "calendar.png"

					onClicked: caldlg2.open();
				}
			}

			Label {
				id: lblnWeeks
				text: mesocyclesModel.columnLabel(5)
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: 50
				Layout.maximumWidth: 50
				color: AppSettings.fontColor
			}

			TPTextInput {
				id: txtMesoNWeeks
				text: mesocyclesModel.get(mesoIdx, 5)
				width: txtMesoEndDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				readOnly: true
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
				ToolTip.text: qsTr("On a mesocycle, there should be at least one rest day(R)")

				property bool bMesoSplitOK: false
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
						if (mesoSplit !== text) {
							mesocyclesModel.set(mesoIdx, 6, text);
							mesoSplit = text;
							JSF.checkWhetherCanCreatePlan();
							showCalendarChangedDialog();
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
				text: mesocyclesModel.columnLabel(7)
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
				contentHeight: txtMesoDrugs.implicitHeight

				TextArea.flickable: TextArea {
					id: txtMesoDrugs
					text: mesocyclesModel.get(mesoIdx, 7)
					color: AppSettings.fontColor

					onEditingFinished: mesocyclesModel.set(mesoIdx, 7, text);
				}
				Component.onCompleted: vBar.position = 0
				ScrollBar.vertical: ScrollBar { id: vBar }
				ScrollBar.horizontal: ScrollBar {}
			}

			Label {
				text: mesocyclesModel.columnLabel(4)
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

	function showCalendarChangedDialog() {
		if (!calendarChangeDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				calendarChangeDlg = component.createObject(mainwindow, { parentPage: mesoPropertiesPage, title:qsTr("Adjust meso calendar?"),
					button1Text: qsTr("Yes"), button2Text: qsTr("No"), customItemSource:"TPAdjustMesoCalendarFrame.qml" });
				calendarChangeDlg.button1Clicked.connect(preserveOldCalenarInfo);
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		calendarChangeDlg.show(-1);
	}

	function preserveOldCalenarInfo() {
		bPreserveOldCalendar = calendarChangeDlg.customBoolProperty1;
		bPreserveOldCalendarUntilYesterday = calendarChangeDlg.customBoolProperty2;
		bChangedCalendar = true;
		if (bChangeStartDate) {
			if(mesocyclesModel.setDate(mesoIdx, 2, caldlg.selectedDate))
				txtMesoStartDate.text = runCmd.formatDate(caldlg.selectedDate);
		}
		else {
			if (mesocyclesModel.setDate(mesoIdx, 3, caldlg2.selectedDate))
				txtMesoEndDate.text = runCmd.formatDate(caldlg2.selectedDate);
		}
		txtMesoNWeeks.text = runCmd.calculateNumberOfWeeks(mesocyclesModel.getDate(mesoIdx, 2), mesocyclesModel.getDate(mesoIdx, 3));
		mesocyclesModel.set(mesoIdx, 5, txtMesoNWeeks.text);
		bPreserveOldCalendar = bPreserveOldCalendarUntilYesterday = false;
	}

	function saveMeso() {
		if (mesocyclesModel.modified || mesoSplitModel.modified) {
			appDB.saveMesocycle(bNewMeso, bChangedCalendar, bPreserveOldCalendar, bPreserveOldCalendarUntilYesterday);
			bNewMeso = false;
			bChangedCalendar = false;
		}
	}
} //Page
