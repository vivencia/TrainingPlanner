pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.Dialogs
import TpQml.User

import "./MesocyclePageElements"

TPPage {
	id: mesoPage
	imageSource: ":/images/backgrounds/backimage-meso.jpg"
	backgroundOpacity: 0.6
	objectName: "mesoPage"

	required property MesoManager mesoManager
	required property MesocyclesModel mesoModel
	property TPBalloonTip missingFieldsTip: requiredFieldsMissingLoader.item as TPBalloonTip

	Loader {
		id: requiredFieldsMissingLoader
		active: false
		asynchronous: true

		property int start_field: -1

		sourceComponent: TPBalloonTip {
			parentPage: mesoPage
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
					missingFieldsTip.showInWindow(-Qt.AlignBottom|Qt.AlignHCenter);
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
			missingFieldsTip.showTimed(5000, Qt.AlignBottom|Qt.AlignHCenter);
			break;
		case 8:
			missingFieldsTip.subImageLabel = "?";
			missingFieldsTip.title = qsTr("Accept program from coach?");
			missingFieldsTip.message = qsTr("Until you accept this program you can only view it");
			missingFieldsTip.button1Text = qsTr("Yes");
			missingFieldsTip.button2Text = qsTr("No");
			missingFieldsTip.button1Clicked.connect(function() { mesoPage.mesoManager.incorporateMeso(); });
			break;
		default:
			missingFieldsTip.title = qsTr("New program setup incomplete");
			missingFieldsTip.subImageLabel = String(wrong_field_counter);
			switch (field) {
			case MesocyclesModel.MESO_FIELD_NAME: missingFieldsTip.message = qsTr("Change and/or accept the program's name"); break; //MESO_FIELD_NAME
			case MesocyclesModel.MESO_FIELD_STARTDATE: missingFieldsTip.message = qsTr("Change and/or accept the start date"); break; //MESO_FIELD_STARTDATE
			case MesocyclesModel.MESO_FIELD_ENDDATE: missingFieldsTip.message = qsTr("Change and/or accept the end date"); break; //MESO_FIELD_ENDDATE
			case MesocyclesModel.MESO_FIELD_SPLIT: missingFieldsTip.message = qsTr("Change and/or accept the split division"); break; //MESO_FIELD_SPLIT
			}
			break;
		}
	}

	TPScrollView {
		parentPage: mesoPage
		navButtonsVisible: true
		contentHeight: layoutMain.implicitHeight
		anchors.fill: mesoPage

		ColumnLayout {
			id: layoutMain
			spacing: 10
			anchors {
				fill: parent
				leftMargin: 5
				rightMargin: 5
				topMargin: 0
				bottomMargin: requiredFieldsMissingLoader.active ? mesoPage.missingFieldsTip.height + 20 : 10
			}

			Loader {
				active: !mesoPage.mesoManager.ownMeso && AppUserModel.currentClients.count > 0
				asynchronous: true
				Layout.fillWidth: true

				sourceComponent: ColumnLayout {
					id: loaderLayout
					spacing: 10

					TPLabel {
						id: lblClient
						text: mesoPage.mesoModel.clientLabel
					}

					TPCoachesAndClientsList {
						id: clientsList
						currentRow: AppUserModel.findUserById(mesoPage.mesoManager.client)
						buttonString: qsTr("Go to client's page")
						Layout.preferredHeight: 0.2 * mesoPage.height
						Layout.fillWidth: true
						Layout.minimumHeight: height

						onItemSelected: (userRow) => mesoPage.mesoManager.client = AppUserModel.userId_QML(userRow);
						onButtonClicked: ItemManager.getClientsPage();
					} //TPCoachesAndClientsList

					TPLabel {
						id: lblCoachName
						text: mesoPage.mesoModel.coachLabel
						visible: !mesoPage.mesoManager.coachIsMainUser
					}

					TPTextInput {
						id: txtCoachName
						text: mesoPage.mesoManager.coachName
						readOnly: true
						visible: !mesoPage.mesoManager.coachIsMainUser

						Layout.fillWidth: true
					}
				} //ColumnLayout: Loader sourceComponent
			} //Loader

			TPLabel {
				text: mesoPage.mesoModel.mesoNameLabel
				Layout.topMargin: 10

				TPImage {
					source: "set-completed"
					enabled: mesoPage.mesoManager.mesoNameOK
					width: AppSettings.itemDefaultHeight
					height: width

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}
				}
			}

			TPTextInput {
				id: txtMesoName
				text: mesoPage.mesoManager.name
				ToolTip.text: mesoPage.mesoManager.mesoNameErrorTooltip
				ToolTip.visible: !mesoPage.mesoManager.mesoNameOK
				Layout.fillWidth: true

				onEditingFinished: mesoPage.mesoManager.name = text;
				onEnterOrReturnKeyPressed: cboMesoType.forceActiveFocus();
			}

			TPLabel {
				text: mesoPage.mesoModel.typeLabel
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
						mesoPage.mesoManager.type = textAt(index);
					else
						txtMesoTypeOther.forceActiveFocus();
					currentIndex = index;
				}

				Component.onCompleted: {
					let cboidx = find(mesoPage.mesoManager.type);
					if (cboidx === -1)
						cboidx = typeModel.count - 1;
					currentIndex = cboidx;
				}
			}

			TPTextInput {
				id: txtMesoTypeOther
				text: mesoPage.mesoManager.type
				showClearTextButton: !readOnly
				visible: cboMesoType.currentIndex === typeModel.count - 1
				Layout.fillWidth: true

				onEditingFinished: mesoPage.mesoManager.type = text;
				onEnterOrReturnKeyPressed: txtMesoFile.forceActiveFocus();
			}


			TPLabel {
				text: mesoPage.mesoModel.fileLabel
			}

			TPTextInput {
				id: txtMesoFile
				text: mesoPage.mesoManager.displayFileName
				readOnly: true
				showClearTextButton: true
				ToolTip.text: mesoPage.mesoManager.fileName
				Layout.minimumWidth: 0.8 * parent.width
				Layout.maximumWidth: 0.8 * parent.width

				onTextCleared: mesoPage.mesoManager.fileName = "";

				TPButton {
					id: btnChooseMesoFile
					imageSource: "choose-file"
					width: AppSettings.itemDefaultHeight
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

						onAccepted: mesoPage.mesoManager.fileName = AppUtils.getCorrectPath(selectedFile);
					}
				}

				TPButton {
					id: btnOpenMesoFile
					imageSource: txtMesoFile.text.indexOf("pdf") !== -1 ? "pdf-icon" : "doc-icon"
					visible: AppUtils.canReadFile(mesoPage.mesoManager.fileName)

					anchors {
						left: btnChooseMesoFile.right
						verticalCenter: parent.verticalCenter
					}

					onClicked: AppOsInterface.viewExternalFile(mesoPage.mesoManager.fileName);
				}
			}

			TPLabel {
				text: mesoPage.mesoModel.startDateLabel

				TPImage {
					source: "set-completed"
					enabled: mesoPage.mesoManager.startDateOK
					width: AppSettings.itemDefaultHeight
					height: width

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}
				}
			}

			TPTextInput {
				id: txtMesoStartDate
				text: mesoPage.mesoManager.strStartDate
				readOnly: true
				Layout.fillWidth: false
				Layout.minimumWidth: 0.5*parent.width

				CalendarDialog {
					id: caldlg
					showDate: mesoPage.mesoManager.startDate
					initDate: mesoPage.mesoManager.minimumMesoStartDate
					finalDate: mesoPage.mesoManager.maximumMesoEndDate
					parentPage: mesoPage

					onDateSelected: (date) => mesoPage.mesoManager.startDate = date;
				}

				TPButton {
					id: btnStartDate
					imageSource: "calendar.png"
					width: AppSettings.itemDefaultHeight
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
				checked: mesoPage.mesoManager.realMeso
				Layout.preferredWidth: 0.9 * parent.width
				Layout.topMargin: 15
				Layout.bottomMargin: 15

				onClicked: mesoPage.mesoManager.realMeso = checked;

				TPButton {
					imageSource: "question.png"
					width: AppSettings.itemDefaultHeight
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
				text: mesoPage.mesoModel.endDateLabel
				visible: mesoPage.mesoManager.realMeso

				TPImage {
					source: "set-completed"
					enabled: mesoPage.mesoManager.endDateOK
					width: AppSettings.itemDefaultHeight
					height: width

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}
				}
			}

			TPTextInput {
				id: txtMesoEndDate
				text: mesoPage.mesoManager.strEndDate
				readOnly: true
				visible: mesoPage.mesoManager.realMeso
				Layout.fillWidth: false
				Layout.minimumWidth: 0.5 * parent.width

				CalendarDialog {
					id: caldlg2
					showDate: mesoPage.mesoManager.endDate
					initDate: mesoPage.mesoManager.minimumMesoStartDate
					finalDate: mesoPage.mesoManager.maximumMesoEndDate
					parentPage: mesoPage

					onDateSelected: (date) => {
						mesoPage.mesoManager.endDate = date;
						mesoSplitSetup.forcusOnFirstItem();
					}
				}

				TPButton {
					id: btnEndDate
					imageSource: "calendar.png"
					width: AppSettings.itemDefaultHeight
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
				text: mesoPage.mesoModel.nWeeksLabel
				visible: mesoPage.mesoManager.realMeso
			}

			TPTextInput {
				id: txtMesoNWeeks
				text: mesoPage.mesoManager.weeks
				readOnly: true
				visible: mesoPage.mesoManager.realMeso
				Layout.preferredWidth: 0.2 * parent.width
			}

			MesoSplitSetup {
				id: mesoSplitSetup
				mesoManager: mesoPage.mesoManager
				mesoModel: mesoPage.mesoModel
				Layout.fillWidth: true
				Layout.topMargin: 10
			}

			TPLabel {
				text: mesoPage.mesoModel.notesLabel
				Layout.topMargin: 10
				Layout.fillWidth: true
			}

			TPMultiLineEdit {
				Layout.fillWidth: true
				Layout.preferredHeight: AppSettings.pageHeight * 0.15
				text: mesoPage.mesoManager.notes
				onTextAltered: (_text) => mesoPage.mesoManager.notes = _text;
			}

			TPButton {
				text: qsTr("Send to client")
				autoSize: true
				visible: !mesoPage.mesoManager.ownMeso
				enabled: mesoPage.mesoManager.canExport
				Layout.alignment: Qt.AlignCenter

				onClicked: mesoPage.mesoManager.sendMesocycleFileToClient();
			}
		} //ColumnLayout
	} //ScrollView
} //Page
