import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import com.vivenciasoftware.qmlcomponents

import "../"
import "../Dialogs"
import "../TPWidgets"

TPPage {
	id: mesoPropertiesPage
	objectName: "mesoPage"

	required property QmlItemManager itemManager

	property int useMode
	property string mesoSplit: mesocyclesModel.get(itemManager.mesoIdx, 6);
	property date minimumMesoStartDate
	property date maximumMesoEndDate
	property date calendarStartDate //Also used on newMeso to revert data to the original value gathered from HomePage

	property int muscularGroupId
	property bool bDoNotRemoveMeso: false
	property bool bMesoNameOK: false
	property bool bNewMeso: mesocyclesModel.getInt(itemManager.mesoIdx, 0) === -1
	property bool bRealMeso: mesocyclesModel.isRealMeso(itemManager.mesoIdx)
	property bool bOwnMeso: mesocyclesModel.isOwnMeso(itemManager.mesoIdx)
	property bool bPreserveOldCalendar: false
	property bool bPreserveOldCalendarUntilYesterday: false

	onPageActivated: {
		mesocyclesModel.setCurrentMesoIdx(itemManager.mesoIdx);
		if (bDoNotRemoveMeso)
			bDoNotRemoveMeso = false;
	}

	onPageDeActivated: {
		if (!bDoNotRemoveMeso)
		{
			if (bNewMeso) {
				if (mesocyclesModel.get(itemManager.mesoIdx, 0) >= 0)
					bNewMeso = false;
				else
					itemManager.scheduleMesocycleRemoval();
			}
		}
	}

	header: ToolBar {
		height: headerHeight
		enabled: mesocyclesModel.get(itemManager.mesoIdx, 0) !== -1

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

			onClicked: {
				bDoNotRemoveMeso = true;
				itemManager.getMesoCalendarPage();
			}
		}
	}

	ScrollView {
		anchors.fill: parent
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth //stops bouncing to the sides
		contentHeight: colMain.implicitHeight + 20

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
				text: mesocyclesModel.get(itemManager.mesoIdx, 1);
				wrapMode: Text.WordWrap
				ToolTip.text: qsTr("Name too short")
				width: parent.width - 20
				Layout.leftMargin: 10
				Layout.minimumWidth: width
				Layout.maximumWidth: width


				onTextEdited: {
					bMesoNameOK = text.length >= 5;
					ToolTip.visible = !bMesoNameOK;
				}

				onEditingFinished: {
					if (bMesoNameOK)
						bMesoNameOK = mesocyclesModel.set(itemManager.mesoIdx, 1, text);
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
						const coaches = userModel.getCoaches();
						for(var i = 0; i < coaches.length; ++i)
							coachesModel.append({ "text": coaches[i], "value": i});
						if (!bNewMeso)
							currentIndex = find(mesocyclesModel.get(itemManager.mesoIdx, 7));
						else
							currentIndex = find(userModel.getCurrentUserName(true));
						if (currentIndex === -1)
							displayText = qsTr("(Select coach ...)");
					}

					onActivated: (index) => {
						mesocyclesModel.set(itemManager.mesoIdx, 7, textAt(index));
						displayText = currentText;
					}
				}

				TPButton {
					imageSource: "manage-coaches"

					onClicked: {
						bDoNotRemoveMeso = true;
						appControl.openClientsOrCoachesPage(false, true);
					}
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
					mesocyclesModel.setOwnMeso(itemManager.mesoIdx, bOwnMeso);
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
						const clients = userModel.getClients();
						for(var x = 0; x < clients.length; ++x)
							clientsModel.append({ "text": clients[x], "value": x});
						if (!bNewMeso)
							currentIndex = find(mesocyclesModel.get(itemManager.mesoIdx, 8));
						else
							currentIndex = find(userModel.getCurrentUserName(false));
						if (currentIndex === -1)
							displayText = qsTr("(Select client ...)");
					}

					onActivated: (index) => {
						mesocyclesModel.set(itemManager.mesoIdx, 8, textAt(index));
						displayText = currentText;
					}
				}

				TPButton {
					id: btnManageClients
					imageSource: "manage-clients"

					onClicked: {
						bDoNotRemoveMeso = true;
						appControl.openClientsOrCoachesPage(true, false);
					}
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
						currentIndex = find(mesocyclesModel.get(itemManager.mesoIdx, 10));
						if (currentIndex === -1)
							currentIndex = 6;
					}

					onActivated: (index) => {
						if (index < 6)
							mesocyclesModel.set(itemManager.mesoIdx, 10, textAt(index));
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
				text: mesocyclesModel.get(itemManager.mesoIdx, 10)
				width: parent.width - 20
				visible: cboMesoType.currentIndex === 6
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				Layout.leftMargin: 5

				onEditingFinished: mesocyclesModel.set(itemManager.mesoIdx, 10, text);
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
					text: appUtils.getFileName(mesocyclesModel.get(itemManager.mesoIdx, 9))
					width: (mesoPropertiesPage.width - 20)*0.8
					Layout.minimumWidth: width

					onEditingFinished: mesocyclesModel.set(itemManager.mesoIdx, 9, text);

					onEnterOrReturnKeyPressed: {
						if (bNewMeso)
							caldlg.open();
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
							const chosenFile = appUtils.getCorrectPath(currentFile);
							txtMesoFile.text = appUtils.getFileName(chosenFile);
							mesocyclesModel.set(itemManager.mesoIdx, 9, chosenFile);
							btnOpenMesoFile.visible = appUtils.canReadFile(chosenFile);
						}
					}
				}

				TPButton {
					id: btnOpenMesoFile
					imageSource: txtMesoFile.text.indexOf("pdf") !== -1 ? "pdf-icon" : "doc-icon"
					visible: appUtils.canReadFile(mesocyclesModel.get(itemManager.mesoIdx, 9))
					Layout.leftMargin: -10

					onClicked: osInterface.viewExternalFile(mesocyclesModel.get(itemManager.mesoIdx, 9));
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
				text: appUtils.formatDate(mesocyclesModel.getDate(itemManager.mesoIdx, 2))
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
						if (mesocyclesModel.setMesoStartDate(itemManager.mesoIdx, caldlg.selectedDate)) {
							txtMesoStartDate.text = appUtils.formatDate(caldlg.selectedDate);
							txtMesoNWeeks.text = appUtils.calculateNumberOfWeeks(mesocyclesModel.getDate(itemManager.mesoIdx, 2), mesocyclesModel.getDate(itemManager.mesoIdx, 3));
							mesocyclesModel.set(itemManager.mesoIdx, 5, txtMesoNWeeks.text);
							if (bNewMeso && bRealMeso)
								caldlg2.open();
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
					bRealMeso = checked;
					mesocyclesModel.setIsRealMeso(itemManager.mesoIdx, bRealMeso);
					mesocyclesModel.setDate(itemManager.mesoIdx, 3, bRealMeso ? maximumMesoEndDate : mesocyclesModel.getEndDate(itemManager.mesoIdx));
					txtMesoEndDate.text = appUtils.formatDate(mesocyclesModel.getEndDate(itemManager.mesoIdx));
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
				text: appUtils.formatDate(mesocyclesModel.getDate(itemManager.mesoIdx, 3))
				readOnly: true
				visible: bRealMeso
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2

				CalendarDialog {
					id: caldlg2
					showDate: mesocyclesModel.getDate(itemManager.mesoIdx, 3)
					initDate: minimumMesoStartDate
					finalDate: maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: function(date) {
						if (mesocyclesModel.setMesoEndDate(itemManager.mesoIdx, caldlg2.selectedDate)) {
							txtMesoEndDate.text = appUtils.formatDate(caldlg2.selectedDate)
							txtMesoNWeeks.text = appUtils.calculateNumberOfWeeks(mesocyclesModel.getDate(itemManager.mesoIdx, 2), mesocyclesModel.getDate(itemManager.mesoIdx, 3));
							mesocyclesModel.set(itemManager.mesoIdx, 5, txtMesoNWeeks.text);
						}
						mesoSplitSetup.forcusOnFirstItem();
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
				text: mesocyclesModel.get(itemManager.mesoIdx, 5)
				readOnly: true
				width: txtMesoEndDate.width
				visible: bRealMeso
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
			}

			MesoSplitSetup {
				id: mesoSplitSetup
				Layout.fillWidth: true
				Layout.leftMargin: -5
			}

			Label {
				text: mesocyclesModel.columnLabel(4)
				font.bold: true
				color: AppSettings.fontColor
				Layout.leftMargin: 5
				Layout.topMargin: 10
			}
			Flickable {
				height: Math.min(contentHeight, 60)
				width: parent.width - 25
				contentHeight: txtMesoNotes.implicitHeight
				Layout.leftMargin: 5
				Layout.minimumWidth: width
				Layout.maximumWidth: width

				TextArea.flickable: TextArea {
					id: txtMesoNotes
					text: mesocyclesModel.get(itemManager.mesoIdx, 4)
					color: AppSettings.fontColor

					background: Rectangle {
						color: AppSettings.primaryColor
						opacity: 0.8
						radius: 6
						border.color: AppSettings.fontColor
					}

					onEditingFinished: mesocyclesModel.set(itemManager.mesoIdx, 4, text);
				}

				Component.onCompleted: vBar2.position = 0
				ScrollBar.vertical: ScrollBar { id: vBar2 }
				ScrollBar.horizontal: ScrollBar {}
			}
		} //ColumnLayout
	} //ScrollView

	Component.onCompleted: {
		if (bNewMeso)
			txtMesoName.forceActiveFocus();
		mesocyclesModel.realMesoChanged.connect(function (mesoidx) {
			if (itemManager.mesoIdx === mesoidx)
				bRealMeso = mesocyclesModel.isRealMeso(itemManager.mesoIdx);
		});
		userModel.userAdded.connect(updateCoachesModel);
		mesocyclesModel.mesoCalendarFieldsChanged.connect(showCalendarChangedDialog);
		mesocyclesModel.modifiedChanged.connect(function() { appDB.saveMesocycle(itemManager.mesoIdx) });
	}

	function updateCoachesModel(userrow: int) {
		const use_mode = userModel.appUseMode(userrow);
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
				clientsModel.append({ "text": clients[x], "value": x});
		}
	}

	property bool alreadyCalled: false
	property TPComplexDialog calendarChangeDlg: null
	function showCalendarChangedDialog() {
		if (!calendarChangeDlg) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				calendarChangeDlg = component.createObject(mainwindow, { parentPage: mesoPropertiesPage, title:qsTr("Adjust meso calendar?"),
					button1Text: qsTr("Yes"), button2Text: qsTr("No"), customItemSource:"TPAdjustMesoCalendarFrame.qml" });
				calendarChangeDlg.button1Clicked.connect(changeCalendar);
				calendarChangeDlg.button2Clicked.connect(function() { alreadyCalled = false; }); //A "No", warrants a possible new confirmation
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
		if (alreadyCalled)
			changeCalendar();
		else {
			calendarChangeDlg.show(-1);
			alreadyCalled = true;
		}
	}

	function changeCalendar() {
		bPreserveOldCalendar = calendarChangeDlg.customBoolProperty1;
		bPreserveOldCalendarUntilYesterday = calendarChangeDlg.customBoolProperty2;
		appDB.changeMesoCalendar(itemManager.mesoIdx, bPreserveOldCalendar, bPreserveOldCalendarUntilYesterday);
	}
} //Page
