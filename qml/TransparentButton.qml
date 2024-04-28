import QtQuick
import QtQuick.Controls

ToolButton {
	property string imageSource
	property bool leftAlign: true

	signal buttonClicked(int clickid);
	property bool bEmitSignal: false
	property int clickId: -1

	id: button
	height: 40
	rightPadding: 0
	leftPadding: 0
	rightInset: 0
	leftInset: 0

	onPressed: anim.start();
	onReleased: bEmitSignal = true;

	contentItem: Text {
		visible: false
	}

	FontMetrics {
		id: fontMetrics
		font.family: buttonText.font.family
		font.pointSize: AppSettings.fontSizeText
	}

	Text {
		id: buttonText
		text: button.text
		font.pointSize: AppSettings.fontSizeText
		font.bold: true
		font.capitalization: Font.MixedCase
		color: AppSettings.fontColor
		anchors.verticalCenter: parent.verticalCenter

		Component.onCompleted: {
			const textWidth = fontMetrics.boundingRect(text).width;
			if (textWidth >= parent.width) {
				width = parent.width;
				wrapMode = Text.WordWrap;
			}
			else
				width = textWidth;
			if (leftAlign) {
				anchors.left = parent.left;
				anchors.leftMargin = imageSource.length === 0 ? 5 : buttonImage.width;
			}
			else {
				anchors.right = parent.right;
				anchors.rightMargin = 10;
			}
		}
	}

	Image {
		id: buttonImage
		source: imageSource
		visible: imageSource.length > 0
		fillMode: Image.PreserveAspectFit
		width: 20
		height: 20

		anchors {
			right: buttonText.left
			verticalCenter: parent.verticalCenter
		}
	}

	background: Rectangle {
		id: buttonBack
		color: AppSettings.primaryLightColor
		opacity: 0.3

		property double fillPosition: !anim.running

		Behavior on fillPosition {
			NumberAnimation {
				id: flash
				duration: 300
			}
		}

		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0;								color: AppSettings.primaryLightColor }
			GradientStop { position: buttonBack.fillPosition - 0.001;	color: AppSettings.primaryLightColor }
			GradientStop { position: buttonBack.fillPosition + 0.001;	color: AppSettings.primaryColor }
			GradientStop { position: 1.0;								color: AppSettings.primaryColor }
		}
	}

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: button
			property: "scale"
			to: 1.4
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: button
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: {
			if (bEmitSignal) {
				bEmitSignal = false;
				buttonClicked(clickId);
			}
		}
	}
}
