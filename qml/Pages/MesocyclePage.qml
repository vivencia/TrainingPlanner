import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../Dialogs"
import "../TPWidgets"

TPPage {
	id: mesoPropertiesPage
	objectName: "mesoPage"

	required property MesoManager mesoManager
	readonly property bool bMesoNameOK: txtMesoName.text.length >= 5

	header: ToolBar {
		height: headerHeight

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: appSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: appSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: appSettings.primaryColor; }
				GradientStop { position: 0.75; color: appSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			text: qsTr("Calendar")
			imageSource: "meso-calendar.png"
			imageSize: 20
			enabled: !mesoManager.isNewMeso

			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: 20
			}

			onClicked: mesoManager.getCalendarPage();
		}

		TPImage {
			id: imgNewMesoOK
			source: "set-completed.png"
			enabled: !mesoManager.isNewMeso
			width: 45
			height: 45

			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 50
			}
		}

		TPLabel {
			id: lblNewMesoRequiredFieldsCounter
			text: parseInt(fieldCounter)
			visible: mesoManager.isNewMeso
			font: AppGlobals.smallFont

			property int fieldCounter: 4
			anchors {
				left: imgNewMesoOK.right
				leftMargin: 5
				bottom: parent.bottom
			}

			function decreaseCounter() { fieldCounter--; }
			function increaseCounter() { fieldCounter++; }
		}
	}

	ScrollView {
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn
		contentWidth: availableWidth
		contentHeight: colMain.implicitHeight + 20

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 10
			bottomMargin: 10
		}

		ColumnLayout {
			id: colMain
			spacing: 5
			anchors.fill: parent

			TPLabel {
				text: mesoManager.nameLabel

				TPButton {
					imageSource: "set-completed"
					fixedSize: true
					checkable: true
					visible: mesoManager.isNewMeso
					enabled: bMesoNameOK
					height: 25
					width: 25

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onCheck: {
						txtMesoName.readOnly = checked;
						if (checked) {
							mesoManager.name = txtMesoName.text;
							lblNewMesoRequiredFieldsCounter.decreaseCounter();
						}
						else {
							txtMesoName.forceActiveFocus();
							lblNewMesoRequiredFieldsCounter.increaseCounter();
						}
					}
				}
			}

			TPTextInput {
				id: txtMesoName
				text: mesoManager.name
				ToolTip.text: qsTr("Name too short")
				ToolTip.visible:!bMesoNameOK;
				width: 0.9*parent.width
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				onEnterOrReturnKeyPressed: {
					if (cboCoaches.visible)
						cboCoaches.forceActiveFocus();
					else
						cboClients.forceActiveFocus();
				}
			}

			TPLabel {
				id: lblCoaches
				text: mesoManager.coachLabel
				visible: mesoManager.hasCoach
				Layout.fillWidth: true
			}

			TPComboBox {
				id: cboCoaches
				currentIndex: userModel.currentCoach(userModel.userRow(mesoManager.client))
				visible: mesoManager.hasCoach
				implicitWidth: parent.width*0.8

				model: ListModel {
					id: coachesModel

					Component.onCompleted: {
						const coaches = userModel.getCoaches();
						for(var i = 0; i < coaches.length; ++i)
							append({ "text": coaches[i], "value": i, "enabled": true });
					}
				}

				onActivated: (index) => mesoManager.coach = textAt(index);

				TPButton {
					imageSource: "manage-coaches"

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: appControl.openClientsOrCoachesPage(false, true);
				}
			}

			TPCheckBox {
				text: qsTr("This plan is for myself")
				checked: mesoManager.ownMeso
				visible: mesoManager.ownerIsCoach
				Layout.fillWidth: true

				onClicked: mesoManager.ownMeso = checked;
			}

			RowLayout {
				visible: mesoManager.ownerIsCoach && !mesoManager.ownMeso
				spacing: 0
				Layout.fillWidth: true

				TPLabel {
					text: mesoManager.clientLabel
					width: 0.30*parent.width
				}

				TPComboBox {
					id: cboClients
					//editText: mesoManager.client
					currentIndex: userModel.currentClient(userModel.userRow(mesoManager.client))
					implicitWidth: 0.6*parent.width
					Layout.minimumWidth: width

					model: ListModel {
						id: clientsModel

						Component.onCompleted: {
							const clients = userModel.getClients();
							for(var x = 0; x < clients.length; ++x)
								append({ "text": clients[x], "value": x, "enabled":true });
						}
					}

					onActivated: (index) => mesoManager.client = textAt(index);
				}

				TPButton {
					id: btnManageClients
					imageSource: "manage-clients"

					onClicked: appControl.openClientsOrCoachesPage(true, false);
				}
			}

			RowLayout {
				spacing: 5
				Layout.fillWidth: true

				TPLabel {
					text: mesoManager.typeLabel
					width: 0.2*parent.width
				}

				TPComboBox {
					id: cboMesoType
					model: mesoTypeModel
					editText: mesoManager.type
					width: 0.75*parent.width
					Layout.minimumWidth: width

					onActivated: (index) => {
						if (index < 6)
							mesoManager.type = textAt(index);
						else
							txtMesoTypeOther.forceActiveFocus();
					}

					ListModel {
						id: mesoTypeModel
						ListElement { text: qsTr("Weigth Loss"); value: 0; enabled: true; }
						ListElement { text: qsTr("Muscle Gain"); value: 1; enabled: true; }
						ListElement { text: qsTr("Bulking"); value: 2; enabled: true; }
						ListElement { text: qsTr("Pre-contest"); value: 3; enabled: true; }
						ListElement { text: qsTr("Strength Build-up"); value: 4; enabled: true; }
						ListElement { text: qsTr("Recovery"); value: 5; enabled: true; }
						ListElement { text: qsTr("Other"); value: 6; enabled: true; }
					}
				}
			}

			TPTextInput {
				id: txtMesoTypeOther
				text: mesoManager.type
				visible: cboMesoType.currentIndex === 6
				width: parent.width
				Layout.maximumWidth: width

				onEditingFinished: mesoManager.type = text;
				onEnterOrReturnKeyPressed: txtMesoFile.forceActiveFocus();
			}


			TPLabel {
				text: qsTr("Instructions file")
			}

			RowLayout {
				spacing: 0
				Layout.fillWidth: true

				TPTextInput {
					id: txtMesoFile
					text: mesoManager.fileName
					readOnly: true
					width: 0.8*parent.width
					Layout.maximumWidth: width
					Layout.minimumWidth: width
				}

				TPButton {
					id: btnChooseMesoFile
					imageSource: "choose-file"
					Layout.leftMargin: 5

					onClicked: fileDialog.open();

					FileDialog {
						id: fileDialog
						title: qsTr("Choose the instruction's file for this mesocycles")
						nameFilters: [qsTr("PDF Files") + " (*.pdf)", qsTr("Documents") + " (*.doc *.docx *.odt)"]
						options: FileDialog.ReadOnly
						currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
						fileMode: FileDialog.OpenFile

						onAccepted: mesoManager.file = appUtils.getCorrectPath(selectedFile);
					}
				}

				TPButton {
					id: btnOpenMesoFile
					imageSource: txtMesoFile.text.indexOf("pdf") !== -1 ? "pdf-icon" : "doc-icon"
					visible: appUtils.canReadFile(mesoManager.file)
					Layout.leftMargin: -10

					onClicked: osInterface.viewExternalFile(mesoManager.file);
				}
			}

			TPLabel {
				text: mesoManager.startDateLabel

				TPButton {
					imageSource: "set-completed"
					checkable: true
					fixedSize: true
					visible: mesoManager.isNewMeso
					height: 25
					width: 25

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onCheck: {
						btnStartDate.enabled = !checked;
						if (checked) {
							mesoManager.acceptStartDate();
							lblNewMesoRequiredFieldsCounter.decreaseCounter();
						}
						else {
							caldlg.open();
							lblNewMesoRequiredFieldsCounter.increaseCounter();
						}
					}
				}
			}

			TPTextInput {
				id: txtMesoStartDate
				text: mesoManager.strStartDate
				Layout.fillWidth: false
				Layout.minimumWidth: parent.width/2
				readOnly: true

				CalendarDialog {
					id: caldlg
					showDate: mesoManager.startDate
					initDate: mesoManager.minimumMesoStartDate
					finalDate: mesoManager.maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: (date) => mesoManager.startDate = date;
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
				checked: mesoManager.realMeso
				Layout.fillWidth: true

				onPressAndHold: ToolTip.show(qsTr("A Mesocycle is a short-term plan, with defined starting and ending points and a specific goal in sight"), 5000);
				onClicked: {
					mesoManager.realMeso = checked;
					if (checked) {
						mesoManager.acceptEndDate();
						lblNewMesoRequiredFieldsCounter.decreaseCounter();
					}
					else
						lblNewMesoRequiredFieldsCounter.increaseCounter();
				}
			}

			TPLabel {
				text: mesoManager.endDateLabel
				visible: mesoManager.realMeso

				TPButton {
					imageSource: "set-completed"
					checkable: true
					fixedSize: true
					visible: mesoManager.isNewMeso
					height: 25
					width: 25

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onCheck: {
						btnEndDate.enabled = !checked;
						if (checked) {
							mesoManager.acceptEndDate();
							lblNewMesoRequiredFieldsCounter.decreaseCounter();
						}
						else {
							caldlg2.open();
							lblNewMesoRequiredFieldsCounter.increaseCounter();
						}
					}
				}
			}

			TPTextInput {
				id: txtMesoEndDate
				text: mesoManager.strEndDate
				readOnly: true
				visible: mesoManager.realMeso
				Layout.fillWidth: false
				Layout.minimumWidth: parent.width/2

				CalendarDialog {
					id: caldlg2
					showDate: mesoManager.endDate
					initDate: mesoManager.minimumMesoStartDate
					finalDate: mesoManager.maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: (date) => {
						mesoManager.endDate = date;
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

			TPLabel {
				id: lblnWeeks
				text: mesoManager.weeksLabel
				visible: mesoManager.realMeso
			}

			TPTextInput {
				id: txtMesoNWeeks
				text: mesoManager.weeks
				readOnly: true
				visible: mesoManager.realMeso
				Layout.alignment: Qt.AlignLeft
				Layout.maximumWidth: parent.width*0.2
				Layout.minimumWidth: parent.width*0.2
			}

			MesoSplitSetup {
				id: mesoSplitSetup
				width: parent.width
				Layout.minimumWidth: width
				Layout.maximumWidth: width
			}

			TPLabel {
				text: mesoManager.notesLabel
				Layout.topMargin: 10
			}
			Flickable {
				height: Math.min(contentHeight, 60)
				width: parent.width
				contentHeight: txtMesoNotes.implicitHeight
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				Layout.minimumHeight: 60

				TextArea.flickable: TextArea {
					id: txtMesoNotes
					text: mesoManager.notes
					color: appSettings.fontColor

					background: Rectangle {
						color: appSettings.primaryColor
						opacity: 0.8
						radius: 6
						border.color: appSettings.fontColor
					}

					onEditingFinished: mesoManager.notes = text;
				}

				Component.onCompleted: vBar2.position = 0
				ScrollBar.vertical: ScrollBar { id: vBar2 }
				ScrollBar.horizontal: ScrollBar {}
			}
		} //ColumnLayout
	} //ScrollView

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
		mesoManager.changeMesoCalendar(calendarChangeDlg.customBoolProperty1, calendarChangeDlg.customBoolProperty2);
	}

	function updateCoachesAndClientsModels(use_mode: int) {
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
} //Page
