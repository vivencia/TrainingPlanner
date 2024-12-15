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

	header: TPToolBar {
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
				visible: mesoManager.hasCoach
				implicitWidth: parent.width*0.8

				model: ListModel {
					id: coachesModel

					function populate(): void {
						const currentCoach = userModel.userName(userModel.currentCoach(userModel.userRow(mesoManager.coach)));
						append({ "text": currentCoach, "value": 0, "enabled": true });
						const coaches = userModel.getCoaches();
						for(let i = 0; i < coaches.length; ++i) {
							if (coaches[i] !== currentCoach)
								append({ "text": coaches[i], "value": i, "enabled": true });
							if (coaches[i] === mesoManager.coach)
								cboCoaches.currentIndex = i;
						}
					}
				}

				onActivated: (index) => mesoManager.coach = textAt(index);

				TPButton {
					imageSource: "manage-coaches"

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: itemManager.getClientsOrCoachesPage(false, true);
				}
			}

			TPCheckBox {
				text: qsTr("This plan is for myself")
				checked: mesoManager.ownMeso
				visible: mesoManager.ownerIsCoach
				Layout.fillWidth: true

				onClicked: mesoManager.ownMeso = checked;
			}

			Row {
				visible: mesoManager.ownerIsCoach && !mesoManager.ownMeso
				spacing: 0
				Layout.fillWidth: true

				TPLabel {
					text: mesoManager.clientLabel
					width: 0.20*parent.width
				}

				TPComboBox {
					id: cboClients
					implicitWidth: 0.7*parent.width
					Layout.minimumWidth: width

					model: ListModel {
						id: clientsModel

						function populate(): void {
							const currentClient = userModel.userName(userModel.currentClient());
							append({ "text": currentClient, "value": 0, "enabled": true });
							const clients = userModel.getClients();
							for(let i = 0; i < clients.length; ++i) {
								if (clients[i] !== currentClient)
									append({ "text": clients[i], "value": i, "enabled": true });
								if (clients[i] === mesoManager.client)
									cboClients.currentIndex = i;
							}
						}
					}

					onActivated: (index) => mesoManager.client = textAt(index);
				}

				TPButton {
					id: btnManageClients
					imageSource: "manage-clients"

					onClicked: itemManager.getClientsOrCoachesPage(true, false);
				}
			}

			Row {
				spacing: 5
				Layout.fillWidth: true

				TPLabel {
					text: mesoManager.typeLabel
					width: 0.2*parent.width
				}

				TPComboBox {
					id: cboMesoType
					width: 0.75*parent.width
					Layout.minimumWidth: width

					model: ListModel {
						id: typeModel
						ListElement { text: qsTr("Weigth Loss"); value: 0; enabled: true; }
						ListElement { text: qsTr("Muscle Gain"); value: 1; enabled: true; }
						ListElement { text: qsTr("Bulking"); value: 2; enabled: true; }
						ListElement { text: qsTr("Pre-contest"); value: 3; enabled: true; }
						ListElement { text: qsTr("Strength Build-up"); value: 4; enabled: true; }
						ListElement { text: qsTr("Physical Recovery"); value: 5; enabled: true; }
						ListElement { text: qsTr("Physical Maintenance"); value: 6; enabled: true; }
						ListElement { text: qsTr("Other"); value: 7; enabled: true; }

						Component.onCompleted: {
							for (let i = 0; i < count; ++i) {
								if (get(i) === mesoManager.type)
								{
									cboMesoType.currentIndex = i;
									return;
								}
							}
						}
					}

					onActivated: (index) => {
						if (index < (typeModel.count - 1))
							mesoManager.type = textAt(index);
						else
							txtMesoTypeOther.forceActiveFocus();
					}
				}
			}

			TPTextInput {
				id: txtMesoTypeOther
				text: mesoManager.type
				visible: cboMesoType.currentIndex === typeModel.count - 1
				width: parent.width
				Layout.maximumWidth: width

				onEditingFinished: mesoManager.type = text;
				onEnterOrReturnKeyPressed: txtMesoFile.forceActiveFocus();
			}


			TPLabel {
				text: qsTr("Instructions file")
			}

			TPTextInput {
				id: txtMesoFile
				text: mesoManager.fileName
				readOnly: true
				ToolTip.text: mesoManager.fileName
				width: 0.8*parent.width
				Layout.maximumWidth: width
				Layout.minimumWidth: width

				TPButton {
					id: btnChooseMesoFile
					imageSource: "choose-file"

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

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

					anchors {
						left: btnChooseMesoFile.right
						verticalCenter: parent.verticalCenter
					}

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
				readOnly: true
				Layout.fillWidth: false
				Layout.minimumWidth: parent.width/2

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

			ScrollView {
				ScrollBar.horizontal.policy: ScrollBar.AsNeeded
				ScrollBar.vertical.policy: ScrollBar.AsNeeded
				contentWidth: availableWidth
				height: appSettings.pageHeight*0.15
				Layout.fillWidth: true
				Layout.preferredHeight: height

				TextArea {
					id: txtMesoNotes
					text: mesoManager.notes
					color: appSettings.fontColor
					textMargin: 0
					height: 50

					onEditingFinished: mesoManager.notes = text;
					onActiveFocusChanged: cursorPosition = activeFocus ? length : 0;
					onTextChanged: cursorPosition = 0;
				}
			}
		} //ColumnLayout
	} //ScrollView

	property bool alreadyCalled: false
	property TPComplexDialog calendarChangeDlg: null
	function showCalendarChangedDialog(): void {
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

	function changeCalendar(): void {
		mesoManager.changeMesoCalendar(calendarChangeDlg.customBoolProperty1, calendarChangeDlg.customBoolProperty2);
	}

	function updateCoachesAndClientsModels(use_mode: int): void {
		switch (use_mode) {
			case -1:
				coachesModel.populate();
				clientsModel.populate();
			break;
			case 0:
				coachesModel.clear();
				coachesModel.populate();
			break;
			case 1:
				clientsModel.clear();
				clientsModel.populate();
			break;
		}
	}
} //Page
