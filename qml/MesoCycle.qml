import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "jsfunctions.js" as JSF

Page {
	id: mesoPropertiesPage
	required property ListModel mesosModel
	required property int mesoId

	property int idxModel
	property string mesoName
	property string mesoNote
	property date mesoStartDate
	property date mesoEndDate
	property int nWeeks
	property string mesoSplit
	property string mesoDrugs
	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date fixedMesoEndDate //Used on newMeso to revert data to the original value gathered from HomePage
	property int week1: 0
	property int week2: 1
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

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
	property bool bCanSave: true
	property bool bMesoSplitChanged: false
	property bool bDate1Changed: false
	property bool bDate2Changed: false

	property var mesoCalendarObject: null

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

	header: ToolBar {
		ToolButton {
			id: btnManageMeso
			text: qsTr("Mesocycle Calendar")
			font.bold: true
			font.capitalization: Font.MixedCase
			display: AbstractButton.TextBesideIcon
			anchors.centerIn: parent
			enabled: !bNewMeso && !bModified
			icon.source: "qrc:/images/"+lightIconFolder+"edit-mesocycle.png"

			onClicked: {
				if (mesoCalendarObject === null)
					createMesoCalendarObject(true);
				else
					mesoPropertiesPage.StackView.view.push(mesoCalendarObject);
			}
		}
	}

	ScrollView {
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight

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
				text: qsTr("Mesocycle's name")
				font.bold: true
				Layout.alignment: Qt.AlignHCenter
				Layout.topMargin: 10
				color: "white"
			}
			TPTextInput {
				id: txtMesoName
				text: mesoName
				Layout.alignment: Qt.AlignHCenter
				Layout.minimumWidth: parent.width / 2
				Layout.maximumWidth: parent.width - 20
				wrapMode: Text.WordWrap
				ToolTip.text: qsTr("Mesocycle name too short")

				onTextEdited: {
					if (text.length >= 5) {
						if (!bNewMeso) {
							if (text !== mesosModel.get(idxModel).mesoName)
								mesoName = text;
						}
						else
							mesoName = text;
						bCanSave = true;
						bModified = true;
					}
					else {
						bCanSave = false;
					}
					ToolTip.visible = !bCanSave;
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
				text: qsTr("Start date for meso")
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
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
					windowTitle: qsTr("Please select the initial date for the mesocycle ") + mesoName
					onDateSelected: function(date, nweek) {
						if (bNewMeso || (date !== mesosModel.get(idxModel).mesoStartDate)) {
							mesoStartDate = date;
							week1 = nweek;
							nWeeks = JSF.calculateNumberOfWeeks(week1, week2);
							bDate1Changed = true;
						}
						else
							bDate1Changed = false;

						if (bNewMeso)
							caldlg2.open();
					}
				}

				RoundButton {
					id: btnStartDate
					anchors.left: txtMesoStartDate.right
					anchors.verticalCenter: txtMesoStartDate.verticalCenter
					anchors.leftMargin: 10
					icon.source: "qrc:/images/white/calendar.png"

					onClicked: caldlg.open();
				}
			}

			Label {
				text: qsTr("End date for meso")
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: "white"
			}
			TPTextInput {
				id: txtMesoEndDate
				text: JSF.formatDateToDisplay(mesoEndDate, AppSettings.appLocale)
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				readOnly: true

				Keys.onReturnPressed: {
					if (btnSaveMeso.enabled)
						btnSaveMeso.clicked();
				}

				CalendarDialog {
					id: caldlg2
					showDate: mesoEndDate
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					windowTitle: qsTr("Please select the end date for the mesocycle ") + mesoName
					onDateSelected: function(date, nweek) {
						if (bNewMeso || (date !== mesosModel.get(idxModel).mesoEndDate)) {
							mesoEndDate = date;
							week2 = nweek;
							nWeeks = JSF.calculateNumberOfWeeks(week1, week2);
							bDate2Changed = true;
						}
						else
							bDate2Changed = false;

						txtMesoSplit.forceActiveFocus();
					}
				}

				RoundButton {
					id: btnEndDate
					anchors.left: txtMesoEndDate.right
					anchors.verticalCenter: txtMesoEndDate.verticalCenter
					anchors.leftMargin: 10
					icon.source: "qrc:/images/"+darkIconFolder+"calendar.png"

					onClicked: caldlg2.open();
				}
			}

			Label {
				id: lblnWeeks
				text: qsTr("Number of weeks: ")
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: 50
				Layout.maximumWidth: 50
				color: "white"
			}

			TPTextInput {
				id: txtMesonWeeks
				text: nWeeks.toString()
				width: txtMesoEndDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				readOnly: true

				onTextChanged: {
					if (bLoadCompleted)
						bModified = true;
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
				regularExpression: new RegExp(/[A-FR]+/);
			}
			TPTextInput {
				id: txtMesoSplit
				width: txtMesoStartDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				validator: regEx
				text: mesoSplit
				ToolTip.text: qsTr("On a mesocycle, there must be at least one rest day(R)")

				RoundButton {
					id: btnTrainingSplit
					anchors {
						left: txtMesoSplit.right
						verticalCenter: txtMesoSplit.verticalCenter
						leftMargin: 10
					}
					icon.source: paneTrainingSplit.visible ? "qrc:/images/"+lightIconFolder+"fold-up.png" :
													"qrc:/images/"+lightIconFolder+"fold-down.png"
					onClicked: paneTrainingSplit.shown = !paneTrainingSplit.shown
				}

				onTextEdited: {
					if (bNewMeso || (text !== mesosModel.get(idxModel).mesoSplit)) {
						if (text.indexOf('R') === -1) {
							bCanSave = false;
						}
						else {
							bCanSave = true;
							mesoSplit = text;
							bMesoSplitChanged = true;
						}
					}
					else
						bMesoSplitChanged = false;
					bModified = true;
					ToolTip.visible = !bCanSave;
				}

				Keys.onReturnPressed: { //Alphanumeric keyboard
					if (!paneTrainingSplit.shown)
						btnTrainingSplit.clicked();
					txtSplitA.forceActiveFocus();
				}
				Keys.onEnterPressed: { //Numeric keyboard
					if (!paneTrainingSplit.shown)
						btnTrainingSplit.clicked();
					txtSplitA.forceActiveFocus();
				}
			}

			Frame {
				id: frmMesoAdjust
				Layout.fillWidth: true
				Layout.rightMargin: 20
				Layout.leftMargin: 5
				visible: !bNewMeso & (bDate1Changed | bDate2Changed | bMesoSplitChanged)
				padding: 0
				spacing: 0

				ColumnLayout {
					id: layoutSplit
					anchors.fill: parent
					spacing: 0

					CheckBox {
						id: chkPreserveOldCalendar
						text: qsTr("Preserve previous calendar information?")
						checked: false
						Layout.fillWidth: true
						Layout.leftMargin: 5

						indicator: Rectangle {
							implicitWidth: 26
							implicitHeight: 26
							x: chkPreserveOldCalendar.leftPadding
							y: parent.height / 2 - height / 2
							radius: 5
							border.color: chkPreserveOldCalendar.down ? "#17a81a" : "#21be2b"

							Rectangle {
								width: 14
								height: 14
								x: 6
								y: 6
								radius: 2
								color: chkPreserveOldCalendar.down ? "#17a81a" : "#21be2b"
								visible: chkPreserveOldCalendar.checked
							}
						}
						contentItem: Text {
							text: chkPreserveOldCalendar.text
							wrapMode: Text.WordWrap
							opacity: enabled ? 1.0 : 0.3
							verticalAlignment: Text.AlignVCenter
							leftPadding: chkPreserveOldCalendar.indicator.width + chkPreserveOldCalendar.spacing
							color: "white"
						}

						onClicked: {
							if (checked) {
								if (!optPreserveOldCalendar.checked && !optPreserveOldCalendarUntilYesterday.checked)
									optPreserveOldCalendar.checked = true;
							}
						}
					} //CheckBox

					RadioButton {
						id: optPreserveOldCalendar
						text: qsTr("All of the old information")
						Layout.fillWidth: true
						enabled: chkPreserveOldCalendar.checked
						checked: false
					}

					RadioButton {
						id: optPreserveOldCalendarUntilYesterday
						text: qsTr("Only until yesterday")
						Layout.fillWidth: true
						checked: false
						enabled: chkPreserveOldCalendar.checked && isDateWithinMeso(today)
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
				clip: true
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
							else
								txtMesoDrugs.forceActiveFocus();
						}
						Keys.onEnterPressed: { //Numeric keyboard
							if (mesoSplit.indexOf('C') !== -1)
								txtSplitC.forceActiveFocus();
							else
								txtMesoDrugs.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Day C: ")
						Layout.row: 2
						Layout.column: 0
						color: "white"
					}
					TPTextInput {
						id: txtSplitC
						text: strSplitC
						font.bold: true
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
							else
								txtMesoDrugs.forceActiveFocus();
						}
						Keys.onEnterPressed: { //Numeric keyboard
							if (mesoSplit.indexOf('D') !== -1)
								txtSplitD.forceActiveFocus();
							else
								txtMesoDrugs.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Day D: ")
						Layout.row: 3
						Layout.column: 0
						color: "white"
					}
					TPTextInput {
						id: txtSplitD
						text: strSplitD
						font.bold: true
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
							else
								txtMesoDrugs.forceActiveFocus();
						}
						Keys.onEnterPressed: { //Numeric keyboard
							if (mesoSplit.indexOf('E') !== -1)
								txtSplitE.forceActiveFocus();
							else
								txtMesoDrugs.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Day E: ")
						Layout.row: 4
						Layout.column: 0
						color: "white"
					}
					TPTextInput {
						id: txtSplitE
						text: strSplitE
						font.bold: true
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
							else
								txtMesoDrugs.forceActiveFocus();
						}
						Keys.onEnterPressed: { //Numeric keyboard
							if (mesoSplit.indexOf('F') !== -1)
								txtSplitF.forceActiveFocus();
							else
								txtMesoDrugs.forceActiveFocus();
						}
					}

					Label {
						text: qsTr("Day F: ")
						Layout.row: 5
						Layout.column: 0
						color: "white"
					}
					TPTextInput {
						id: txtSplitF
						text: strSplitF
						font.bold: true
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

						Keys.onReturnPressed: { //Alphanumeric keyboard
							txtMesoDrugs.forceActiveFocus();
						}
						Keys.onEnterPressed: { //Numeric keyboard
							txtMesoDrugs.forceActiveFocus();
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
							mesoPropertiesPage.StackView.view.push("ExercisesPlanner.qml", { "mesoId":mesoId, "mesoSplit":mesoSplit });
						}
					}
				} //GridLayout
			} //Pane

			Label {
				text: qsTr("Drug Protocol: ")
				font.bold: true
				Layout.leftMargin: 5
				color: "white"
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
					text: mesoDrugs
					color: "white"

					onEditingFinished: {
						if (bNewMeso || (text !== mesosModel.get(idxModel).mesoDrugs)) {
							mesoDrugs = text;
							bModified = true;
						}
					}
				}
				ScrollBar.vertical: ScrollBar { id: vBar}
				ScrollBar.horizontal: ScrollBar { id: hBar }
			}

			Label {
				text: qsTr("Notes: ")
				font.bold: true
				Layout.leftMargin: 5
				color: "white"
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
					text: mesoNote
					color: "white"

					onEditingFinished: {
						if (bNewMeso || (text !== mesosModel.get(idxModel).mesoNote)) {
							mesoNote = text;
							bModified = true;
						}
					}
				}
				ScrollBar.vertical: ScrollBar {}
				ScrollBar.horizontal: ScrollBar {}
			}
		} //ColumnLayout
	} //ScrollView

	footer: ToolBar {
		id: mesoCycleToolBar
		width: parent.width

		ToolButton {
			id: btnRevert
			text: qsTr("Cancel alterations")
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			font.capitalization: Font.MixedCase
			display: AbstractButton.TextUnderIcon
			icon.source: "qrc:/images/"+lightIconFolder+"revert-day.png"
			icon.height: 20
			icon.width: 20
			enabled: bModified

			onClicked: {
				if (!bNewMeso) {
					mesoPropertiesPage.mesoName = mesosModel.get(idxModel).mesoName;
					mesoPropertiesPage.mesoStartDate = mesosModel.get(idxModel).mesoStartDate;
					mesoPropertiesPage.mesoEndDate = mesosModel.get(idxModel).mesoEndDate;
					mesoPropertiesPage.nWeeks = mesosModel.get(idxModel).nWeeks;
					mesoPropertiesPage.mesoSplit = mesosModel.get(idxModel).mesoSplit;
					mesoPropertiesPage.mesoDrugs = mesosModel.get(idxModel).mesoDrugs;
					mesoPropertiesPage.mesoNote = mesosModel.get(idxModel).mesoNote;
					strSplitA = divisionModel.get(idxDivision).splitA;
					strSplitB = divisionModel.get(idxDivision).splitB;
					strSplitC = divisionModel.get(idxDivision).splitC;
					strSplitD = divisionModel.get(idxDivision).splitD;
					strSplitE = divisionModel.get(idxDivision).splitE;
					strSplitF = divisionModel.get(idxDivision).splitF;
				}
				else {
					mesoPropertiesPage.mesoName = "Novo mesociclo";
					mesoPropertiesPage.mesoStartDate = calendarStartDate;
					mesoPropertiesPage.mesoEndDate = fixedMesoEndDate;
					mesoPropertiesPage.nWeeks = JSF.calculateNumberOfWeeks(calendarStartDate.getDay(), fixedMesoEndDate.getDay());
					mesoPropertiesPage.mesoSplit = "ABCRDER";
					mesoPropertiesPage.mesoDrugs = " ";
					mesoPropertiesPage.mesoNote = " ";
					strSplitA = " ";
					strSplitB = " ";
					strSplitC = " ";
					strSplitD = " ";
					strSplitE = " ";
					strSplitF = " ";
				}

				bModified = false;
				bDate1Changed = false;
				bDate2Changed = false;
				bMesoSplitChanged = false;
			}
		} //btnRevert

		ToolButton {
			id: btnSaveMeso
			text: qsTr("Save Information")
			font.capitalization: Font.MixedCase
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			display: AbstractButton.TextUnderIcon
			icon.source: "qrc:/images/"+lightIconFolder+"save-day.png"
			icon.height: 20
			icon.width: 20
			enabled: bModified & bCanSave

			onClicked: {
				if (mesoName.length === 0)
					mesoName = "Novo Mesociclo";
				if (mesoNote.length === 0)
					mesoNote = " ";
				if (mesoDrugs.length === 0)
					mesoDrugs = " ";
				if (mesoSplit.length === 0)
					mesoSplit = " ";
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
					let results = Database.newMeso(mesoName, mesoStartDate.getTime(), mesoEndDate.getTime(), mesoNote,
									   nWeeks, mesoSplit, mesoDrugs);
					mesoId = parseInt(results.insertId);

					mesosModel.append ({
						mesoId: mesoId,
						mesoName: mesoName,
						mesoStartDate: mesoStartDate,
						mesoEndDate: mesoEndDate,
						mesoNote: mesoNote,
						nWeeks: nWeeks,
						mesoSplit: mesoSplit,
						mesoDrugs: mesoDrugs,
						minimumMesoStartDate: minimumMesoStartDate,
						maximumMesoEndDate: maximumMesoEndDate,
						week1: week1,
						week2: week2
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
					createMesoCalendarObject(true);
				}
				else {
					Database.updateMesoName(mesoId, mesoName);
					Database.updateMesoStartDate(mesoId, mesoStartDate.getTime());
					Database.updateMesoEndDate(mesoId, mesoEndDate.getTime());
					Database.updateMesonWeeks(mesoId, nWeeks);
					Database.updateMesoSplit(mesoId, mesoSplit);
					Database.updateMesoDivision(mesoId,'A', strSplitA);
					Database.updateMesoDivision(mesoId,'B', strSplitB);
					Database.updateMesoDivision(mesoId,'C', strSplitC);
					Database.updateMesoDivision(mesoId,'D', strSplitD);
					Database.updateMesoDivision(mesoId,'E', strSplitE);
					Database.updateMesoDivision(mesoId,'F', strSplitF);
					Database.updateMesoDrugs(mesoId, mesoDrugs);
					Database.updateMesoNote(mesoId, mesoNote);

					if (bDate1Changed || bDate2Changed || bMesoSplitChanged) {
						if (Database.checkCalendarForMesoExists(mesoId)) {
							if (mesoCalendarObject === null)
								createMesoCalendarObject(false);
							else {
								mesoCalendarObject.mesoStartDate = mesosModel.get(idxModel).mesoStartDate;
								mesoCalendarObject.mesoEndDate = mesosModel.get(idxModel).mesoEndDate;
								mesoCalendarObject.mesoSplit = mesosModel.get(idxModel).mesoSplit;
								mesoCalendarObject.mesoName = mesosModel.get(idxModel).mesoName;
							}
							mesoPropertiesPage.StackView.view.push(mesoCalendarObject);
							mesoCalendarObject.refactoryDatabase(mesoStartDate, mesoEndDate, mesoSplit, chkPreserveOldCalendar.checked, optPreserveOldCalendarUntilYesterday.checked);
							bDate1Changed = bDate2Changed = bMesoSplitChanged = false;
						}
					}

					mesosModel.setProperty(idxModel, "mesoName", mesoName);
					mesosModel.setProperty(idxModel, "mesoStartDate", mesoStartDate);
					mesosModel.setProperty(idxModel, "mesoEndDate", mesoEndDate);
					mesosModel.setProperty(idxModel, "mesonWeeks", nWeeks)
					mesosModel.setProperty(idxModel, "mesoSplit", mesoSplit);
					divisionModel.setProperty(idxDivision, "splitA", strSplitA);
					divisionModel.setProperty(idxDivision, "splitB", strSplitB);
					divisionModel.setProperty(idxDivision, "splitC", strSplitC);
					divisionModel.setProperty(idxDivision, "splitD", strSplitD);
					divisionModel.setProperty(idxDivision, "splitE", strSplitE);
					divisionModel.setProperty(idxDivision, "splitF", strSplitF);
					mesosModel.setProperty(idxModel, "mesoDrugs", mesoDrugs);
					mesosModel.setProperty(idxModel, "mesoNote", mesoNote);
				}
				bDate1Changed = bDate2Changed = bMesoSplitChanged = false;
				appDBModified = true;
				bModified = false;
			} //onClicked
		} //btnSaveMeso

		Component.onCompleted: {
			bLoadCompleted = true;
			checkWhetherCanCreatePlan();
			if (bNewMeso)
				txtMesoName.forceActiveFocus();
		}
	} //footer

	function isDateWithinMeso(date) {
		return (date >= mesoStartDate) && (date <= mesoEndDate);
	}

	function createMesoCalendarObject(bshowpage) {
		var component = Qt.createComponent("MesoContent.qml");
		if (component.status === Component.Ready) {
			mesoCalendarObject = component.createObject(null, {
					mesoId: mesoId, mesoName: mesosModel.get(idxModel).mesoName, mesoStartDate: mesosModel.get(idxModel).mesoStartDate,
					mesoEndDate: mesosModel.get(idxModel).mesoEndDate, mesoSplit: mesosModel.get(idxModel).mesoSplit, bVisualLoad: bshowpage
			});
			if (bshowpage)
				mesoPropertiesPage.StackView.view.push(mesoCalendarObject);
		}
	}

	function checkWhetherCanCreatePlan()
	{
		var ok = true;
		if (mesoSplit.indexOf('A') !== -1) {
			ok &= (txtSplitA.length > 1);
			txtSplitA.cursorPosition = 0;
		}
		if (mesoSplit.indexOf('B') !== -1) {
			ok &= (txtSplitB.length > 1);
			txtSplitB.cursorPosition = 0;
		}
		if (mesoSplit.indexOf('C') !== -1) {
			ok &= (txtSplitC.length > 1);
			txtSplitC.cursorPosition = 0;
		}
		if (mesoSplit.indexOf('D') !== -1) {
			ok &= (txtSplitD.length > 1);
			txtSplitD.cursorPosition = 0;
		}
		if (mesoSplit.indexOf('E') !== -1) {
			ok &= (txtSplitE.length > 1);
			txtSplitE.cursorPosition = 0;
		}
		if (mesoSplit.indexOf('F') !== -1) {
			ok &= (txtSplitF.length > 1);
			txtSplitF.cursorPosition = 0;
		}
		btnCreateExercisePlan.enabled = ok;
	}
} //Page

