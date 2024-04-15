import QtQuick
import QtQuick.Controls

ToolButton {
	signal buttonClicked(int clickid);
	property bool bEmitSignal: false
	property int clickId: -1

	id: button
	width: parent.width
	height: 40
	rightPadding: 0
	leftPadding: 0
	rightInset: 0
	leftInset: 0

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

	contentItem: Text {
		text: button.text
		color: "white"
		font.pixelSize: AppSettings.fontSizeText
		font.capitalization: Font.MixedCase
		font.bold: true
		wrapMode: Text.WordWrap
	}

	onPressed: {
		anim.start();
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

	onReleased: {
		bEmitSignal = true;
	}
}
