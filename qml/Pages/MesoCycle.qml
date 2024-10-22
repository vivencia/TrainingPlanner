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
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 20
			}

			onClicked: mesoManager.getCalendarPage();
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

			TPLabel {
				text: mesoManager.nameLabel
				Layout.alignment: Qt.AlignHCenter
				Layout.topMargin: 10
			}
			TPTextInput {
				id: txtMesoName
				text: mesoManager.name
				wrapMode: Text.WordWrap
				ToolTip.text: qsTr("Name too short")
				ToolTip.visible:!bMesoNameOK;
				width: parent.width - 20
				Layout.leftMargin: 10
				Layout.minimumWidth: width
				Layout.maximumWidth: width

				readonly property bool bMesoNameOK: text.length >= 5

				onEditingFinished: {
					if (bMesoNameOK)
						mesoManager.name = text;
				}

				onEnterOrReturnKeyPressed: {
					if (cboCoaches.visible)
						cboCoaches.forceActiveFocus();
					else
						cboClients.forceActiveFocus();
				}
			}

			RowLayout {
				visible: mesoManager.hasCoach
				height: 30
				spacing: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true

				TPLabel {
					id: lblCoaches
					text: mesoManager.coachLabel
					width: appSettings.pageWidth/2 - 10
				}

				TPComboBox {
					id: cboCoaches
					editText: mesoManager.coach
					implicitWidth: appSettings.pageWidth/2
					Layout.minimumWidth: width

					model: ListModel {
						id: coachesModel

						Component.onCompleted: {
							const coaches = userModel.getCoaches();
							for(var i = 0; i < coaches.length; ++i)
								append({ "text": coaches[i], "value": i, "enabled": true });
						}
					}

					onActivated: (index) => mesoManager.coach = textAt(index);
				}

				TPButton {
					imageSource: "manage-coaches"

					onClicked: appControl.openClientsOrCoachesPage(false, true);
				}
			}

			TPCheckBox {
				text: qsTr("This plan is for myself")
				checked: mesoManager.ownMeso
				visible: mesoManager.ownerIsCoach
				Layout.leftMargin: 5
				Layout.fillWidth: true

				onClicked: mesoManager.ownMeso = checked;
			}

			RowLayout {
				visible: mesoManager.ownerIsCoach && !mesoManager.ownMeso
				height: 30
				spacing: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true

				TPLabel {
					id: lblClients
					text: mesoManager.clientLabel
				}

				TPComboBox {
					id: cboClients
					editText: mesoManager.client
					implicitWidth: appSettings.pageWidth*0.6
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
				height: 30
				spacing: 5
				Layout.leftMargin: 5
				Layout.fillWidth: true

				TPLabel {
					text: mesoManager.typeLabel
					width: (parent.width - 20)*0.2
				}

				TPComboBox {
					id: cboMesoType
					model: mesoTypeModel
					editText: mesoManager.type
					width: (mesoPropertiesPage.width - 20)*0.75
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
				width: parent.width - 20
				visible: cboMesoType.currentIndex === 6
				Layout.minimumWidth: width
				Layout.maximumWidth: width
				Layout.leftMargin: 5

				onEditingFinished: mesoManager.type = text;
				onEnterOrReturnKeyPressed: txtMesoFile.forceActiveFocus();
			}


			TPLabel {
				text: qsTr("Instructions file")
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
			}

			RowLayout {
				height: 30
				spacing: 0
				Layout.leftMargin: 5
				Layout.fillWidth: true

				TPTextInput {
					id: txtMesoFile
					text: mesoManager.fileName
					readOnly: true
					width: (mesoPropertiesPage.width - 20)*0.8
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

						onAccepted: btnOpenMesoFile.visible = mesoManager.file = selectedFile;
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
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
			}

			TPTextInput {
				id: txtMesoStartDate
				text: mesoManager.startDate
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
				readOnly: true

				CalendarDialog {
					id: caldlg
					showDate: mesoManager.calendarStartDate
					initDate: mesoManager.minimumMesoStartDate
					finalDate: mesoManager.maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: (date) => mesoManager.minimumMesoStartDate = date;
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
				Layout.leftMargin: 5
				Layout.fillWidth: true

				onPressAndHold: ToolTip.show(qsTr("A Mesocycle is a short-term plan, with defined starting and ending points and a specific goal in sight"), 5000);

				onClicked: mesoManager.realMeso = checked;
			}

			TPLabel {
				text: mesoManager.endDateLabel
				visible: mesoManager.realMeso
				Layout.leftMargin: 5
			}
			TPTextInput {
				id: txtMesoEndDate
				text: mesoManager.endDate
				readOnly: true
				visible: mesoManager.realMeso
				Layout.fillWidth: false
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2

				CalendarDialog {
					id: caldlg2
					showDate: mesoManager.calendarStartDate
					initDate: mesoManager.minimumMesoStartDate
					finalDate: mesoManager.maximumMesoEndDate
					parentPage: mesoPropertiesPage

					onDateSelected: (date) => {
						mesoManager.maximumMesoEndDate = date;
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
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
			}

			TPTextInput {
				id: txtMesoNWeeks
				text: mesoManager.weeks
				readOnly: true
				visible: mesoManager.realMeso
				Layout.alignment: Qt.AlignLeft
				Layout.leftMargin: 5
				Layout.minimumWidth: parent.width / 2
			}

			MesoSplitSetup {
				id: mesoSplitSetup
				Layout.fillWidth: true
				Layout.leftMargin: 0
			}

			TPLabel {
				text: mesoManager.notesLabel
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

	function updateCoachesAndClientsModels(userrow: int) {
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
} //Page
