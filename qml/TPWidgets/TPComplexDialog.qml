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
	property string button1Text: qsTr("Yes")
	property string button2Text: qsTr("No")
	property string backColor: appSettings.primaryColor
	property string textColor: appSettings.fontColor

	property Item customItem: null
	property bool customBoolProperty1
	property bool customBoolProperty2
	property bool customBoolProperty3
	property int customIntProperty1
	readonly property int customItemTopPos: lblTitle.height + 10
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
				customItem = component.createObject(customItemLayoutManager, { parentDlg: dialog, "Layout.fillWidth": true });
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	TPLabel {
		id: lblTitle
		text: title
		horizontalAlignment: Text.AlignHCenter
		visible: title.length > 0

		anchors {
			top: parent.top
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: closeButtonVisible ? btnClose.left : parent.right
			rightMargin: 5
		}
	}

	ColumnLayout {
		id: customItemLayoutManager
		spacing: 0

		anchors {
			top: lblTitle.bottom
			topMargin: 5
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}
	}

	RowLayout {
		id: buttonsRow
		spacing: 0
		height: 30

		anchors {
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
			bottom: parent.bottom
		}

		TPButton {
			id: btn1
			text: button1Text
			flat: false
			autoSize: true
			visible: button1Text.length > 0
			Layout.alignment: Qt.AlignTop|Qt.AlignHCenter

			onClicked: {
				button1Clicked();
				dialog.closePopup();
			}
		}

		TPButton {
			id: btn2
			text: button2Text
			flat: false
			autoSize: true
			visible: button2Text.length > 0
			Layout.alignment: Qt.AlignTop|Qt.AlignHCenter
			Layout.maximumWidth: availableWidth - btn1.width - 10

			onClicked: {
				button2Clicked();
				dialog.closePopup();
			}
		}
	}

	function show(ypos: int): void {
		dialogOpened();
		let new_height = 0;
		if (title.length > 0)
			new_height = lblTitle.height;
		new_height += customItemLayoutManager.childrenRect.height + buttonsRow.height + 20;
		dialog.height = new_height;
		show1(ypos);
	}
}
