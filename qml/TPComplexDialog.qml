import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Popup {
	property string title: ""
	property string button1Text: ""
	property string button2Text: ""
	property string imageSource: ""
	property string backColor: AppSettings.primaryColor
	property string textColor: AppSettings.fontColor
	property int startYPosition: 0

	property int finalXPos: 0
	property int finalYPos: 0
	property int startYPos: 0

	property string customItemSource: ""
	property bool customProperty1
	property bool customProperty2

	signal button1Clicked();
	signal button2Clicked();

	id: dialog
	closePolicy: Popup.NoAutoClose
	modal: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	width: windowWidth * 0.9

	Rectangle {
		id: backRec
		anchors.fill: parent
		color: backColor
		radius: 8
		layer.enabled: true
		visible: false
	}

	background: backRec

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
			to: 1
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
			from: 1
			to: 0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	onCustomItemSourceChanged: {
		if (customItemSource.length > 0) {
			var component = Qt.createComponent(customItemSource, Qt.Asynchronous);

			function finishCreation() {
				var item;
				if (imageSource.length === 0) {
					item = component.createObject(mainLayout, { text: label, "Layout.row": 1,  "Layout.column": 0, "Layout.columnSpan": 2,
						"Layout.leftMargin": 5, "Layout.rightMargin": 5, "Layout.fillWidth": true });
				}
				else {
					item = component.createObject(mainLayout, { text: label, "Layout.row": 1,  "Layout.column": 1, "Layout.rightMargin": 5,
						"Layout.fillWidth": true });
				}
			}

			if (component.status === Component.Ready)
				finishCreation();
			else
				component.statusChanged.connect(finishCreation);
		}
	}

	GridLayout
	{
		id: mainLayout
		rows: 3
		columns: 2
		columnSpacing: 5
		rowSpacing: 5

		Label {
			id: lblTitle
			text: title
			color: textColor
			elide: Text.ElideRight
			font.pointSize: AppSettings.fontSize
			font.weight: Font.Black
			visible: title.length > 0
			height: 30
			padding: 0
			Layout.row: 0
			Layout.column: 0
			Layout.columnSpan: 2
			Layout.leftMargin: 5
			Layout.topMargin: 0
			Layout.rightMargin: 5
			Layout.fillWidth: true
		}

		Image {
			id: imgElement
			source: imageSource != "" ? "qrc:/images/"+AppSettings.iconFolder+imageSource : imageSource
			fillMode: Image.PreserveAspectFit
			asynchronous: true
			layer.enabled: true
			visible: false
			width: 50
			height: 50
		}

		MultiEffect {
			id: imgEffects
			visible: imageSource.length > 0
			source: imgElement
			anchors.fill: imgElement
			shadowEnabled: true
			shadowOpacity: 0.5
			blurMax: 16
			shadowBlur: 1
			shadowHorizontalOffset: 5
			shadowVerticalOffset: 5
			shadowColor: "black"
			shadowScale: 1
			Layout.row: 1
			Layout.column: 0
			Layout.leftMargin: 5
		}

		TPButton {
			id: btn1
			text: button1Text
			visible: button1Text.length > 0
			z: 2
			Layout.row: 2
			Layout.column: 0
			Layout.leftMargin: 5

			onClicked: {
				button1Clicked();
				dialog.close();
			}
		}

		TPButton {
			id: btn2
			text: button2Text
			visible: button2Text.length > 0
			z: 2
			Layout.row: 2
			Layout.column: button1Text.length > 0 ? 1 : 0
			Layout.leftMargin: 5
			Layout.rightMargin: 5

			onClicked: {
				button2Clicked();
				dialog.close();
			}
		}
	} //GridLayout

	MouseArea {
		id: mouseArea
		property var prevPos
		z: 1
		anchors.fill: parent

		onPressed: (mouse) => {
			prevPos = { x: mouse.x, y: mouse.y };
		}

		onPositionChanged: {
			const deltaX = mouseX - prevPos.x;
			if ( Math.abs(deltaX) >= 10) {
				x += deltaX;
				if (deltaX > 0)
					finalXPos = windowWidth + 300;
				else
					finalXPos = -300;
				closeTransition.enabled = false;
				alternateCloseTransition.start();
				dialog.close();
			}
			prevPos = { x: mouseX, y: mouseY };
		}
	}

	function show(ypos) {
		dialog.height = lblTitle.height + lblMessage.height +
							(button1Text.length > 0 ? 2*btn1.buttonHeight : (button2Text.length > 0 ? 2*btn1.buttonHeight : 10)) +
							(checkBoxText.length > 0 ? chkBox.height : 0);
		dialog.x = (windowWidth - width)/2;

		if (ypos < 0)
			ypos = (windowHeight-dialog.height)/2;

		dialog.y = finalYPos = ypos;
		if (ypos <= windowHeight/2)
			startYPos = -300;
		else
			startYPos = windowHeight + 300;
		dialog.open();
	}
}
