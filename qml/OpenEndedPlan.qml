import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF

Page {
	id: openEndedPage
	objectName: "openEndedPage"
	width: windowWidth
	height: windowHeight

	required property int mesoId
	required property int idxModel
	required property date mesoStartDate
	required property date mesoEndDate

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
	property bool bEmptyPlan: false

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
			color: AppSettings.fontColor
		}

		TPTextInput {
			id: txtMesoStartDate
			text: runCmd.formatDate(mesocyclesModel.getDate(idxModel, 2))
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
				icon.source: "qrc:/images/"+AppSettings.iconFolder+"calendar.png"

				onClicked: caldlg.open();
			}
		}

		Label {
			text: qsTr("Weekly Training Division: ")
			font.bold: true
			Layout.alignment: Qt.AlignLeft
			Layout.leftMargin: 5
			color: AppSettings.fontColor
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
				color: AppSettings.fontColor
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
				color: AppSettings.fontColor
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
				color: AppSettings.fontColor
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
				color: AppSettings.fontColor
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
				color: AppSettings.fontColor
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
				color: AppSettings.fontColor
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

			TPButton {
				id: btnCreateExercisePlan
				text: qsTr("Exercises Planner")
				Layout.row: 6
				Layout.column: 0
				Layout.columnSpan: 2
				Layout.alignment: Qt.AlignCenter
				highlight: bEmptyPlan

				onClicked: {
					for (var i = 0; i < mesoPlannerList.length; ++i) {
						if (mesoPlannerList[i].mesoId === mesoId) {
							appStackView.push(mesoPlannerList[i].Object, StackView.DontLoad);
							break;
						}
					}
					var component = Qt.createComponent("ExercisesPlanner.qml", Qt.Asynchronous);

					function finishCreation() {
						var mesoPlannerObject = component.createObject(openEndedPage, {
								"mesoId":mesoId, "mesoSplit":mesoSplit, "width":openEndedPage.width, "height":openEndedPage.height
						});
						mesoPlannerList.push({ "mesoId": mesoId, "Object":mesoPlannerObject });
						appStackView.push(mesoPlannerObject, StackView.DontLoad);
					}

					if (component.status === Component.Ready)
						finishCreation();
					else
						component.statusChanged.connect(finishCreation);
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
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}

		TPButton {
			id: btnRevert
			text: qsTr("Cancel alterations")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"revert-day.png"
			textUnderIcon: true
			enabled: bModified
			anchors.left: parent.left
			anchors.leftMargin: 5
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

		TPButton {
			id: btnSaveMeso
			text: qsTr("Save Information")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"save-day.png"
			textUnderIcon: true
			enabled: bModified
			anchors.right: parent.right
			anchors.rightMargin: 5
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
					mesoName = qsTr("Open Ended Training Schedule");
					let results = Database.newMeso(mesoName, mesoStartDate.getTime(), 0, "##", 0, mesoSplit, "##");
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
				}
				else {
					Database.updateMeso(mesoId, mesoName, mesoStartDate.getTime(), 0, "##", 0, mesoSplit, "##");
					Database.updateMesoDivision(mesoId, strSplitA, strSplitB, strSplitC, strSplitD, strSplitE, strSplitF);

					mesosModel.setProperty(idxModel, "mesoStartDate", mesoStartDate);
					mesosModel.setProperty(idxModel, "mesoSplit", mesoSplit);
					divisionModel.setProperty(idxDivision, "splitA", strSplitA);
					divisionModel.setProperty(idxDivision, "splitB", strSplitB);
					divisionModel.setProperty(idxDivision, "splitC", strSplitC);
					divisionModel.setProperty(idxDivision, "splitD", strSplitD);
					divisionModel.setProperty(idxDivision, "splitE", strSplitE);
					divisionModel.setProperty(idxDivision, "splitF", strSplitF);
				}
				bModified = false;
			} //onClicked
		} //btnSaveMeso

		Component.onCompleted: {
			bLoadCompleted = true;
			JSF.checkWhetherCanCreatePlan();
		}
	} //footer
} //Page
