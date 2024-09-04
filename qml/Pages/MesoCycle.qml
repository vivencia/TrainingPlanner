import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
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

	property int useMode
	property string mesoSplit: mesocyclesModel.get(mesoIdx, 6);
	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

	property bool bMesoNameOK: false
	property bool bNewMeso: mesoId === -1
	property bool bRealMeso: mesocyclesModel.isRealMeso(mesoIdx)
	property bool bOwnMeso: mesocyclesModel.isOwnMeso(mesoIdx)
	property bool bCalendarCreated: false
	property bool bPreserveOldCalendar: false
	property bool bPreserveOldCalendarUntilYesterday: false
	property bool bChangedCalendar: false
	property bool bCanSave: true

	onPageActivated: appDB.setWorkingMeso(mesoIdx);

	onPageDeActivated: {
		if (mesoId === -1)
			appDB.scheduleMesocycleRemoval(mesoIdx);
	}

	header: ToolBar {
		height: headerHeight
		enabled: mesoId !== -1

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
			imageSource: "meso-calendar.png"
			imageSize: 20
			visible: bOwnMeso

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 20
			}

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
				wrapMode: Text.WordWrap
				ToolTip.text: qsTr("Name too short")
				width: parent.width - 20
				Layout.alignment: Qt.AlignHCenter
				Layout.minimumWidth: width
				Layout.maximumWidth: width


				onTextEdited: {
					bMesoNameOK = text.length >= 5;
					ToolTip.visible = !bMesoNameOK;
				}

				onEditingFinished: {
					if (bMesoNameOK)
						bMesoNameOK = mesocyclesModel.set(mesoIdx, 1, text);
				}

				onEnterOrReturnKeyPressed: {
					if (cboCoaches.visible)
						cboCoaches.forceActiveFocus();
					else
						cboClients.forceActiveFocus();
				}
			}

			RowLayout {
				visible: useMode >= 3
				height: 30
				spacing: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true

				Label {
					id: lblCoaches
					text: mesocyclesModel.columnLabel(7)
					font.bold: true
					color: AppSettings.fontColor
					width: windowWidth/2 - 10

					FontMetrics {
						id: fontMetrics
						font.family: lblCoaches.font.family
						font.pointSize: AppSettings.fontSizeText
					}
				}

				TPComboBox {
					id: cboCoaches
					implicitWidth: windowWidth/2
					Layout.minimumWidth: width

					model: ListModel {
						id: coachesModel
					}

					Component.onCompleted: {
						updateCoachesModel(2);
						if (!bNewMeso)
							currentIndex = find(mesocyclesModel.get(mesoIdx, 7));
						else
							currentIndex = find(userModel.getCurrentUserName(true));
						if (currentIndex === -1)
							displayText = qsTr("(Select coach ...)");
					}

					onActivated: (index) => {
						mesocyclesModel.set(mesoIdx, 7, textAt(index));
						displayText = currentText;
					}
				}

				TPButton {
					imageSource: "manage-coaches"

					onClicked: appDB.openClientsOrCoachesPage(false, true);
				}
			}

			TPCheckBox {
				text: qsTr("This plan is for myself")
				checked: bOwnMeso
				visible: useMode === 2 || useMode === 4
				Layout.leftMargin: 5
				Layout.fillWidth: true

				onClicked: {
					bOwnMeso = checked;
					mesocyclesModel.set(mesoIdx, 8, bOwnMeso ? userModel.userName(0) : userModel.getCurrentUserName(false));
				}
			}

			RowLayout {
				visible: useMode === 2 ? true : !bOwnMeso
				height: 30
				spacing: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true

				Label {
					id: lblClients
					text: mesocyclesModel.columnLabel(8)
					font.bold: true
					color: AppSettings.fontColor
					width: fontMetrics2.boundingRect(text).width

					FontMetrics {
						id: fontMetrics2
						font.family: lblClients.font.family
						font.pointSize: AppSettings.fontSizeText
					}
				}

				TPComboBox {
					id: cboClients
					implicitWidth: windowWidth*0.6
					Layout.minimumWidth: width

					model: ListModel {
						id: clientsModel
					}

					Component.onCompleted: {
						updateCoachesModel(0);
						if (!bNewMeso)
							currentIndex = find(mesocyclesModel.get(mesoIdx, 8));
						else
							currentIndex = find(userModel.getCurrentUserName(false));
						if (currentIndex === -1)
							displayText = qsTr("(Select client ...)");
					}

					onActivated: (index) => {
						mesocyclesModel.set(mesoIdx, 8, textAt(index));
						displayText = currentText;
					}
				}

				TPButton {
					id: btnManageClients
					imageSource: "manage-clients"

					onClicked: appDB.openClientsOrCoachesPage(true, false);
				}
			}

			RowLayout {
				height: 30
				spacing: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true

				Label {
					text: mesocyclesModel.columnLabel(10)
					font.bold: true
					color: AppSettings.fontColor
					width: (parent.width - 20)*0.2
				}

				TPComboBox {
					id: cboMesoType
					model: mesoTypeModel
					width: (mesoPropertiesPage.width - 20)*0.75
					Layout.minimumWidth: width

					Component.onCompleted: {
						currentIndex = find(mesocyclesModel.get(mesoIdx, 10));
						if (currentIndex === -1)
							currentIndex = 6;
					}

					onActivated: (index) => {
						if (index < 6)
							mesocyclesModel.set(mesoIdx, 10, textAt(index));
						else
							txtMesoTypeOther.forceActiveFocus();
					}

					ListModel {
						id: mesoTypeModel
						ListElement { text: qsTr("Weigth Loss"); value: 0; }
						ListElement { text: qsTr("Muscle Gain"); value: 1; }
						ListElement { text: qsTr("Bulking"); value: 2; }
						ListElement { text: qsTr("Pre-contest"); value: 3; }
						ListElement { text: qsTr("Strength Build-up"); value: 4; }
						ListElement { text: qsTr("Recovery"); value: 5; }
						ListElement { text: qsTr("Other"); value: 6; }
					}
				}
			}

			TPTextInput {
				id: txtMesoTypeOther
				text: mesocyclesModel.get(mesoIdx, 10)
				width: parent.width - 20
				visible: cboMesoType.currentIndex === 6
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				Layout.leftMargin: 5

				onEditingFinished: mesocyclesModel.set(mesoIdx, 10, text);
				onEnterOrReturnKeyPressed: txtMesoFile.forceActiveFocus();
			}


			Label {
				text: qsTr("Instructions file")
				font.bold: true
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				color: AppSettings.fontColor
			}

			RowLayout {
				height: 30
				spacing: 0
				Layout.leftMargin: 5
				Layout.fillWidth: true

				TPTextInput {
					id: txtMesoFile
					text: runCmd.getFileName(mesocyclesModel.get(mesoIdx, 9))
					width: (mesoPropertiesPage.width - 20)*0.8
					Layout.minimumWidth: width

					onEditingFinished: mesocyclesModel.set(mesoIdx, 9, text);

					onEnterOrReturnKeyPressed: {
						if (bNewMeso)
							caldlg.open();
						else
							txtMesoSplit.forceActiveFocus();
					}
				}

				TPButton {
					id: btnChooseMesoFile
					imageSource: "choose-file"
					Layout.leftMargin: -5

					onClicked: fileDialog.open();

					FileDialog {
						id: fileDialog
						title: qsTr("Choose the instruction's file for this mesocycles")
						nameFilters: [qsTr("PDF Files") + " (*.pdf)", qsTr("Documents") + " (*.doc *.docx *.odt)"]
						options: FileDialog.ReadOnly
						currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
						fileMode: FileDialog.OpenFile

						onAccepted: {
							txtMesoFile.text = runCmd.getFileName(selectedFile);
							mesocyclesModel.set(mesoIdx, 9, selectedFile);
						}
					}
				}

				TPButton {
					id: btnOpenMesoFile
					imageSource: txtMesoFile.text.indexOf("pdf") !== -1 ? "pdf-icon" : "doc-icon"
					visible: runCmd.canReadFile(mesocyclesModel.get(mesoIdx, 9))
					Layout.leftMargin: -10

					onClicked: appDB.viewExternalFile(mesocyclesModel.get(mesoIdx, 9));
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
						bCanSave = bNewMeso;
						if (mesocyclesModel.setDate(mesoIdx, 2, caldlg.selectedDate)) {
							txtMesoStartDate.text = runCmd.formatDate(caldlg.selectedDate);
							if (bNewMeso && bRealMeso)
								caldlg2.open();
							else
								showCalendarChangedDialog();
						}
					}
				}

				TPButton {
					id: btnStartDate
					imageSource: "calendar.png"
					anchors.left: txtMesoStartDate.right
					anchors.verticalCenter: txtMesoStartDate.verticalCenter

					onClicked: caldlg.open();
				}
			}

			TPCheckBox {
				text: qsTr("Mesocycle-style plan")
				checked: bRealMeso
				Layout.leftMargin: 5
				Layout.fillWidth: true

				onPressAndHold: ToolTip.show(qsTr("A Mesocycle is a short-term plan, with defined starting and ending points and a specific goal in sight"), 5000);

				onClicked: {
					bCanSave = bNewMeso;
					bRealMeso = checked;
					mesocyclesModel.setIsRealMeso(mesoIdx, bRealMeso);
					mesocyclesModel.setDate(mesoIdx, 3, bRealMeso ? maximumMesoEndDate : mesocyclesModel.getEndDate(mesoIdx));
					txtMesoEndDate.text = runCmd.formatDate(mesocyclesModel.getEndDate(mesoIdx));
					if (!bNewMeso)
						showCalendarChangedDialog();
				}
			}

			Label {
				text: mesocyclesModel.columnLabel(3)
				font.bold: true
				color: AppSettings.fontColor
				visible: bRealMeso
				Layout.leftMargin: 5
			}
			TPTextInput {
				id: txtMesoEndDate
				text: runCmd.formatDate(mesocyclesModel.getDate(mesoIdx, 3))
				readOnly: true
				visible: bRealMeso
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2

				CalendarDialog {
					id: caldlg2
					showDate: mesocyclesModel.getDate(mesoIdx, 3)
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: function(date) {
						bCanSave = bNewMeso;
						if (mesocyclesModel.setDate(mesoIdx, 3, caldlg2.selectedDate)) {
							txtMesoEndDate.text = runCmd.formatDate(caldlg2.selectedDate)
							txtMesoSplit.forceActiveFocus();
							if (!bNewMeso)
								showCalendarChangedDialog();
						}
					}
				}

				TPButton {
					id: btnEndDate
					imageSource: "calendar.png"
					anchors.left: txtMesoEndDate.right
					anchors.verticalCenter: txtMesoEndDate.verticalCenter

					onClicked: caldlg2.open();
				}
			}

			Label {
				id: lblnWeeks
				text: mesocyclesModel.columnLabel(5)
				font.bold: true
				color: AppSettings.fontColor
				visible: bRealMeso
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: 50
				Layout.maximumWidth: 50
			}

			TPTextInput {
				id: txtMesoNWeeks
				text: mesocyclesModel.get(mesoIdx, 5)
				readOnly: true
				width: txtMesoEndDate.width
				visible: bRealMeso
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
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
				maximumLength: 7
				width: txtMesoStartDate.width
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				ToolTip.text: qsTr("On a mesocycle, there should be at least one rest day(R)")

				property bool bMesoSplitOK: false

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
							if (!bNewMeso)
								showCalendarChangedDialog();
						}
					}
				}

				onEnterOrReturnKeyPressed: {
					if (!paneTrainingSplit.shown)
						btnTrainingSplit.clicked();
					JSF.moveFocusToNextField('0');
				}

				TPButton {
					id: btnTrainingSplit
					imageSource: paneTrainingSplit.shown ? "fold-up.png" : "fold-down.png"
					hasDropShadow: false

					anchors {
						left: txtMesoSplit.right
						verticalCenter: txtMesoSplit.verticalCenter
						leftMargin: 10
					}

					onClicked: paneTrainingSplit.shown = !paneTrainingSplit.shown
				}
			}

			Pane {
				id: paneTrainingSplit
				visible: height > 0
				height: shown ? implicitHeight : 0
				padding: 0
				Layout.fillWidth: true
				Layout.leftMargin: 20

				property bool shown: false

				Behavior on height {
					NumberAnimation {
						duration: 300
						easing.type: Easing.InOutBack
					}
				}

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

						onEnterOrReturnKeyPressed: JSF.moveFocusToNextField('A');
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

						onEnterOrReturnKeyPressed: JSF.moveFocusToNextField('B');
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

						onEnterOrReturnKeyPressed: JSF.moveFocusToNextField('C');
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

						onEnterOrReturnKeyPressed: JSF.moveFocusToNextField('D');
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

						onEnterOrReturnKeyPressed: JSF.moveFocusToNextField('E');
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

						onEnterOrReturnKeyPressed: JSF.moveFocusToNextField('F');
					}

					TPButton {
						id: btnCreateExercisePlan
						text: qsTr("Exercises Planner")
						Layout.row: 6
						Layout.column: 0
						Layout.columnSpan: 2
						Layout.alignment: Qt.AlignCenter

						onClicked: appDB.createExercisesPlannerPage();
					}
				} //GridLayout
			} //Pane

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

	Component.onCompleted: {
		JSF.checkWhetherCanCreatePlan();
		if (bNewMeso)
			txtMesoName.forceActiveFocus();
		mesocyclesModel.modifiedChanged.connect(saveMeso);
		mesocyclesModel.realMesoChanged.connect(function (mesoidx) {
			if (mesoidx === mesoIdx)
				bRealMeso = mesocyclesModel.isRealMeso(mesoidx);
		});
		mesoSplitModel.modifiedChanged.connect(saveMeso);
		userModel.userAdded.connect(updateCoachesModel);
	}

	function updateCoachesModel(use_mode: int) {
		if (use_mode === 2 || use_mode === 4) {
			const coaches = userModel.getCoaches();
			coachesModel.clear();
			for(var i = 0; i < coaches.length; ++i)
				coachesModel.append({ "text": coaches[i], "value": i});
		}
		else if (use_mode === 0) {
			const clients = userModel.getClients();
			clientsModel.clear();
			for(var x = 0; x < clients.length; ++x)
				clientsModel.append({ "text": clients[i], "value": i});
		}
	}

	function saveMeso() {
		if (bCanSave) {
			appDB.saveMesocycle(bChangedCalendar, bPreserveOldCalendar, bPreserveOldCalendarUntilYesterday);
			if (bNewMeso)
				mesoId = mesocyclesModel.getInt(mesoIdx, 0);
			bChangedCalendar = false;
		}
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

	property bool alreadyCalled: false
	property TPComplexDialog calendarChangeDlg: null
	function showCalendarChangedDialog() {
		if (!calendarChangeDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				calendarChangeDlg = component.createObject(mainwindow, { parentPage: mesoPropertiesPage, title:qsTr("Adjust meso calendar?"),
					button1Text: qsTr("Yes"), button2Text: qsTr("No"), customItemSource:"TPAdjustMesoCalendarFrame.qml" });
				calendarChangeDlg.button1Clicked.connect(preserveOldCalenarInfo);
				calendarChangeDlg.button2Clicked.connect(function() { alreadyCalled = false; }); //A "No", warrants a possible new confirmation
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		if (alreadyCalled) {
			if (AppSettings.alwaysAskConfirmation) {
				if (!bNewMeso)
					calendarChangeDlg.show(-1);
				else
					if (bCalendarCreated)
						preserveOldCalenarInfo();
			}
			else
				preserveOldCalenarInfo();
		}
		else {
			if (!bNewMeso || bCalendarCreated)
				calendarChangeDlg.show(-1);
			alreadyCalled = true;
		}
	}

	function preserveOldCalenarInfo() {
		bPreserveOldCalendar = calendarChangeDlg.customBoolProperty1;
		bPreserveOldCalendarUntilYesterday = calendarChangeDlg.customBoolProperty2;
		bChangedCalendar = true;
		txtMesoNWeeks.text = runCmd.calculateNumberOfWeeks(mesocyclesModel.getDate(mesoIdx, 2), mesocyclesModel.getDate(mesoIdx, 3));
		mesocyclesModel.modified = false; //force emit modifiedChanged()
		bCanSave = true;
		mesocyclesModel.set(mesoIdx, 5, txtMesoNWeeks.text);
		bPreserveOldCalendar = bPreserveOldCalendarUntilYesterday = false;
	}
} //Page
