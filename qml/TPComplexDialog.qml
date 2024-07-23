import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Popup {
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string backColor: AppSettings.primaryColor
	property string textColor: AppSettings.fontColor
	property int startYPosition: 0

	property int finalXPos: 0
	property int finalYPos: 0
	property int startYPos: 0

	property string customItemSource: ""
	property var customItem: null
	property bool customBoolProperty1
	property bool customBoolProperty2
	property bool customBoolProperty3
	property int customIntProperty1
	property string customStringProperty1
	property var customModel: []
	property bool bAdjustHeightEveryOpen: false

	signal button1Clicked();
	signal button2Clicked();
	signal dialogOpened();

	id: dialog
	closePolicy: Popup.NoAutoClose
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	width: windowWidth * 0.9
	height: totalHeight + 10

	property int totalHeight: 0

	Rectangle {
		id: backRec
		anchors.fill: parent
		color: backColor
		radius: 8
		layer.enabled: true
		visible: true
	}

	background: backgroundEffect

	MultiEffect {
		id: backgroundEffect
		visible: true
		source: backRec
		anchors.fill: backRec
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		opacity: 0.9
	}

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

	enter: Transition {
		NumberAnimation {
			property: "y"
			from: startYPos
			to: finalYPos
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			property: "opacity"
			from: 0
			to: 0.9
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	exit: Transition {
		id: closeTransition
		NumberAnimation {
			property: "y"
			from: finalYPos
			to: startYPos
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			property: "opacity"
			from: 0.9
			to: 0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	onCustomItemSourceChanged: {
		if (customItemSource.length > 0) {
			var component = Qt.createComponent(customItemSource, Qt.Asynchronous);

			function finishCreation() {
				customItem = component.createObject(mainLayout, { parentDlg: dialog, "Layout.row": 1,  "Layout.column": 0, "Layout.columnSpan": 2,
						"Layout.leftMargin": 5, "Layout.rightMargin": 5, "Layout.fillWidth": true, bHasMesoPlan: customBoolProperty1,
						bHasPreviousTDays: customBoolProperty2, noExercises: customBoolProperty3, z:2 });
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

		Label {
			id: lblTitle
			text: title
			color: textColor
			elide: Text.ElideRight
			horizontalAlignment: Text.AlignHCenter
			font.pointSize: AppSettings.fontSize
			font.weight: Font.Black
			visible: title.length > 0
			height: 30
			padding: 0
			Layout.row: 0
			Layout.column: 0
			Layout.columnSpan: 2
			Layout.leftMargin: 5
			Layout.topMargin: 10
			Layout.rightMargin: 5
			Layout.minimumWidth: mainLayout.width - 10
			Layout.maximumWidth: mainLayout.width - 10

			Component.onCompleted: totalHeight += height;
		}

		TPButton {
			id: btn1
			text: button1Text
			visible: button1Text.length > 0
			z: 2
			Layout.row: 2
			Layout.column: 0
			Layout.columnSpan: button2Text.length > 0 ? 1 : 2
			Layout.rightMargin: 5
			Layout.bottomMargin: 10
			Layout.topMargin: -15
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
			visible: button2Text.length > 0
			z: 2
			Layout.row: 2
			Layout.column: button1Text.length > 0 ? 1 : 0
			Layout.leftMargin: 5
			Layout.bottomMargin: 10
			Layout.topMargin: -15
			Layout.alignment: Qt.AlignLeft

			onClicked: {
				button2Clicked();
				dialog.close();
			}

			Component.onCompleted: totalHeight += height;
		}
	} //GridLayout

	function show(ypos) {
		dialog.x = (windowWidth - dialog.width)/2;

		if (ypos < 0)
			ypos = (windowHeight-dialog.height)/2;

		dialog.y = finalYPos = ypos;
		if (ypos <= windowHeight/2)
			startYPos = -300;
		else
			startYPos = windowHeight + 300;
		if (bAdjustHeightEveryOpen) {
			dialogOpened();
			height += customItem.height;
		}
		dialog.open();
	}
}
