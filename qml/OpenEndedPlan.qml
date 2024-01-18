import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF

Page {
	id: openEndedPage
	required property ListModel mesosModel
	required property int mesoId

	property int idxModel
	property string mesoName
	property date mesoStartDate
	property string mesoSplit
	property date minimumMesoStartDate
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage
	property date maximumMesoEndDate

	property int idxDivision: 0
	property string strSplitA: " "
	property string strSplitB: " "
	property string strSplitC: " "
	property string strSplitD: " "
	property string strSplitE: " "
	property string strSplitF: " "

	property bool bLoadCompleted: false
	property bool bNewMeso: mesoId === -1
	property bool bModified: false

	background: Rectangle {
		color: primaryDarkColor
		opacity: 0.7
		Image {
			anchors.fill: parent
			source: "qrc:/images/app_logo.png"
			fillMode: Image.PreserveAspectFit
			opacity: 0.6
		}
	}

	onBModifiedChanged: {
		if (bLoadCompleted)
			bNavButtonsEnabled = !bModified;
	}

	ListModel {
		id: divisionModel

		Component.onCompleted: {
			if (mesoId !== -1) {
				let divisions = Database.getDivisionForMeso(mesoId);
				for (let division of divisions)
					append(division);

				if (rowCount() > 0) {
					strSplitA = get(0).splitA;
					strSplitB = get(0).splitB;
					strSplitC = get(0).splitC;
					strSplitD = get(0).splitD;
					strSplitE = get(0).splitE;
					strSplitF = get(0).splitF;
				}
			}
		}
	}

	ColumnLayout {
		id: colMain
		anchors.fill: parent
		spacing: 5

		Label {
			text: qsTr("Start date for the period")
			font.bold: true
			Layout.alignment: Qt.AlignLeft
			Layout.leftMargin: 5
			Layout.topMargin: 10
			color: "white"
		}

		TPTextInput {
			id: txtMesoStartDate
			text: JSF.formatDateToDisplay(mesoStartDate, AppSettings.appLocale)
			Layout.fillWidth: false
			Layout.leftMargin: 5
			Layout.minimumWidth: parent.width / 2
			readOnly: true

			CalendarDialog {
				id: caldlg
				showDate: calendarStartDate
				initDate: minimumMesoStartDate
				finalDate: maximumMesoEndDate
				windowTitle: qsTr("Please select the initial date for the program ") + mesoName
				onDateSelected: function(date, nweek) {
					if (bNewMeso || (date !== mesosModel.get(idxModel).mesoStartDate))
						mesoStartDate = date;
				}
			}

			RoundButton {
				id: btnStartDate
				anchors.left: txtMesoStartDate.right
				anchors.verticalCenter: txtMesoStartDate.verticalCenter
				anchors.leftMargin: 10
				icon.source: "qrc:/images/"+lightIconFolder+"calendar.png"

				onClicked: caldlg.open();
			}
		}

		Label {
			text: qsTr("Weekly Training Division: ")
			font.bold: true
			Layout.alignment: Qt.AlignLeft
			Layout.leftMargin: 5
			color: "white"
		}

		RegularExpressionValidator {
			id: regEx
			regularExpression: new RegExp(/[A-F]+/);
		}

		TPTextInput {
			id: txtMesoSplit
			width: txtMesoStartDate.width
			Layout.alignment: Qt.AlignLeft
			Layout.leftMargin: 5
			Layout.minimumWidth: parent.width / 2
			validator: regEx
			text: mesoSplit

			onTextEdited: {
				if (bNewMeso || (text !== mesosModel.get(idxModel).mesoSplit))
					mesoSplit = text;
				bModified = true;
			}

			Keys.onReturnPressed: { //Alphanumeric keyboard
				txtSplitA.forceActiveFocus();
			}
			Keys.onEnterPressed: { //Numeric keyboard
				txtSplitA.forceActiveFocus();
			}
		}

		GridLayout {
			Layout.fillWidth: true
			columns: 2
			rows: 7

			Label {
				text: qsTr("Day A: ")
				font.bold: true
				Layout.row: 0
				Layout.column: 0
				color: "white"
			}
			TPTextInput {
				id: txtSplitA
				text: strSplitA
				Layout.row: 0
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitA)) {
						strSplitA = text;
						bModified = true;
						if (text.length >=3)
							checkWhetherCanCreatePlan();
					}
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					txtSplitB.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					txtSplitB.forceActiveFocus();
				}
			}
			Label {
				text: qsTr("Day B: ")
				font.bold: true
				Layout.row: 1
				Layout.column: 0
				color: "white"
			}
			TPTextInput {
				id: txtSplitB
				text: strSplitB
				Layout.row: 1
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitB)) {
						strSplitB = text;
						bModified = true;
						if (text.length >=3)
						checkWhetherCanCreatePlan();
					}
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (mesoSplit.indexOf('C') !== -1)
						txtSplitC.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (mesoSplit.indexOf('C') !== -1)
						txtSplitC.forceActiveFocus();
					}
			}

			Label {
				text: qsTr("Day C: ")
				font.bold: true
				Layout.row: 2
				Layout.column: 0
				color: "white"
			}
			TPTextInput {
				id: txtSplitC
				text: strSplitC
				Layout.row: 2
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitC)) {
						strSplitC = text;
						bModified = true;
						if (text.length >=3)
						checkWhetherCanCreatePlan();
					}
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (mesoSplit.indexOf('D') !== -1)
						txtSplitD.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (mesoSplit.indexOf('D') !== -1)
						txtSplitD.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Day D: ")
				font.bold: true
				Layout.row: 3
				Layout.column: 0
				color: "white"
			}
			TPTextInput {
				id: txtSplitD
				text: strSplitD
				Layout.row: 3
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitD)) {
						strSplitD = text;
						bModified = true;
						if (text.length >=3 )
						checkWhetherCanCreatePlan();
					}
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (mesoSplit.indexOf('E') !== -1)
						txtSplitE.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (mesoSplit.indexOf('E') !== -1)
						txtSplitE.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Day E: ")
				font.bold: true
				Layout.row: 4
				Layout.column: 0
				color: "white"
			}
			TPTextInput {
				id: txtSplitE
				text: strSplitE
				Layout.row: 4
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitE)) {
						strSplitE = text;
						bModified = true;
						if (text.length >=3 )
						checkWhetherCanCreatePlan();
					}
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (mesoSplit.indexOf('F') !== -1)
						txtSplitF.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (mesoSplit.indexOf('F') !== -1)
						txtSplitF.forceActiveFocus();
				}
			}

			Label {
				text: qsTr("Day F: ")
				font.bold: true
				Layout.row: 5
				Layout.column: 0
				color: "white"
			}
			TPTextInput {
				id: txtSplitF
				text: strSplitF
				Layout.row: 5
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitF)) {
						strSplitF = text;
						bModified = true;
						if (text.length >=3 )
						checkWhetherCanCreatePlan();
					}
				}
			}

			ButtonFlat {
				id: btnCreateExercisePlan
				text: qsTr("Exercises Planner")
				Layout.row: 6
				Layout.column: 0
				Layout.columnSpan: 2
				Layout.alignment: Qt.AlignCenter

				onClicked: {
					openEndedPage.StackView.view.push("ExercisesPlanner.qml", { "mesoId":mesoId, "mesoSplit":mesoSplit });
				}
			}
		} //GridLayout

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true
		}
	} //ColumnLayout

	footer: ToolBar {
		id: mesoCycleToolBar
		height: 55
		width: parent.width

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnRevert
			text: qsTr("Cancel alterations")
			imageSource: "qrc:/images/"+lightIconFolder+"revert-day.png"
			enabled: bModified
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				if (!bNewMeso) {
					openEndedPage.mesoStartDate = mesosModel.get(idxModel).mesoStartDate;
					openEndedPage.mesoSplit = mesosModel.get(idxModel).mesoSplit;
					strSplitA = divisionModel.get(idxDivision).splitA;
					strSplitB = divisionModel.get(idxDivision).splitB;
					strSplitC = divisionModel.get(idxDivision).splitC;
					strSplitD = divisionModel.get(idxDivision).splitD;
					strSplitE = divisionModel.get(idxDivision).splitE;
					strSplitF = divisionModel.get(idxDivision).splitF;
				}
				else {
					openEndedPage.mesoStartDate = calendarStartDate;
					openEndedPage.mesoSplit = "ABC";
					strSplitA = " ";
					strSplitB = " ";
					strSplitC = " ";
					strSplitD = " ";
					strSplitE = " ";
					strSplitF = " ";
				}
				bModified = false;
			}
		} //btnRevert

		ButtonFlat {
			id: btnSaveMeso
			text: qsTr("Save Information")
			imageSource: "qrc:/images/"+lightIconFolder+"save-day.png"
			enabled: bModified
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				if (mesoSplit.length === 0)
					mesoSplit = "ABC";
				if (strSplitA.length === 0)
					strSplitA = " ";
				if (strSplitB.length === 0)
					strSplitB = " ";
				if (strSplitC.length === 0)
					strSplitC = " ";
				if (strSplitD.length === 0)
					strSplitD = " ";
				if (strSplitE.length === 0)
					strSplitE = " ";
				if (strSplitF.length === 0)
					strSplitF = " ";
				if (bNewMeso) {
					let results = Database.newMeso(qsTr("Open Ended Training Schedule"), mesoStartDate.getTime(), 0, " ", 0, mesoSplit);
					mesoId = parseInt(results.insertId);

					mesosModel.append ({
						mesoId: mesoId,
						mesoName: mesoName,
						mesoStartDate: mesoStartDate,
						mesoEndDate: 0,
						mesoNote: " ",
						nWeeks: 0,
						mesoSplit: mesoSplit,
						mesoDrugs: " ",
						realMeso: false
					});

					idxModel = mesosModel.count - 1;

					let results2 = Database.newMesoDivision(mesoId, strSplitA, strSplitB, strSplitC, strSplitD, strSplitE, strSplitF);
					divisionModel.append({
						"divisionId": parseInt(results2.insertId),
						"mesoId": parseInt(mesoId),
						"splitA": strSplitA,
						"splitB": strSplitB,
						"splitC": strSplitC,
						"splitD": strSplitD,
						"splitE": strSplitE,
						"splitF": strSplitF
					});

					bNewMeso = false;
					dateTimer.triggered(); //Update tabBar and the meso model index it uses
				}
				else {
					Database.updateMesoStartDate(mesoId, mesoStartDate.getTime());
					Database.updateMesoSplit(mesoId, mesoSplit);
					Database.updateMesoDivision(mesoId,'A', strSplitA);
					Database.updateMesoDivision(mesoId,'B', strSplitB);
					Database.updateMesoDivision(mesoId,'C', strSplitC);
					Database.updateMesoDivision(mesoId,'D', strSplitD);
					Database.updateMesoDivision(mesoId,'E', strSplitE);
					Database.updateMesoDivision(mesoId,'F', strSplitF);

					mesosModel.setProperty(idxModel, "mesoStartDate", mesoStartDate);
					mesosModel.setProperty(idxModel, "mesoSplit", mesoSplit);
					divisionModel.setProperty(idxDivision, "splitA", strSplitA);
					divisionModel.setProperty(idxDivision, "splitB", strSplitB);
					divisionModel.setProperty(idxDivision, "splitC", strSplitC);
					divisionModel.setProperty(idxDivision, "splitD", strSplitD);
					divisionModel.setProperty(idxDivision, "splitE", strSplitE);
					divisionModel.setProperty(idxDivision, "splitF", strSplitF);
				}
				appDBModified = true;
				bModified = false;
			} //onClicked
		} //btnSaveMeso

		Component.onCompleted: {
			bLoadCompleted = true;
			checkWhetherCanCreatePlan();
		}
	} //footer

	function checkWhetherCanCreatePlan()
	{
		var ok = true;
		if (mesoSplit.indexOf('A') !== -1) {
			ok &= (txtSplitA.length > 1);
			txtSplitA.cursorPosition = 0;
			txtSplitA.enabled = true;
		}
		else
			txtSplitA.enabled = false;

		if (mesoSplit.indexOf('B') !== -1) {
			ok &= (txtSplitB.length > 1);
			txtSplitB.cursorPosition = 0;
			txtSplitB.enabled = true;
		}
		else
			txtSplitB.enabled = false;

		if (mesoSplit.indexOf('C') !== -1) {
			ok &= (txtSplitC.length > 1);
			txtSplitC.cursorPosition = 0;
			txtSplitC.enabled = true;
		}
		else
			txtSplitC.enabled = false;

		if (mesoSplit.indexOf('D') !== -1) {
			ok &= (txtSplitD.length > 1);
			txtSplitD.cursorPosition = 0;
			txtSplitD.enabled = true;
		}
		else
			txtSplitD.enabled = false;

		if (mesoSplit.indexOf('E') !== -1) {
			ok &= (txtSplitE.length > 1);
			txtSplitE.cursorPosition = 0;
			txtSplitE.enabled = true;
		}
		else
			txtSplitE.enabled = false;

		if (mesoSplit.indexOf('F') !== -1) {
			ok &= (txtSplitF.length > 1);
			txtSplitF.cursorPosition = 0;
			txtSplitF.enabled = true;
		}
		else
			txtSplitF.enabled = false;

		btnCreateExercisePlan.enabled = ok;
	}
} //Page
