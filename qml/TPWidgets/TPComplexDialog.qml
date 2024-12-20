import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPopup {
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string backColor: appSettings.primaryColor
	property string textColor: appSettings.fontColor

	property int startYPosition: 0
	property int finalXPos: 0

	property string customItemSource: ""
	property var customItem: null
	property bool customBoolProperty1
	property bool customBoolProperty2
	property bool customBoolProperty3
	property int customIntProperty1
	property string customStringProperty1
	property string customStringProperty2
	property string customStringProperty3
	property var customModel: []
	property bool bAdjustHeightEveryOpen: false
	property int totalHeight: 0

	signal button1Clicked();
	signal button2Clicked();
	signal dialogOpened();

	id: dialog
	modal: true
	width: appSettings.pageWidth * 0.9
	height: totalHeight + 20

	NumberAnimation {
		id: alternateCloseTransition
		target: dialog
		alwaysRunToEnd: true
		running: false
		property: "x"
		from: x
		to: finalXPos
		duration: 500
		easing.type: Easing.InOutCubic
	}

	onCustomItemSourceChanged: {
		if (customItemSource.length > 0) {
			var component = Qt.createComponent("qrc:/qml/TPWidgets/ComplexDialogModules/"+customItemSource, Qt.Asynchronous);

			function finishCreation() {
				customItem = component.createObject(mainLayout, { parentDlg: dialog, "Layout.row": 1,  "Layout.column": 0, "Layout.columnSpan": 2,
						"Layout.leftMargin": 5, "Layout.rightMargin": 5, "Layout.fillWidth": true });
				totalHeight += customItem.height;
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
			height: visible ? _preferredHeight : 0
			Layout.row: 0
			Layout.column: 0
			Layout.columnSpan: 2
			Layout.leftMargin: 5
			Layout.topMargin: 20
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
				dialog.close();
			}

			Component.onCompleted: totalHeight += height;
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
				dialog.close();
			}

			Component.onCompleted: totalHeight += height;
		}
	} //GridLayout

	function show(ypos) {
		dialog.x = (appSettings.pageWidth - dialog.width)/2;

		if (ypos < 0)
			ypos = (appSettings.pageHeight-dialog.height)/2;

		finalYPos = ypos;
		if (ypos <= appSettings.pageHeight/2)
			startYPos = -300;
		else
			startYPos = appSettings.pageHeight + 300;
		if (bAdjustHeightEveryOpen) {
			dialogOpened();
			totalHeight = lblTitle.height + 2*btn1.height + customItem.height + 20;
		}
		dialog.open();
	}
}
