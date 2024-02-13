import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vivenciasoftware.qmlcomponents

import "jsfunctions.js" as JSF

Page {
	id: mesoPropertiesPage
	required property int mesoId
	required property int idxModel
	required property date mesoStartDate
	required property date mesoEndDate

	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date fixedMesoEndDate //Used on newMeso to revert data to the original value gathered from HomePage
	property int week1: 0
	property int week2: 1
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

	property string mesoSplit: mesocyclesListModel.get(idxModel, 6)
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

	property var mesocycleCalendarPage: null
	property var mesoStatisticsPage: null

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: primaryDarkColor
		opacity: 0.7
	}

	onBModifiedChanged: {
		if (bLoadCompleted)
			bNavButtonsEnabled = !bModified;
	}

	header: ToolBar {
		height: btnManageMeso.height + 20
		enabled: !bNewMeso && !bModified
		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnManageMeso
			text: qsTr("Calendar")
			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 20
			}
			imageSource: "qrc:/images/"+lightIconFolder+"edit-mesocycle.png"

			onClicked: {
				if (mesocycleCalendarPage === null)
					createMesoCalendarObject(true);
				else
					appStackView.push(mesocycleCalendarPage, StackView.DontLoad);
			}
		}

		ButtonFlat {
			id: btnStatistics
			text: qsTr("Statistics")
			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: 20
			}
			imageSource: "qrc:/images/"+lightIconFolder+"statistics.png"

			onClicked: {
				if (mesoStatisticsPage === null)
					createMesoStatisticsObject();
				else
					appStackView.push(mesoStatisticsPage, StackView.DontLoad);
			}
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
				text: qsTr("Mesocycle's name")
				font.bold: true
				Layout.alignment: Qt.AlignHCenter
				Layout.topMargin: 10
				color: "white"
			}
			TPTextInput {
				id: txtMesoName
				text: mesocyclesListModel.get(idxModel, 1);
				Layout.alignment: Qt.AlignHCenter
				Layout.minimumWidth: parent.width / 2
				Layout.maximumWidth: parent.width - 20
				wrapMode: Text.WordWrap
				ToolTip.text: qsTr("Mesocycle name too short")

				onTextEdited: {
					if (text.length >= 5) {
						if (text !== mesocyclesListModel.get(idxModel, 1)) {
							bCanSave = true;
							bModified = true;
						}
					}
					else
						bCanSave = false;
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
				text: runCmd.formatDate(mesoStartDate)
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				readOnly: true

				CalendarDialog {
					id: caldlg
					showDate: calendarStartDate
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					windowTitle: qsTr("Please select the initial date for the mesocycle ") + txtMesoName.text
					onDateSelected: function(date, nweek) {
						if ( date !== mesocyclesListModel.getDate(idxModel, 2) ) {
							mesoStartDate = date;
							week1 = nweek;
							nWeeks = runCmd.calculateNumberOfWeeks(week1, week2);
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
				text: runCmd.formatDate(mesoEndDate)
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
					windowTitle: qsTr("Please select the end date for the mesocycle ") + txtMesoName.text
					onDateSelected: function(date, nweek) {
						if ( date !== mesocyclesListModel.getDate(idxModel, 3)) {
							mesoEndDate = date;
							week2 = nweek;
							nWeeks = runCmd.calculateNumberOfWeeks(week1, week2);
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
				text: mesocyclesListModel.get(idxModel, 5)
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
				text: mesocyclesListModel.get(idxModel, 6)
				validator: regEx
				width: txtMesoStartDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
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
					if ( text !== mesocyclesListModel.get(idxModel, 6) ) {
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
				visible: !bNewMeso & (bDate1Changed | bDate2Changed | bMesoSplitChanged)
				padding: 0
				spacing: 0

				background: Rectangle {
					border.color: "white"
					color: "transparent"
					radius: 6
				}

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
							border.color: chkPreserveOldCalendar.down ? primaryDarkColor : primaryLightColor

							Rectangle {
								width: 14
								height: 14
								x: 6
								y: 6
								radius: 2
								color: chkPreserveOldCalendar.down ? primaryDarkColor : primaryLightColor
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
							font.pixelSize: AppSettings.fontSizeText
							font.bold: true
						}

						onClicked: {
							if (checked) {
								if (!optPreserveOldCalendar.checked && !optPreserveOldCalendarUntilYesterday.checked)
									optPreserveOldCalendar.checked = true;
							}
						}
					} //CheckBox

					TPRadioButton {
						id: optPreserveOldCalendar
						text: qsTr("All of the old information")
						Layout.fillWidth: true
						enabled: chkPreserveOldCalendar.checked
						checked: false
					}

					TPRadioButton {
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

						Keys.onReturnPressed: { //Alphanumeric keyboard
							JSF.moveFocusToNextField('F');
						}
						Keys.onEnterPressed: { //Numeric keyboard
							JSF.moveFocusToNextField('F');
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
							for (var i = 0; i < mesoPlannerList.length; ++i) {
								if (mesoPlannerList[i].mesoId === mesoId) {
									appStackView.push(mesoPlannerList[i].Object, StackView.DontLoad);
									break;
								}
							}

							var component = Qt.createComponent("ExercisesPlanner.qml", Qt.Asynchronous);

							function finishCreation() {
								var mesoPlannerObject = component.createObject(mesoPropertiesPage, {
										"mesoId":mesoId, "mesoSplit":mesoSplit, "width":mesoPropertiesPage.width, "height":mesoPropertiesPage.height
								});
								mesoPlannerList.push({ "mesoId": mesoId, "Object":mesoPlannerObject });
								appStackView.push(mesoPlannerObject, StackView.DontLoad);
							}

							if (component.status === Component.Ready)
								finishCreation();
							else
								component.statusChanged.connect(finishCreation);
						} //onClicked
					} //ButtonFlat
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
					text: mesocyclesListModel.get(idxModel, 7)
					color: "white"

					onEditingFinished: {
						if ( text !== mesocyclesListModel.get(idxModel, 7) )
							bModified = true;
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
					text: mesocyclesListModel.get(idxModel, 4)
					color: "white"

					onEditingFinished: {
						if ( text !== mesocyclesListModel.get(idxModel, 4) )
							bModified = true;
					}
				}
				ScrollBar.vertical: ScrollBar {}
				ScrollBar.horizontal: ScrollBar {}
			}
		} //ColumnLayout
	} //ScrollView

	Component.onCompleted: {
		bLoadCompleted = true;
		JSF.checkWhetherCanCreatePlan();
		if (bNewMeso)
			txtMesoName.forceActiveFocus();
	}

	Component.onDestruction: {
		if (mesocycleCalendarPage !== null)
			mesocycleCalendarPage.destroy();
		if (mesoStatisticsPage !== null)
			mesoStatisticsPage.destroy();
	}

	footer: ToolBar {
		id: mesoCycleToolBar
		width: parent.width
		height: 55

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnRevert
			text: qsTr("Cancel alterations")
			anchors.left: parent.left
			anchors.leftMargin: 5
			anchors.verticalCenter: parent.verticalCenter
			textUnderIcon: true
			imageSource: "qrc:/images/"+lightIconFolder+"revert-day.png"
			enabled: bModified

			onClicked: {
				if (!bNewMeso) {
					txtMesoName.text = Qt.binding(function() { return mesocyclesListModel.get(idxModel, 1) });
					mesoStartDate = mesocyclesListModel.getDate(idxModel, 2);
					mesoEndDate = mesocyclesListModel.getDate(idxModel, 3);
					txtMesoNotes = Qt.binding(function() { return mesocyclesListModel.get(idxModel, 4) });
					txtMesonWeeks = Qt.binding(function() { return mesocyclesListModel.get(idxModel, 5) });
					txtMesoSplit = Qt.binding(function() { return mesocyclesListModel.get(idxModel, 6) });
					txtMesoDrugs = Qt.binding(function() { return mesocyclesListModel.get(idxModel, 7) });
					strSplitA = divisionModel.get(idxDivision).splitA;
					strSplitB = divisionModel.get(idxDivision).splitB;
					strSplitC = divisionModel.get(idxDivision).splitC;
					strSplitD = divisionModel.get(idxDivision).splitD;
					strSplitE = divisionModel.get(idxDivision).splitE;
					strSplitF = divisionModel.get(idxDivision).splitF;
				}
				else {
					txtMesoName.text = "Novo mesociclo";
					mesoStartDate = calendarStartDate;
					mesoEndDate = fixedMesoEndDate;
					txtMesonWeeks.text = runCmd.calculateNumberOfWeeks(calendarStartDate, fixedMesoEndDate);
					txtMesoSplit.text = "ABCRDER";
					txtMesoDrugs.text = "";
					txtMesoNotes.text = "";
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

		ButtonFlat {
			id: btnSaveMeso
			text: qsTr("Save Information")
			anchors.right: parent.right
			anchors.rightMargin: 5
			anchors.verticalCenter: parent.verticalCenter
			textUnderIcon: true
			imageSource: "qrc:/images/"+lightIconFolder+"save-day.png"
			enabled: bModified & bCanSave

			onClicked: {
				if (bNewMeso) {

					function getMesoId() {
						appDB.qmlReady.disconnect(getMesoId);
						mesoId = appDB.insertId();
					}

					appDB.pass_object(mesocyclesListModel);
					appDB.qmlReady.connect(getMesoId);
					appDB.newMesocycle(mesoName, mesoStartDate, mesoEndDate, mesoNote, nWeeks, mesoSplit, mesoDrugs);
					idxModel = mesocyclesListModel.count - 1;

					/*let results2 = Database.newMesoDivision(mesoId, strSplitA, strSplitB, strSplitC, strSplitD, strSplitE, strSplitF);
					divisionModel.append({
						"divisionId": parseInt(results2.insertId),
						"mesoId": parseInt(mesoId),
						"splitA": strSplitA,
						"splitB": strSplitB,
						"splitC": strSplitC,
						"splitD": strSplitD,
						"splitE": strSplitE,
						"splitF": strSplitF
					});*/

					bNewMeso = false;
					createMesoCalendarObject(true);
				}
				else {
					mesocyclesListModel.setCurrentRow(idxModel);
					appDB.pass_object(mesocyclesListModel);
					appDB.updateMesocycle(mesoId, mesoName, mesoStartDate, mesoEndDate, mesoNote, nWeeks, mesoSplit, mesoDrugs);
					//Database.updateMesoDivision(mesoId, strSplitA, strSplitB, strSplitC, strSplitD, strSplitE, strSplitF);

					if (bDate1Changed || bDate2Changed || bMesoSplitChanged) {
						if (Database.checkIfCalendarForMesoExists(mesoId)) {
							if (mesocycleCalendarPage === null)
								createMesoCalendarObject(false);
							else {
								mesocycleCalendarPage.mesoStartDate = mesocyclesListModel.getDate(idxModel, 2);
								mesocycleCalendarPage.mesoEndDate = mesocyclesListModel.getDate(idxModel, 3);
								mesocycleCalendarPage.mesoSplit = mesocyclesListModel.get(idxModel, 6);
								mesocycleCalendarPage.mesoName = mesocyclesListModel.get(idxModel, 1);
							}
							appStackView.push(mesocycleCalendarPage);
							mesocycleCalendarPage.refactoryDatabase(mesoStartDate, mesoEndDate, mesoSplit, chkPreserveOldCalendar.checked, optPreserveOldCalendarUntilYesterday.checked);
							bDate1Changed = bDate2Changed = bMesoSplitChanged = false;
						}
					}
				}
				bDate1Changed = bDate2Changed = bMesoSplitChanged = false;
				bModified = false;
			} //onClicked
		} //btnSaveMeso
	} //footer

	function isDateWithinMeso(date) {
		return (date >= mesoStartDate) && (date <= mesoEndDate);
	}

	function createMesoCalendarObject(bshowpage) {
		var component = Qt.createComponent("MesoContent.qml", Qt.Asynchronous);

		function finishCreation() {
			mesocycleCalendarPage = component.createObject(mesoPropertiesPage, {
					mesoId: mesoId, mesoName: mesocyclesListModel.get(idxModel, 1), mesoStartDate: mesocyclesListModel.getDate(idxModel, 2),
					mesoEndDate: mesocyclesListModel.getDate(idxModel, 3), mesoSplit: mesocyclesListModel.get(idxModel, 6), bVisualLoad: bshowpage
			});
			if (bshowpage)
				appStackView.push(mesocycleCalendarPage, StackView.DontLoad);
		}
		if (component.status === Component.Ready)
			finishCreation();
		else
			component.statusChanged.connect(finishCreation);
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
} //Page
