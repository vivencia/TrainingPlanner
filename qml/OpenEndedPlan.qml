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
	property bool bFirstTime: false
	property var firstTimeTip: null

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
		if (bLoadCompleted) {
			if (bFirstTime && firstTimeTip) {
				firstTimeTip.message = qsTr("Click here");
				firstTimeTip.yPos = mesoCycleToolBar.y;
				firstTimeTip.xPos = openEndedPage.width;
				firstTimeTip.visible = true;
			}
		}
	}

	ToolTip {
		id: firstTimeToolTip
		text: qsTr("Change the Weekly Training Division field or specify which muscle groups are trainned on the corresponding division day")
		timeout: -1
		x: 0
		y: mesoCycleToolBar.y - height
		width: mainwindow.width * 0.9
		height: implicitContentHeight + 2*contentItem.padding
		parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
		visible: bFirstTime

		contentItem: Text {
			text: firstTimeToolTip.text
			wrapMode: Text.WordWrap
			color: "white"
			font.pixelSize: AppSettings.fontSizeText
			width: parent.width
			padding: 5
		} //contentItem

		background: Rectangle {
			color: "black"
			opacity: 0.6
			radius: 8
		}
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
		spacing: 10

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
				JSF.moveFocusToNextField('0');
			}
			Keys.onEnterPressed: { //Numeric keyboard
				JSF.moveFocusToNextField('0');
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
				visible: txtSplitA.visible
			}
			TPTextInput {
				id: txtSplitA
				text: strSplitA
				Layout.row: 0
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20
				visible: mesoSplit.indexOf('A') !== -1

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitA)) {
						strSplitA = text;
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
				color: "white"
				visible: txtSplitB.visible
			}
			TPTextInput {
				id: txtSplitB
				text: strSplitB
				Layout.row: 1
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20
				visible: mesoSplit.indexOf('B') !== -1

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitB)) {
						strSplitB = text;
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
				color: "white"
				visible: txtSplitC.visible
			}
			TPTextInput {
				id: txtSplitC
				text: strSplitC
				Layout.row: 2
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20
				visible: mesoSplit.indexOf('C') !== -1

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitC)) {
						strSplitC = text;
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
				color: "white"
				visible: txtSplitD.visible
			}
			TPTextInput {
				id: txtSplitD
				text: strSplitD
				Layout.row: 3
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20
				visible: mesoSplit.indexOf('D') !== -1

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitD)) {
						strSplitD = text;
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
				color: "white"
				visible: txtSplitE.visible
			}
			TPTextInput {
				id: txtSplitE
				text: strSplitE
				Layout.row: 4
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20
				visible: mesoSplit.indexOf('E') !== -1

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitE)) {
						strSplitE = text;
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
				color: "white"
				visible: txtSplitF.visible
			}
			TPTextInput {
				id: txtSplitF
				text: strSplitF
				Layout.row: 5
				Layout.column: 1
				Layout.fillWidth: true
				Layout.rightMargin: 20
				visible: mesoSplit.indexOf('F') !== -1

				onEditingFinished: {
					if (bNewMeso || (text !== divisionModel.get(idxDivision).splitF)) {
						strSplitF = text;
						bModified = true;
						if (text.length >=3 )
							JSF.checkWhetherCanCreatePlan();
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
			textUnderIcon: true
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
			textUnderIcon: true
			enabled: bModified
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter

			onClicked: {
				if (bFirstTime) {
					firstTimeToolTip.text = qsTr("Click on HOME now!")
					firstTimeTip.visible = false;
				}

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
					mesoName = qsTr("Open Ended Training Schedule");
					let results = Database.newMeso(mesoName, mesoStartDate.getTime(), 111, "##", 0, mesoSplit, "##");
					mesoId = parseInt(results.insertId);

					mesosModel.append ({
						mesoId: mesoId,
						mesoName: mesoName,
						mesoStartDate: mesoStartDate,
						mesoEndDate: new Date(),
						mesoNote: "##",
						nWeeks: 0,
						mesoSplit: mesoSplit,
						mesoDrugs: "##",
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
			JSF.checkWhetherCanCreatePlan();
		}
	} //footer
} //Page
