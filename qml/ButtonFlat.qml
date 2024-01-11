import QtQuick
import QtQuick.Controls

Rectangle {
	id: button
	property alias textColor: buttonText.color
	property alias imageMirror: buttonImage.mirror
	property alias font: buttonText.font
	property alias text: buttonText.text
	property string imageSource
	property bool bPressed: false
	signal clicked();

	color: bPressed ? primaryColor : paneBackgroundColor
	radius: 10
	opacity: button.enabled ? (bPressed ? 0.12 : 1.0) : 0.3
	implicitWidth: buttonText.contentWidth + (buttonImage.visible ? buttonImage.width + 10 : 10)
	implicitHeight: buttonText.height + 10

	Label {
		id: buttonText
		opacity: button.enabled ? 1.0 : 0.3
		color: button.enabled ? "white" : "black"
		anchors.verticalCenter: parent.verticalCenter
		anchors.left: parent.left
		anchors.leftMargin: 5
		font.weight: Font.Medium
		font.bold: true
	}

	Image {
		id: buttonImage
		width: 20
		height: 20
		fillMode: Image.PreserveAspectFit
		anchors.verticalCenter: parent.verticalCenter
		mirror: false
		source: imageSource
		visible: imageSource.length > 1
	}

	MouseArea {
		anchors.fill: parent
		onClicked: (mouse) => {
			if (!mouse.wasHeld)
				button.clicked();
			bPressed = false;
		}
		onPressed: (mouse) => {
			mouse.accepted = true;
			bPressed = true;
		}
		onReleased: (mouse) => {
			mouse.accepted = true;
			bPressed = false;
		}
	}

	Component.onCompleted: {
		if (buttonText.text.length === 0)
			buttonImage.anchors.horizontalCenter = button.horizontalCenter
		else
			buttonImage.anchors.left = buttonText.right;
	}
} //Rectangle
