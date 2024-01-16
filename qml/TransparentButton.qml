import QtQuick
import QtQuick.Controls

ToolButton {
	signal buttonClicked();
	property bool bEmitSignal: false

	id: button
	width: parent.width
	height: 40
	rightPadding: 0
	leftPadding: 0
	rightInset: 0
	leftInset: 0

	background: Rectangle {
		id: buttonBack
		color: primaryLightColor
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
			GradientStop { position: 0.0;								color: primaryLightColor }
			GradientStop { position: buttonBack.fillPosition - 0.001;	color: primaryLightColor }
			GradientStop { position: buttonBack.fillPosition + 0.001;	color: primaryColor }
			GradientStop { position: 1.0;								color: primaryColor }
		}
	}

	contentItem: Text {
		text: button.text
		color: "white"
		font.pixelSize: AppSettings.fontSizeText
		font.capitalization: Font.MixedCase
		font.bold: true
		leftPadding: (parent.width - contentWidth) / 2
	}

	onPressed: {
		anim.start();
	}

	SequentialAnimation {
		id: anim

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
				buttonClicked();
			}
		}
	}

	onReleased: {
		bEmitSignal = true;
	}
}
