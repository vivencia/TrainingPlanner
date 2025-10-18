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

	onPageDeActivated: mesoManager.sendMesocycleFileToServer();

	Connections {
		target: mesoManager
		function onNewMesoFieldCounterChanged(next_field: int): void {
			if (!newMesoLoader.active) {
				newMesoLoader.start_field = next_field;
				newMesoLoader.active = true;
			}
			else
				newMesoMessageHandler(next_field);
		}
	}
	Connections {
		target: userModel
		function onCoachesNamesChanged() {
			coachesModel.clear();
			coachesModel.populate();
		}
	}

	Loader {
		id: newMesoLoader
		active: false
		asynchronous: true

		property int start_field: -1

		sourceComponent: TPBalloonTip {
			parentPage: mesoPropertiesPage
			imageSource: "set-completed"
			button1Text: ""
			button2Text: ""
			keepAbove: true
			movable: true

			onClosed: {
				if (mesoManager.newMesoFieldCounter === 0) {
					mesoManager.newMesoFieldCounter = -1;
					newMesoLoader.active = false;
				}
			}
		}

		onLoaded: {
			newMesoMessageHandler(start_field);
			start_field = -1;
			item.show(-2);
		}
	}

	function newMesoMessageHandler(next_field: int): void {
		switch (next_field) {
			case 0:
				newMesoTip.subImageLabel = "OK";
				newMesoTip.title = qsTr("New program setup complete!");
				newMesoTip.message = qsTr("Required fields setup");
				newMesoTip.showTimed(5000, -3);
			break;
			case 21:
				newMesoTip.subImageLabel = "?";
				newMesoTip.title = qsTr("Accept program from coach?");
				newMesoTip.message = qsTr("Until you accept this program you can only view it");
				newMesoTip.button1Text = qsTr("Yes");
				newMesoTip.button2Text = qsTr("No");
				newMesoTip.button1Clicked.connect(function() { mesoManager.incorporateMeso();} )
			break;
			default:
				newMesoTip.title = qsTr("New program setup incomplete");
				newMesoTip.subImageLabel = String(mesoManager.newMesoFieldCounter);
				switch (next_field) {
					case 1: newMesoTip.message = qsTr("Change and/or accept the program's name"); break; //MESOCYCLES_COL_NAME
					case 2: newMesoTip.message = qsTr("Change and/or accept the start date"); break; //MESOCYCLES_COL_STARTDATE
					case 3: newMesoTip.message = qsTr("Change and/or accept the end date"); break; //MESOCYCLES_COL_ENDDATE
					case 6: newMesoTip.message = qsTr("Change and/or accept the split division"); break; //MESOCYCLES_COL_SPLIT
				}
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
				active: !mesoManager.ownMeso && userModel.haveClients
				asynchronous: true
				Layout.fillWidth: true

				sourceComponent: ColumnLayout {
					spacing: 5

					TPLabel {
						id: lblClient
						text: itemManager.appMesocyclesModel.clientLabel
					}

					TPCoachesAndClientsList {
						id: clientsList
						currentRow: userModel.findUserByName(mesoManager.client)
						buttonString: qsTr("Go to client's page")
						height: 0.2 * mesoPropertiesPage.height
						allowNotConfirmed: false
						Layout.fillWidth: true
						Layout.preferredHeight: height

						onItemSelected: (userRow) => mesoManager.client = userModel.userId(userRow);
						onButtonClicked: itemManager.getClientsPage();
					} //TPCoachesAndClientsList

					TPLabel {
						id: lblCoachName
						text: itemManager.appMesocyclesModel.coachLabel
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
				text: itemManager.appMesocyclesModel.mesoNameLabel
				Layout.topMargin: 10

				TPButton {
					imageSource: "set-completed"
					visible: mesoManager.isNewMeso
					enabled: mesoManager.mesoNameOK
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: {
						if (mesoManager.isNewMeso)
							mesoManager.name = txtMesoName.text;
					}
				}
			}

			TPTextInput {
				id: txtMesoName
				text: mesoManager.name
				ToolTip.text: mesoManager.mesoNameErrorTooltip
				ToolTip.visible: !mesoManager.mesoNameOK
				Layout.preferredWidth: 0.9 * parent.width

				onEditingFinished: mesoManager.name = text;
				onEnterOrReturnKeyPressed: cboMesoType.forceActiveFocus();
			}

			TPLabel {
				text: itemManager.appMesocyclesModel.typeLabel
			}

			TPComboBox {
				id: cboMesoType
				width: 0.75 * parent.width
				Layout.fillWidth: true
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

				Component.onCompleted: {
					let cboidx = find(mesoManager.type);
					if (cboidx === -1)
						cboidx = typeModel.count - 1;
					currentIndex = cboidx;
				}
			}

			TPTextInput {
				id: txtMesoTypeOther
				text: mesoManager.type
				showClearTextButton: !readOnly
				visible: cboMesoType.currentIndex === typeModel.count - 1
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
				showClearTextButton: true
				ToolTip.text: mesoManager.fileName
				Layout.minimumWidth: 0.8 * parent.width
				Layout.maximumWidth: 0.8 * parent.width

				onTextCleared: mesoManager.fileName = "";

				TPButton {
					id: btnChooseMesoFile
					imageSource: "choose-file"
					width: appSettings.itemDefaultHeight
					height: width

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
				text: itemManager.appMesocyclesModel.startDateLabel

				TPButton {
					imageSource: "set-completed"
					visible: mesoManager.isNewMeso
					enabled: mesoManager.mesoNameOK
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: mesoManager.startDate = mesoManager.startDate;
				}
			}

			TPTextInput {
				id: txtMesoStartDate
				text: mesoManager.strStartDate
				readOnly: true
				Layout.fillWidth: false
				Layout.minimumWidth: 0.5*parent.width

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
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						left: txtMesoStartDate.right
						leftMargin: 10
						verticalCenter: txtMesoStartDate.verticalCenter
					}

					onClicked: caldlg.open();
				}
			}

			TPRadioButtonOrCheckBox {
				text: qsTr("Mesocycle-style program")
				radio: false
				checked: mesoManager.realMeso
				Layout.preferredWidth: 0.9 * parent.width
				Layout.topMargin: 15
				Layout.bottomMargin: 15

				onClicked: mesoManager.realMeso = checked;

				TPButton {
					imageSource: "question.png"
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						verticalCenter: parent.verticalCenter
						left: parent.right
						leftMargin: -15
					}

					onClicked: ToolTip.show(qsTr("A Mesocycle is a short-term program, with defined starting and ending points and a specific goal in sight"), 5000);
				}
			}

			TPLabel {
				text: itemManager.appMesocyclesModel.endDateLabel
				visible: mesoManager.realMeso

				TPButton {
					imageSource: "set-completed"
					visible: mesoManager.isNewMeso
					enabled: mesoManager.mesoNameOK
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: mesoManager.endDate = mesoManager.endDate;
				}
			}

			TPTextInput {
				id: txtMesoEndDate
				text: mesoManager.strEndDate
				readOnly: true
				visible: mesoManager.realMeso
				Layout.fillWidth: false
				Layout.minimumWidth: 0.5*parent.width

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
					width: appSettings.itemDefaultHeight
					height: width

					anchors {
						left: txtMesoEndDate.right
						leftMargin: 10
						verticalCenter: txtMesoEndDate.verticalCenter
					}

					onClicked: caldlg2.open();
				}
			}

			TPLabel {
				id: lblnWeeks
				text: itemManager.appMesocyclesModel.nWeeksLabel
				visible: mesoManager.realMeso
			}

			TPTextInput {
				id: txtMesoNWeeks
				text: mesoManager.weeks
				readOnly: true
				visible: mesoManager.realMeso
				Layout.preferredWidth: 0.2*parent.width
			}

			MesoSplitSetup {
				id: mesoSplitSetup
				Layout.fillWidth: true
				Layout.topMargin: 10
			}

			TPLabel {
				text: itemManager.appMesocyclesModel.notesLabel
				Layout.topMargin: 10
			}

			ScrollView {
				contentWidth: availableWidth
				height: appSettings.pageHeight * 0.15
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
				visible: !mesoManager.ownMeso
				enabled: mesoManager.canExport
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoManager.sendMesocycleFileToServer();
			}
		} //ColumnLayout
	} //ScrollView

	property int calDialogAnswer: 0

	Loader {
		id: changeCalendarLoader
		asynchronous: true
		active: false

		sourceComponent: TPComplexDialog {
			title: qsTr("Adjust meso calendar?")
			parentPage: mesoPropertiesPage
			customItemSource: "TPAdjustMesoCalendarFrame.qml"

			onButton1Clicked: {
				calDialogAnswer = customBoolProperty1 ? 1 : 2;
				changeCalendar();
				changeCalendarLoader.active = false;
			}
			onButton2Clicked: {
				mesoManager.doNotChangeMesoCalendar();
				calDialogAnswer = 0; //A "No", warrants a possible new confirmation
				changeCalendarLoader.active = false;
			}
		}

		onLoaded: item.show(-1);
	}

	function showCalendarChangedDialog(): void {
		if (calDialogAnswer > 0)
			changeCalendar();
		else
			changeCalendarLoader.active = true;
	}

	function changeCalendar(): void {
		mesoManager.changeMesoCalendar(calDialogAnswer === 1);
	}
} //Page
