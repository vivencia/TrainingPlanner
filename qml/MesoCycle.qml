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
		height: btnManageMeso.height + 20
		enabled: !bNewMeso && !bModified
		background: Rectangle {
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}

		TPButton {
			id: btnManageMeso
			text: qsTr("Calendar")
			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 20
			}
			imageSource: "qrc:/images/"+lightIconFolder+"edit-mesocycle.png"

			onClicked: {
				var id;
				function calendarPageReady(object, id) {
					if (id === 35) {
						appDB.getPage.disconnect(calendarPageReady);
						appMainMenu.addShortCut( qsTr("Calendar: ") + mesocyclesModel.get(mesoIdx, 1) , object);
					}
				}

				appDB.getPage.connect(calendarPageReady);
				appDB.getMesoCalendar(true);
			}
		}

		TPButton {
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
				text: qsTr("Mesocycle's name:")
				font.bold: true
				Layout.alignment: Qt.AlignHCenter
				Layout.topMargin: 10
				color: "white"
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
				text: qsTr("Start date for meso:")
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: "white"
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

				RoundButton {
					id: btnStartDate
					anchors.left: txtMesoStartDate.right
					anchors.verticalCenter: txtMesoStartDate.verticalCenter
					width: 40
					height: 40
					icon.source: "qrc:/images/white/calendar.png"

					onClicked: caldlg.open();
				}
			}

			Label {
				text: qsTr("End date for meso:")
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: "white"
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

				RoundButton {
					id: btnEndDate
					anchors.left: txtMesoEndDate.right
					anchors.verticalCenter: txtMesoEndDate.verticalCenter
					width: 40
					height: 40
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
				id: txtMesoNWeeks
				text: mesocyclesModel.get(mesoIdx, 5)
				width: txtMesoEndDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				readOnly: true
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
				text: mesocyclesModel.get(mesoIdx, 6)
				validator: regEx
				width: txtMesoStartDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				ToolTip.text: qsTr("On a mesocycle, there should be at least one rest day(R)")

				RoundButton {
					id: btnTrainingSplit
					width: 40
					height: 40
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
					border.color: "white"
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
						color: "white"
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
						color: "white"
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
						Layout.row: 2
						Layout.column: 0
						color: "white"
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
						Layout.row: 3
						Layout.column: 0
						color: "white"
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
						Layout.row: 4
						Layout.column: 0
						color: "white"
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
						Layout.row: 5
						Layout.column: 0
						color: "white"
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

						onClicked: {
							var component = Qt.createComponent("ExercisesPlanner.qml", Qt.Asynchronous);

							function finishCreation() {
								var mesoPlannerObject = component.createObject(mesoPropertiesPage, {
										mesoId:mesoId, mesoIdx:mesoIdx, mesoSplit:mesoSplit
								});
								appMainMenu.addShortCut( qsTr("Exercises Planner: ") + mesocyclesModel.get(mesoIdx, 1) , mesoPlannerObject);
							}

							if (component.status === Component.Ready)
								finishCreation();
							else
								component.statusChanged.connect(finishCreation);
						} //onClicked
					} //TPButton
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
					text: mesocyclesModel.get(mesoIdx, 7)
					color: "white"

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
				text: qsTr("Mesocycle's considerations: ")
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
					text: mesocyclesModel.get(mesoIdx, 4)
					color: "white"

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
	}

	Component.onDestruction: {
		if (mesoStatisticsPage !== null)
			mesoStatisticsPage.destroy();
	}

	footer: ToolBar {
		id: mesoCycleToolBar
		width: parent.width
		height: 55

		background: Rectangle {
			color: AppSettings.primaryDarkColor
			opacity: 0.7
		}

		TPButton {
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
			imageSource: "qrc:/images/"+lightIconFolder+"save-day.png"
			enabled: bNewMeso ? bMesoNameOK : bModified

			onClicked: {
				if (bNewMeso) {
					function getMesoId() {
						appDB.databaseReady.disconnect(getMesoId);
						appDB.newMesoSplit(txtSplitA.text, txtSplitB.text, txtSplitC.text, txtSplitD.text, txtSplitE.text, txtSplitF.text);
						bNewMeso = false;
					}

					appDB.databaseReady.connect(getMesoId);
					appDB.newMesocycle(txtMesoName.text, mesoStartDate, mesoEndDate, txtMesoNotes.text, txtMesoNWeeks.text, txtMesoSplit.text, txtMesoDrugs.text);
					mesoIdx = mesocyclesModel.count - 1;
					bStartDateChanged = bEndDateChanged = bMesoSplitChanged = false;
				}
				else {
					function canProceed() {
						appDB.databaseReady.disconnect(canProceed);
						appDB.updateMesoSplit(txtSplitA.text, txtSplitB.text, txtSplitC.text, txtSplitD.text, txtSplitE.text, txtSplitF.text)

						if (bStartDateChanged || bEndDateChanged || bMesoSplitChanged) {
							appDB.changeMesoCalendar(mesoStartDate, mesoEndDate, mesoSplit, chkPreserveOldCalendar.checked, optPreserveOldCalendarUntilYesterday.checked);
							bStartDateChanged = bEndDateChanged = bMesoSplitChanged = false;
						}
					}
					appDB.databaseReady.connect(canProceed);
					appDB.updateMesocycle(txtMesoName.text, mesoStartDate, mesoEndDate, txtMesoNotes.text, txtMesoNWeeks.text, txtMesoSplit.text, txtMesoDrugs.text);
				}
				bModified = false;
			} //onClicked
		} //btnSaveMeso
	} //footer

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
			appDB.removeMesocycle();
	}
} //Page
