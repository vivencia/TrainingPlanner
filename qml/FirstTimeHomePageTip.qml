import QtQuick
import QtQuick.Controls

ToolTip {
	id: firstTimeTip
	x: (homePage.width - width) / 2
	y: initialPos
	property int initialPos: 0

	FontMetrics {
		id: fontMetrics
		font.family: textPart.font.family
		font.pixelSize: AppSettings.titleFontSizePixelSize
	}

	Component.onCompleted: {
		const point = homePage.mapFromGlobal(homePageToolBar.x, homePageToolBar.y);
		initialPos = point.y
	}

	contentItem: Text {
		id: textPart
		text: qsTr("Start here")
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
			property: "initialPos"
			to: firstTimeTip.initialPos - 20
			duration: 500
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: firstTimeTip
			property: "initialPos"
			to: firstTimeTip.initialPos + 10
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	function startAnim() {
		anim.start();
	}
} //ToolTip
