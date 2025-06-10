import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../Dialogs"
import "../TPWidgets"
import "./MesocyclePageElements"

TPPage {
	id: mesoPropertiesPage
	objectName: "mesoPage"

	required property MesoManager mesoManager

	property TPBalloonTip newMesoTip: newMesoLoader.item

	onPageDeActivated: {
		if (mesoManager.canSendMesoToServer())
			mesoManager.sendMesocycleFileToServer();
	}

	Connections {
		target: mesoManager
		function onNewMesoFieldCounterChanged(fieldCounter: int): void {
			if (newMesoTip)
				newMesoMessageHandler(fieldCounter);
		}
	}
	Connections {
		target: userModel
		function onCoachesNamesChanged() { coachesModel.clear(); coachesModel.populate(); }
	}

	Loader {
		id: newMesoLoader
		active: mesoManager.isTempMeso ? (mesoManager.isNewMeso ?
			mesoManager.newMesoFieldCounter >= 0 : mesoManager.newMesoFieldCounter === 0 || mesoManager.newMesoFieldCounter === 20) : false
		asynchronous: true

		sourceComponent: TPBalloonTip {
			parentPage: mesoPropertiesPage
			imageSource: "set-completed"
			button1Text: "OK"
			button2Text: ""
			keepAbove: true
			movable: true

			onClosed: {
				if (mesoManager.newMesoFieldCounter === 0)
					mesoManager.newMesoFieldCounter = -1;
			}
		}

		onLoaded: {
			if (mesoManager.newMesoFieldCounter !== 20)
				newMesoTip.subImageLabel = String(mesoManager.newMesoFieldCounter);
			newMesoMessageHandler(mesoManager.newMesoFieldCounter);
			item.show(-2);
		}
	}

	function newMesoMessageHandler(fieldCounter: int): void {
		switch (fieldCounter) {
			case 4:
				newMesoTip.title = qsTr("New program setup incomplete");
				newMesoTip.message = qsTr("Change and/or accept the program's name");
				break;
			case 3: newMesoTip.message = qsTr("Change and/or accept the start date"); break;
			case 2: newMesoTip.message = qsTr("Change and/or accept the end date"); break;
			case 1: newMesoTip.message = qsTr("Change and/or accept the split division"); break;
			case 0:
				newMesoTip.title = qsTr("New program setup complete!");
				newMesoTip.message = qsTr("Required fields setup");
				newMesoTip.showTimed(5000, -3);
			break;
			case 20:
				newMesoTip.title = qsTr("Accept program from coach?");
				newMesoTip.message = qsTr("Until you accept this program you can only view it");
				newMesoTip.button1Text = qsTr("Yes");
				newMesoTip.button2Text = qsTr("No");
				newMesoTip.button1Clicked.connect(function() { mesoManager.incorporateMeso();} )
			break;
		}
	}

	ScrollView {
		contentWidth: availableWidth
		ScrollBar.vertical.interactive: true
		ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
		ScrollBar.vertical.policy: ScrollBar.AlwaysOn

		anchors {
			fill: parent
			leftMargin: 5
			rightMargin: 5
			topMargin: 5
			bottomMargin: 5
		}

		ColumnLayout {
			id: colMain
			spacing: 5
			anchors.fill: parent

			Loader {
				active: !mesoManager.ownMeso
				asynchronous: true
				Layout.fillWidth: true

				sourceComponent: ColumnLayout {
					spacing: 5

					TPLabel {
						id: lblClient
						text: mesoManager.clientLabel
					}

					TPClientsList {
						id: clientsList
						clientRow: userModel.clientRow(mesoManager.client)
						buttonString: qsTr("Go to client's page")
						height: 0.2*mesoPropertiesPage.height
						allowNotConfirmedClients: false
						Layout.fillWidth: true
						Layout.preferredHeight: height

						onClientSelected: (userRow) => mesoManager.client = userModel.userId(userRow);
						onButtonClicked: itemManager.getClientsPage();
					} //TPClientsList

					TPLabel {
						id: lblCoachName
						text: mesoManager.coachLabel
					}

					TPTextInput {
						id: txtCoachName
						text: mesoManager.coach
						readOnly: true
						Layout.fillWidth: true
					}
				} //ColumnLayout: Loader sourceComponent
			} //Loader

			TPLabel {
				text: mesocyclesModel.mesoNameLabel
				Layout.topMargin: 10

				TPImage {
					source: "set-completed"
					visible: mesoManager.isNewMeso
					enabled: mesoManager.mesoNameOK
					height: 25
					width: 25

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}
				}
			}

			TPTextInput {
				id: txtMesoName
				text: mesoManager.name
				ToolTip.text: mesoManager.mesoNameErrorTooltip
				ToolTip.visible: !mesoManager.mesoNameOK
				width: 0.9*parent.width
				Layout.preferredWidth: width

				onTextEdited: mesoManager.name = text;
				onEnterOrReturnKeyPressed: cboMesoType.forceActiveFocus();
			}

			Row {
				spacing: 5
				Layout.fillWidth: true

				TPLabel {
					text: mesocyclesModel.typeLabel
					width: 0.2*parent.width
				}

				TPComboBox {
					id: cboMesoType
					width: 0.75*parent.width
					Layout.minimumWidth: width
					currentIndex: {
						let cboidx = find(mesoManager.type);
						if (cboidx === -1)
							cboidx = typeModel.count - 1;
						return cboidx;
					}
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
				Layout.fillWidth: true

				onEditingFinished: mesoManager.type = text;
				onEnterOrReturnKeyPressed: txtMesoFile.forceActiveFocus();
			}


			TPLabel {
				text: qsTr("Instructions file")
			}

			TPTextInput {
				id: txtMesoFile
				text: mesoManager.displayFileName
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
						leftMargin: 5
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

						onAccepted: mesoManager.fileName = appUtils.getCorrectPath(selectedFile);
					}
				}

				TPButton {
					id: btnOpenMesoFile
					imageSource: txtMesoFile.text.indexOf("pdf") !== -1 ? "pdf-icon" : "doc-icon"
					visible: appUtils.canReadFile(mesoManager.fileName)

					anchors {
						left: btnChooseMesoFile.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: osInterface.viewExternalFile(mesoManager.fileName);
				}
			}

			TPLabel {
				text: mesocyclesModel.startDateLabel

				TPImage {
					source: "set-completed"
					visible: mesoManager.isNewMeso
					enabled: mesoManager.startDateOK
					height: 25
					width: 25

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
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
				text: qsTr("Mesocycle-style program")
				checked: mesoManager.realMeso
				Layout.fillWidth: true

				onPressAndHold: ToolTip.show(qsTr("A Mesocycle is a short-term program, with defined starting and ending points and a specific goal in sight"), 5000);
				onClicked: {
					mesoManager.realMeso = checked;
					if (checked)
						mesoManager.acceptEndDate();
				}
			}

			TPLabel {
				text: mesocyclesModel.endDateLabel
				visible: mesoManager.realMeso

				TPImage {
					source: "set-completed"
					visible: mesoManager.isNewMeso
					enabled: mesoManager.endDateOK

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
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
				text: mesocyclesModel.nWeeksLabel
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
				Layout.fillWidth: true
				Layout.topMargin: 10
			}

			TPLabel {
				text: mesocyclesModel.notesLabel
				Layout.topMargin: 10
			}

			ScrollView {
				contentWidth: availableWidth
				height: appSettings.pageHeight*0.15
				Layout.fillWidth: true
				Layout.preferredHeight: height
				ScrollBar.horizontal.interactive: true
				ScrollBar.vertical.interactive: true
				ScrollBar.horizontal.policy: ScrollBar.AsNeeded
				ScrollBar.vertical.policy: ScrollBar.AsNeeded

				TextArea {
					id: txtMesoNotes
					text: mesoManager.notes
					color: "black"
					font.pixelSize: appSettings.fontSize
					font.bold: true
					topPadding: appSettings.fontSize
					leftPadding: 5
					rightPadding: 5
					bottomPadding: 5
					height: 50

					background: Rectangle {
						color: "white"
						radius: 6
						border.color: appSettings.fontColor
					}

					onEditingFinished: mesoManager.notes = text;
					onActiveFocusChanged: cursorPosition = activeFocus ? length : 0;
					onTextChanged: cursorPosition = 0;
				}
			}

			TPButton {
				text: qsTr("Send to client")
				flat: false
				visible: !mesoManager.ownMeso
				enabled: mesoManager.canExport
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoManager.sendMesocycleFileToServer();
			}
		} //ColumnLayout
	} //ScrollView

	property bool alreadyCalled: false
	property TPComplexDialog calendarChangeDlg: null
	function showCalendarChangedDialog(): void {
		if (!calendarChangeDlg) {
			let component = Qt.createComponent("qrc:/qml/TPWidgets/TPComplexDialog.qml", Qt.Asynchronous);

			function finishCreation() {
				calendarChangeDlg = component.createObject(mainwindow, { parentPage: mesoPropertiesPage, title:qsTr("Adjust meso calendar?"),
					customItemSource:"TPAdjustMesoCalendarFrame.qml" });
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
} //Page
