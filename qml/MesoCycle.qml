import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vivenciasoftware.qmlcomponents

import "jsfunctions.js" as JSF

Page {
	id: mesoPropertiesPage
	objectName: "mesoPage"
	width: windowWidth
	height: windowHeight

	required property int mesoId
	required property int mesoIdx
	required property date mesoStartDate
	required property date mesoEndDate

	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date fixedMesoEndDate //Used on newMeso to revert data to the original value gathered from HomePage
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

	property string mesoSplit: mesocyclesModel.get(mesoIdx, 6)

	property bool bMesoNameOK: true
	property bool bStartDateChanged: false
	property bool bEndDateChanged: false
	property bool bMesoSplitChanged: false

	property bool bNewMeso: mesoId === -1
	property bool bModified: false

	property var mesoStatisticsPage: null

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
		enabled: !bNewMeso && !bModified

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
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"edit-mesocycle.png"

			onClicked: appDB.getMesoCalendar(true);
		}

TPBalloonTip {
			id: tertetre2

			function init() {
				message = runCmd.uriToOpen();
				show(-1);
			}
		}
		TPButton {
			text: qsTr("Statistics")
			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: 20
			}
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"statistics.png"

			onClicked: tertetre2.init();
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
					bMesoNameOK= text.length >= 5;
					if (bMesoNameOK) {
						if (text !== mesocyclesModel.get(mesoIdx, 1))
							bModified = true;
					}
					ToolTip.visible = !bMesoNameOK;
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
				ToolTip.text: qsTr("A start date for the mesocycle is necessary")

				CalendarDialog {
					id: caldlg
					showDate: calendarStartDate
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					windowTitle: bNewMeso ? qsTr("Please select the initial date for the mesocycle ") + txtMesoName.text : ""

					onOpenedChanged: {
						if (bNewMeso && !opened)
							txtMesoStartDate.ToolTip.visible = !bStartDateChanged;
					}

					onDateSelected: function(date) {
						bStartDateChanged = date !== mesocyclesModel.getDate(mesoIdx, 2);
						if (bStartDateChanged) {
							mesoStartDate = date;
							txtMesoStartDate.text = runCmd.formatDate(mesoStartDate);
							txtMesoNWeeks.text = runCmd.calculateNumberOfWeeks(mesoStartDate, mesoEndDate);
							bModified = true;
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
				ToolTip.text: qsTr("An end date for the mesocycle is necessary")

				Keys.onReturnPressed: {
					if (btnSaveMeso.enabled)
						btnSaveMeso.clicked();
				}

				CalendarDialog {
					id: caldlg2
					showDate: mesoEndDate
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					windowTitle: bNewMeso ? qsTr("Please select the end date for the mesocycle ") + txtMesoName.text : ""

					onOpenedChanged: {
						if (bNewMeso && !opened)
							txtMesoEndDate.ToolTip.visible = !bEndDateChanged;
					}

					onDateSelected: function(date) {
						bEndDateChanged = date !== mesocyclesModel.getDate(mesoIdx, 3);
						if (bEndDateChanged) {
							mesoEndDate = date;
							txtMesoEndDate.text = runCmd.formatDate(mesoEndDate);
							txtMesoNWeeks.text = runCmd.calculateNumberOfWeeks(mesoStartDate, mesoEndDate);
							bModified = true;
						}
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
					bMesoSplitChanged = text !== mesocyclesModel.get(mesoIdx, 6);
					if (bMesoSplitChanged) {
						ToolTip.visible = text.indexOf('R') === -1;
						bModified = true;
						mesoSplit = text;
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

			Frame {
				id: frmMesoAdjust
				Layout.fillWidth: true
				Layout.rightMargin: 20
				Layout.leftMargin: 5
				visible: bNewMeso ? false : bStartDateChanged || bEndDateChanged || bMesoSplitChanged
				padding: 0
				spacing: 0
				height: 300

				background: Rectangle {
					border.color: AppSettings.fontColor
					color: "transparent"
					radius: 6
				}

				ColumnLayout {
					id: layoutSplit
					anchors.fill: parent
					spacing: 0

					TPCheckBox {
						id: chkPreserveOldCalendar
						text: qsTr("Preserve previous calendar information?")
						checked: false
						Layout.fillWidth: true
						Layout.leftMargin: 5

						onClicked: {
							if (checked) {
								if (!optPreserveOldCalendar.checked && !optPreserveOldCalendarUntilYesterday.checked)
									optPreserveOldCalendar.checked = true;
							}
						}
					} //TPCheckBox

					TPRadioButton {
						id: optPreserveOldCalendar
						text: qsTr("All of the old information")
						enabled: chkPreserveOldCalendar.checked
						checked: false
					}

					TPRadioButton {
						id: optPreserveOldCalendarUntilYesterday
						text: qsTr("Up until yesterday - ") + runCmd.formatDate(runCmd.getDayBefore(new Date()));
						checked: false
						enabled: chkPreserveOldCalendar.checked //&& isDateWithinMeso(today)
					}
				} // ColumnLayout
			} //Frame

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
							if ( text !== mesoSplitModel.get(mesoIdx, 2) ) {
								bModified = true;
								if (text.length >=3)
									JSF.checkWhetherCanCreatePlan();
							}
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
							if ( text !== mesoSplitModel.get(mesoIdx, 3) ) {
								bModified = true;
								if (text.length >=3)
									JSF.checkWhetherCanCreatePlan();
							}
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
							if ( text !== mesoSplitModel.get(mesoIdx, 4) ) {
								bModified = true;
								if (text.length >=3)
									JSF.checkWhetherCanCreatePlan();
							}
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
							if ( text !== mesoSplitModel.get(mesoIdx, 5) ) {
								bModified = true;
								if (text.length >=3 )
									JSF.checkWhetherCanCreatePlan();
							}
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
							if ( text !== mesoSplitModel.get(mesoIdx, 6) ) {
								bModified = true;
								if (text.length >=3 )
									JSF.checkWhetherCanCreatePlan();
							}
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
							if ( text !== mesoSplitModel.get(mesoIdx, 7) ) {
								bModified = true;
								if (text.length >=3 )
									JSF.checkWhetherCanCreatePlan();
							}
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

					onEditingFinished: {
						if ( text !== mesocyclesModel.get(mesoIdx, 7) )
							bModified = true;
					}
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

					onEditingFinished: {
						if ( text !== mesocyclesModel.get(mesoIdx, 4) )
							bModified = true;
					}
				}

				Component.onCompleted: vBar2.position = 0
				ScrollBar.vertical: ScrollBar { id: vBar2 }
				ScrollBar.horizontal: ScrollBar {}
			}
		} //ColumnLayout
	} //ScrollView

	Component.onCompleted: {
		JSF.checkWhetherCanCreatePlan();
		if (bNewMeso)
			txtMesoName.forceActiveFocus();
		mesoPropertiesPage.StackView.onDeactivating.connect(pageDeActivation);
		mesoPropertiesPage.StackView.activating.connect(pageActivation);
	}

	footer: ToolBar {
		id: mesoCycleToolBar
		width: parent.width
		height: 55

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
			id: btnRevert
			text: qsTr("Cancel alterations")
			anchors.left: parent.left
			anchors.leftMargin: 5
			anchors.verticalCenter: parent.verticalCenter
			textUnderIcon: true
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"revert-day.png"
			enabled: bModified

			onClicked: {
				if (!bNewMeso) {
					txtMesoName.text = Qt.binding(function() { return mesocyclesModel.get(mesoIdx, 1) });
					mesoStartDate = mesocyclesModel.getDate(mesoIdx, 2);
					mesoEndDate = mesocyclesModel.getDate(mesoIdx, 3);
					txtMesoNotes.text = Qt.binding(function() { return mesocyclesModel.get(mesoIdx, 4) });
					txtMesoNWeeks.text = Qt.binding(function() { return mesocyclesModel.get(mesoIdx, 5) });
					txtMesoSplit.text = Qt.binding(function() { return mesocyclesModel.get(mesoIdx, 6) });
					txtMesoDrugs.text = Qt.binding(function() { return mesocyclesModel.get(mesoIdx, 7) });
					txtSplitA.text = Qt.binding(function() { return mesoSplitModel.get(mesoIdx, 2) });
					txtSplitB.text = Qt.binding(function() { return mesoSplitModel.get(mesoIdx, 3) });
					txtSplitC.text = Qt.binding(function() { return mesoSplitModel.get(mesoIdx, 4) });
					txtSplitD.text = Qt.binding(function() { return mesoSplitModel.get(mesoIdx, 5) });
					txtSplitE.text = Qt.binding(function() { return mesoSplitModel.get(mesoIdx, 6) });
					txtSplitF.text = Qt.binding(function() { return mesoSplitModel.get(mesoIdx, 7) });
				}
				else {
					mesoStartDate = calendarStartDate;
					mesoEndDate = fixedMesoEndDate;
					txtMesoName.clear();
					txtMesoNWeeks.text = runCmd.calculateNumberOfWeeks(calendarStartDate, fixedMesoEndDate);
					txtMesoSplit.clear();
					txtMesoDrugs.clear();
					txtMesoNotes.clear();
					txtSplitA.clear();
					txtSplitB.clear();
					txtSplitC.clear();
					txtSplitD.clear();
					txtSplitE.clear();
					txtSplitF.clear();
				}
				bModified = false;
				bStartDateChanged = bEndDateChanged = bMesoSplitChanged = false;
			}
		} //btnRevert

		TPButton {
			id: btnSaveMeso
			text: qsTr("Save Information")
			anchors.right: parent.right
			anchors.rightMargin: 5
			anchors.verticalCenter: parent.verticalCenter
			textUnderIcon: true
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"save-day.png"
			enabled: bNewMeso ? bMesoNameOK : bModified

			onClicked: {
				var changeCalendar = false;
				if (bStartDateChanged || bEndDateChanged || bMesoSplitChanged) {
					changeCalendar = true;
					bStartDateChanged = bEndDateChanged = bMesoSplitChanged = false;
				}
				appDB.saveMesocycle(bNewMeso, txtMesoName.text, mesoStartDate, mesoEndDate, txtMesoNotes.text,
									txtMesoNWeeks.text, txtMesoSplit.text, txtMesoDrugs.text,
									txtSplitA.text, txtSplitB.text, txtSplitC.text, txtSplitD.text, txtSplitE.text, txtSplitF.text,
									changeCalendar, chkPreserveOldCalendar.checked, optPreserveOldCalendarUntilYesterday.checked);
				if (bNewMeso)
					bNewMeso = false;
				JSF.checkWhetherCanCreatePlan();
				bModified = false;
			} //onClicked
		} //btnSaveMeso
	} //footer

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

	function createMesoStatisticsObject() {
		var component = Qt.createComponent("GraphicsViewer.qml", Qt.Asynchronous);

		function finishCreation() {
			mesoStatisticsPage = component.createObject(mesoPropertiesPage, {
				width:homePage.width, height:homePage.height
			});
			appStackView.push(mesoStatisticsPage, StackView.DontLoad);
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
	}

	function pageDeActivation() {
		if (bNewMeso)
			appDB.removeMesocycle(mesoIdx);
	}

	function pageActivation() {
		appDB.setWorkingMeso(mesoId, mesoIdx);
	}
} //Page
