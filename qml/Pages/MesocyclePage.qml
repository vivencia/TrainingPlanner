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

//public:
	required property MesoManager mesoManager
	required property MesocyclesModel mesoModel

//private:
	property TPBalloonTip wrongFieldsPopup: requiredFieldsMissingLoader.item as TPBalloonTip

	Component.onCompleted: {
		if (!mesoManager.mesoOK)
			requiredFieldsMissingLoader.active = true;
	}

	Loader {
		id: requiredFieldsMissingLoader
		asynchronous: true
		active: false

		sourceComponent: TPBalloonTip {
			parentPage: mesoPage
			imageSource: "set-completed"
			show_position: Qt.AlignBottom|Qt.AlignHCenter
			title: mesoPage.mesoManager.wrongFieldsCounter > 0 ? qsTr("New program setup incomplete") : qsTr("New program setup complete!");
			message: mesoPage.wrongFieldsMessage()
			subImageLabel: mesoPage.mesoManager.wrongFieldsCounter > 0 ? String(mesoPage.mesoManager.wrongFieldsCounter) : "OK";
			button1Text: mesoPage.mesoManager.wrongFieldsCounter > 0 ? "" : qsTr("Close")
			button2Text: ""
			keepAbove: true
			movable: true
			onButton1Clicked: requiredFieldsMissingLoader.active = false;
		}

		onLoaded: mesoPage.wrongFieldsPopup.tpQmlOpen(mesoPage);
	}

	Connections {
		target: mesoPage.mesoManager
		function onWrongFieldsCounterChanged(): void {
			if (mesoPage.mesoManager.wrongFieldsCounter === 1) {
				if (!requiredFieldsMissingLoader.active) {
					requiredFieldsMissingLoader.active = true;
					return;
				}
			}
			mesoPage.wrongFieldsPopup.message = mesoPage.wrongFieldsMessage();
		}
	}

	function wrongFieldsMessage(): string {
		if (mesoManager.wrongFieldsCounter === 0)
			return qsTr("Required fields setup");
		else {
			let message = "";
			if (!mesoManager.mesoNameOK)
				message = qsTr("Change and/or accept the program's name");
			if (!mesoManager.startDateOK)
				message += '\n' +  qsTr("Change and/or accept the start date");
			if (!mesoManager.endDateOK)
				message += '\n' + qsTr("Change and/or accept the end date");
			if (!mesoManager.splitOK)
				message += '\n' + qsTr("Change and/or accept the split division");
			return message;
		}
	}

	TPScrollView {
		parentPage: mesoPage
		contentHeight: layoutMain.implicitHeight
		anchors.fill: parent

		ColumnLayout {
			id: layoutMain
			spacing: 10
			anchors {
				fill: parent
				leftMargin: 5
				rightMargin: 5
				topMargin: 0
				bottomMargin: 10
			}

			Loader {
				active: !mesoPage.mesoManager.ownMeso
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
						listClients: true
						listConfirmed: true
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
						visible: !mesoPage.mesoManager.mesoForClient
					}

					TPTextInput {
						id: txtCoachName
						text: mesoPage.mesoManager.coachName
						readOnly: true
						visible: !mesoPage.mesoManager.mesoForClient

						Layout.fillWidth: true
					}
				} //ColumnLayout: Loader sourceComponent
			} //Loader

			TPLabel {
				text: mesoPage.mesoModel.mesoNameLabel
				Layout.maximumWidth: parent.width / 2 - 10

				TPButton {
					id: btnNameOK
					imageSource: "set-completed"
					enabled: mesoPage.mesoManager.mesoNameOK
					width: text.length > 0 ? parent.width: AppSettings.itemDefaultHeight
					height: AppSettings.itemDefaultHeight

					property bool name_changed: false
					onName_changedChanged: {
						if (name_changed)
							text = qsTr("Set new name");
						else
							text = "";
					}
					onClicked: mesoPage.mesoManager.name = txtMesoName.text;

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

				onTextEdited: {
					mesoPage.mesoManager.checkMesoName(text);
					btnNameOK.name_changed = true;
				}
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
			}

			TPLabel {
				text: mesoPage.mesoModel.fileLabel
			}

			TPFileViewer {
				id: _meso_file_viewer
				fileOps: mesoPage.mesoManager.instructionsFileViewer()
				useBackground: true
				missingFileInfo: qsTr("No instructions file added")
				Layout.preferredWidth: minimumWidth
				Layout.preferredHeight: minimumHeight
				Layout.alignment: Qt.AlignCenter
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
				Layout.minimumWidth: 0.5 * parent.width

				Loader {
					id: startDateCalendarLoader
					asynchronous: true
					active: false

					property CalendarDialog _dlg

					sourceComponent: CalendarDialog {
						id: caldlg
						showDate: mesoPage.mesoManager.startDate
						initDate: mesoPage.mesoManager.minimumStartDate
						finalDate: AppUtils.createDate(0, 1, 0) //At most, allow to create a program in advance of one month from today
						parentPage: mesoPage
						onClosed: startDateCalendarLoader.active = false;
						onDateSelected: (date) => mesoPage.mesoManager.startDate = date;
						Component.onCompleted: startDateCalendarLoader._dlg = this;
					}

					onLoaded: _dlg.open();
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

					onClicked:startDateCalendarLoader.active = true;
				}
			}

			TPRadioButtonOrCheckBox {
				text: qsTr("Mesocycle-style program")
				boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
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

				Loader {
					id: endDateCalendarLoader
					asynchronous: true
					active: false

					property CalendarDialog _dlg

					sourceComponent: CalendarDialog {
						showDate: mesoPage.mesoManager.endDate
						initDate: mesoPage.mesoManager.minimumEndDate
						finalDate: mesoPage.mesoManager.maximumEndDate
						parentPage: mesoPage
						onClosed: endDateCalendarLoader.active = false;
						onDateSelected: (date) => {
							mesoPage.mesoManager.endDate = date;
							mesoSplitSetup.forcusOnFirstItem();
						}
						Component.onCompleted: endDateCalendarLoader._dlg = this;
					}

					onLoaded: _dlg.open();
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

					onClicked: endDateCalendarLoader.active = true;
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
				Layout.maximumWidth: parent.width * 0.7

				TPButton {
					id: btnNotesOK
					imageSource: "set-completed"
					enabled: notes_changed
					width: text.length > 0 ? parent.width * 0.3 : AppSettings.itemDefaultHeight
					height: AppSettings.itemDefaultHeight

					property bool notes_changed: false
					onNotes_changedChanged: {
						if (notes_changed)
							text = qsTr("Save");
						else
							text = "";
					}
					onClicked: mesoPage.saveNotes();

					anchors {
						left: parent.right
						verticalCenter: parent.verticalCenter
					}
				}
			}

			TPMultiLineEdit {
				id: txtNotes
				Layout.fillWidth: true
				Layout.preferredHeight: minHeight
				text: mesoPage.mesoManager.notes
				minHeight: AppSettings.itemDefaultHeight * 5
				onTextEdited: if (!btnNotesOK.notes_changed) btnNotesOK.notes_changed = true;
				onEditingFinished: (_text) => mesoPage.saveNotes();
			}
		} //ColumnLayout
	} //ScrollView

	function saveNotes(): void {
		if (btnNotesOK.notes_changed) {
			btnNotesOK.notes_changed = false;
			mesoPage.mesoManager.notes = txtNotes.contentsText();
		}
	}
} //Page
