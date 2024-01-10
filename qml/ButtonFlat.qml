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
	width: buttonText.contentWidth + (imageSource.length > 0 ? buttonImage.width + 5: - 5)
	font.capitalization: Font.MixedCase

	contentItem: Text {
		id: buttonText
		text: button.text
		opacity: enabled ? 1.0 : 0.3
		color: "black"
		horizontalAlignment: imageSource ? Text.AlignHCenter : Text.AlignLeft
		verticalAlignment: Text.AlignVCenter

		Image {
			id: buttonImage
			width: 15
			height: 15
			fillMode: Image.PreserveAspectFit
			anchors.verticalCenter: parent.verticalCenter
			anchors.left: buttonText.right
			mirror: false
		}
	}

	background: Rectangle {
		id: buttonBackground
		anchors.fill: parent
		anchors.topMargin: 10
		anchors.bottomMargin: 10
		anchors.rightMargin: - 10
		color: button.pressed ? buttonText.color : primaryColor
		radius: 10
		opacity: button.pressed ? 0.12 : 1.0
		border.color: "black"
	} // background
} // button
