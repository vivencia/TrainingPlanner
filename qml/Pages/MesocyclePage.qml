import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../"
import "../Dialogs"
import "../TPWidgets"
import "../User"
import "./MesocyclePageElements"

TPPage {
	id: mesoPropertiesPage
	imageSource: ":/images/backgrounds/backimage-meso.jpg"
	backgroundOpacity: 0.6
	objectName: "mesoPage"

	required property MesoManager mesoManager
	required property MesocyclesModel mesoModel
	property TPBalloonTip missingFieldsTip: requiredFieldsMissingLoader.item

	Loader {
		id: requiredFieldsMissingLoader
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

			onClosed: requiredFieldsMissingLoader.active = false;
		}
	}

	function wrongFieldValueMessageHandler(wrong_field_counter: int, field: int): void {
		if (!requiredFieldsMissingLoader.active)
		{
			if (wrong_field_counter === 0)
				return;
			else if (wrong_field_counter >= 1) {
				requiredFieldsMissingLoader.loaded.connect(function() {
					missingFieldsTip.show(-4);
					wrongFieldValueMessageHandler(wrong_field_counter, field);
				});
				requiredFieldsMissingLoader.active = true;
				return;
			}
		}

		switch (field) {
			case -1:
				missingFieldsTip.subImageLabel = "OK";
				missingFieldsTip.title = qsTr("New program setup complete!");
				missingFieldsTip.message = qsTr("Required fields setup");
				missingFieldsTip.showTimed(5000, -3);
			break;
			case 21:
				missingFieldsTip.subImageLabel = "?";
				missingFieldsTip.title = qsTr("Accept program from coach?");
				missingFieldsTip.message = qsTr("Until you accept this program you can only view it");
				missingFieldsTip.button1Text = qsTr("Yes");
				missingFieldsTip.button2Text = qsTr("No");
				missingFieldsTip.button1Clicked.connect(function() { mesoManager.incorporateMeso(); });
			break;
			default:
				missingFieldsTip.title = qsTr("New program setup incomplete");
				missingFieldsTip.subImageLabel = String(wrong_field_counter);
				switch (field) {
					case 1: missingFieldsTip.message = qsTr("Change and/or accept the program's name"); break; //MESO_FIELD_NAME
					case 2: missingFieldsTip.message = qsTr("Change and/or accept the start date"); break; //MESO_FIELD_STARTDATE
					case 3: missingFieldsTip.message = qsTr("Change and/or accept the end date"); break; //MESO_FIELD_ENDDATE
					case 6: missingFieldsTip.message = qsTr("Change and/or accept the split division"); break; //MESO_FIELD_SPLIT
				}
			break;
		}
	}

	ScrollView {
		id: scrollView
		contentWidth: availableWidth
		contentHeight: layoutMain.implicitHeight
		ScrollBar.vertical.interactive: Qt.platform.os !== "android"
		anchors.fill: parent

		ColumnLayout {
			id: layoutMain
			spacing: 10
			anchors {
				fill: parent
				leftMargin: 5
				rightMargin: 5
				topMargin: 0
				bottomMargin: requiredFieldsMissingLoader.active ? missingFieldsTip.height + 20 : 10
			}

			Loader {
				active: !mesoManager.ownMeso && userModel.currentClients.count > 0
				asynchronous: true
				Layout.fillWidth: true

				sourceComponent: ColumnLayout {
					id: loaderLayout
					spacing: 10

					TPLabel {
						id: lblClient
						text: mesoModel.clientLabel
					}

					TPCoachesAndClientsList {
						id: clientsList
						currentIndex: userModel.findUserById(mesoManager.client)
						buttonString: qsTr("Go to client's page")
						height: 0.2 * mesoPropertiesPage.height
						Layout.fillWidth: true
						Layout.minimumHeight: height

						onItemSelected: (userRow) => mesoManager.client = userModel.userId(userRow);
						onButtonClicked: itemManager.getClientsPage();
					} //TPCoachesAndClientsList

					TPLabel {
						id: lblCoachName
						text: mesoModel.coachLabel
						visible: !mesoManager.coachIsMainUser
					}

					TPTextInput {
						id: txtCoachName
						text: mesoManager.coachName
						readOnly: true
						visible: !mesoManager.coachIsMainUser

						Layout.fillWidth: true
					}
				} //ColumnLayout: Loader sourceComponent
			} //Loader

			TPLabel {
				text: mesoModel.mesoNameLabel
				Layout.topMargin: 10

				TPImage {
					source: "set-completed"
					enabled: mesoManager.mesoNameOK
					width: appSettings.itemDefaultHeight
					height: width

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
				Layout.fillWidth: true

				onEditingFinished: mesoManager.name = text;
				onEnterOrReturnKeyPressed: cboMesoType.forceActiveFocus();
			}

			TPLabel {
				text: mesoModel.typeLabel
			}

			TPComboBox {
				id: cboMesoType
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
					currentIndex = index;
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

					onClicked: fileDialog.show();

					TPFileDialog {
						id: fileDialog
						title: qsTr("Choose the instruction's file for this mesocycles")

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
				text: mesoModel.startDateLabel

				TPImage {
					source: "set-completed"
					enabled: mesoManager.startDateOK
					width: appSettings.itemDefaultHeight
					height: width

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
				text: mesoModel.endDateLabel
				visible: mesoManager.realMeso

				TPImage {
					source: "set-completed"
					enabled: mesoManager.endDateOK
					width: appSettings.itemDefaultHeight
					height: width

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
				text: mesoModel.nWeeksLabel
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
				text: mesoModel.notesLabel
				Layout.topMargin: 10
				Layout.fillWidth: true
			}

			TPMultiLineEdit {
				Layout.fillWidth: true
				Layout.preferredHeight: appSettings.pageHeight * 0.15
				text: mesoManager.notes
				onTextAltered: (_text) => mesoManager.notes = _text;
			}

			TPButton {
				text: qsTr("Send to client")
				autoSize: true
				visible: !mesoManager.ownMeso
				enabled: mesoManager.canExport
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoManager.sendMesocycleFileToClient();
			}
		} //ColumnLayout
	} //ScrollView
} //Page
