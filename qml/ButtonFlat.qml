import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

// Flat Button
Button {
	id: button
	property alias textColor: buttonText.color
	property alias imageSource: buttonImage.source
	property alias imageMirror: buttonImage.mirror

	focusPolicy: Qt.NoFocus
	leftPadding: 6
	rightPadding: 6
	width: buttonText.contentWidth + buttonImage.width + 5

	contentItem: Text {
		id: buttonText
		text: button.text
		opacity: enabled ? 1.0 : 0.3
		color: flatButtonTextColor
		horizontalAlignment: imageSource ? Text.AlignHCenter : Text.AlignLeft
		verticalAlignment: Text.AlignVCenter
		font {
			capitalization: Font.AllUppercase
			weight: Font.Medium
		}

		Image {
			id: buttonImage
			width: 15
			height: 15
			fillMode: Image.PreserveAspectFit
			anchors.verticalCenter: parent.verticalCenter
			anchors.left: buttonText.right
			mirror: true
		}
	}

	background: Rectangle {
		id: buttonBackground
		Layout.fillWidth: true
		Layout.fillHeight: true
		color: button.pressed ? buttonText.color : "transparent"
		radius: 5
		opacity: button.pressed ? 0.12 : 1.0
	} // background
} // button
