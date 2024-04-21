import QtQuick
import QtQuick.Controls

ToolTip {
	id: firstTimeTip
	property string message
	property bool showTwoImages: false
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	visible: false

	FontMetrics {
		id: fontMetrics
		font.family: textPart.font.family
		font.pointSize: AppSettings.fontSizeTitle
	}

	onVisibleChanged: {
		if (visible)
			anim.start();
		else
			anim.stop();
	}

	Image {
		id: img1
		source: "qrc:/images/point-down.png"
		height: 30
		width: 30
		fillMode: Image.PreserveAspectFit

		anchors {
			left: textPart.right
			leftMargin: -5
			verticalCenter: parent.verticalCenter
		}
	}

	Image {
		id: img2
		source: "qrc:/images/point-down.png"
		height: 30
		width: 30
		fillMode: Image.PreserveAspectFit
		visible: showTwoImages

		anchors {
			right: textPart.left
			rightMargin: -5
			verticalCenter: parent.verticalCenter
		}
	}

	contentItem: Text {
		id: textPart
		text: message
		font.bold: true
		font.pointSize: AppSettings.fontSizeTitle
		color: "white"
		width: fontMetrics.boundingRect(text).width + 10
		height: fontMetrics.boundingRect(text).height + 5
		anchors {
			leftMargin: showTwoImages ? 40 : 5
			rightMargin: 40
			horizontalCenter: parent.horizontalCenter
			verticalCenter: parent.verticalCenter
		}
	}

	background: Rectangle {
		border.color: AppSettings.primaryDarkColor
		border.width: 2
		color: AppSettings.primaryDarkColor
		opacity: 0.7
		radius: 7
	}

	SequentialAnimation {
		id: anim
		loops: Animation.Infinite

		PropertyAnimation {
			target: firstTimeTip
			property: "y"
			to: firstTimeTip.y - 20
			duration: 500
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: firstTimeTip
			property: "y"
			to: firstTimeTip.y + 10
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}
} //ToolTip
