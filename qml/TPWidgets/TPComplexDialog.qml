import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	id: dialog
	modal: true
	width: appSettings.pageWidth * 0.9

	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string backColor: appSettings.primaryColor
	property string textColor: appSettings.fontColor

	property Item customItem: null
	property bool customBoolProperty1
	property bool customBoolProperty2
	property bool customBoolProperty3
	property int customIntProperty1
	property string customItemSource: ""
	property string customStringProperty1
	property string customStringProperty2
	property string customStringProperty3
	property list<string> customModel: []

	signal button1Clicked();
	signal button2Clicked();
	signal dialogOpened();

	onCustomItemSourceChanged: {
		if (customItemSource.length > 0) {
			let component = Qt.createComponent("qrc:/qml/TPWidgets/ComplexDialogModules/"+customItemSource, Qt.Asynchronous);

			function finishCreation() {
				customItem = component.createObject(mainLayout, { parentDlg: dialog, "Layout.row": 1,  "Layout.column": 0, "Layout.columnSpan": 2,
						"Layout.leftMargin": 5, "Layout.rightMargin": 5, "Layout.fillWidth": true });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	GridLayout {
		id: mainLayout
		rows: 3
		columns: 2
		columnSpacing: 5
		rowSpacing: 5
		anchors.fill: parent

		TPLabel {
			id: lblTitle
			text: title
			horizontalAlignment: Text.AlignHCenter
			visible: title.length > 0
			heightAvailable: 50
			height: visible ? _preferredHeight : 0
			Layout.row: 0
			Layout.column: 0
			Layout.columnSpan: 2
			Layout.leftMargin: 5
			Layout.topMargin: 5
			Layout.rightMargin: 5
			Layout.preferredWidth: mainLayout.width - 10
		}

		TPButton {
			id: btn1
			text: button1Text
			flat: false
			visible: button1Text.length > 0
			z: 2
			Layout.row: 2
			Layout.column: 0
			Layout.columnSpan: button2Text.length > 0 ? 1 : 2
			Layout.rightMargin: 5
			Layout.bottomMargin: 10
			Layout.topMargin: -10
			Layout.alignment: button2Text.length > 0 ? Qt.AlignRight : Qt.AlignCenter

			onClicked: {
				button1Clicked();
				dialog.closePopup();
			}
		}

		TPButton {
			id: btn2
			text: button2Text
			flat: false
			visible: button2Text.length > 0
			z: 2
			Layout.row: 2
			Layout.column: button1Text.length > 0 ? 1 : 0
			Layout.leftMargin: 5
			Layout.bottomMargin: 10
			Layout.topMargin: -10
			Layout.alignment: Qt.AlignLeft

			onClicked: {
				button2Clicked();
				dialog.closePopup();
			}
		}
	} //GridLayout

	function show(ypos: int): void {
		dialogOpened();
		height = lblTitle.height + customItem.height + 50;
		show1(ypos);
	}
}
