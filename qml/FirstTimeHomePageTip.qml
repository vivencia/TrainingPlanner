import QtQuick
import QtQuick.Controls

ToolTip {
	id: firstTimeTip
	x: xPos
	y: yPos
	property int yPos
	property int xPos
	property string message

	FontMetrics {
		id: fontMetrics
		font.family: textPart.font.family
		font.pixelSize: AppSettings.titleFontSizePixelSize
	}

	/*onYPosChanged: {
		const point = homePage.mapToGlobal(xPos,yPos);
		firstTimeTip.x = point.x;
		firstTimeTip.y = point.y;
	}*/

	contentItem: Text {
		id: textPart
		text: message
		font.bold: true
		font.pixelSize: AppSettings.titleFontSizePixelSize
		color: "white"
		width: fontMetrics.boundingRect(text).width + 10
		height: fontMetrics.boundingRect(text).height + 5
		anchors {
			left: parent.left
			verticalCenter: parent.verticalCenter
		}
	}

	background: Rectangle {
		border.color: primaryDarkColor
		border.width: 2
		color: primaryDarkColor
		opacity: 0.7
		radius: 7
	}

	Image {
		source: "qrc:/images/point-down.png"
		height: 30
		width: 30
		fillMode: Image.PreserveAspectFit

		anchors {
			left: textPart.right
			verticalCenter: parent.verticalCenter
		}
	}

	SequentialAnimation {
		id: anim
		loops: Animation.Infinite

		PropertyAnimation {
			target: firstTimeTip
			property: "yPos"
			to: firstTimeTip.yPos - 20
			duration: 500
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: firstTimeTip
			property: "yPos"
			to: firstTimeTip.yPos + 10
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	function startAnim() {
		anim.start();
	}
} //ToolTip
