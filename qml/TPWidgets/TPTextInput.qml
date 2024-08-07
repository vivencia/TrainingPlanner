import QtQuick
import QtQuick.Controls

import "../"

TextField {
	property string textColor: AppSettings.fontColor
	property string backgroundColor: AppSettings.primaryDarkColor
	property bool highlight: false

	id: control
	font.pointSize: AppSettings.fontSizeText
	font.weight: Font.Bold //Font.ExtraBold
	color: enabled ? textColor : AppSettings.disabledFontColor
	leftInset: 0
	rightInset: 0
	topInset: 0
	bottomInset: 0
	leftPadding: highlight ? 40 : 5
	topPadding: 0
	bottomPadding: 0
	rightPadding: 5
	placeholderTextColor: "gray"
	implicitWidth: fontMetrics.boundingRect("LorenIpsuM").width + 15
	implicitHeight: fontMetrics.boundingRect("LorenIpsuM").height + 10

	signal enterOrReturnKeyPressed()

	onHighlightChanged: {
		if (highlight)
			anim.start();
		else
			anim.stop();
	}

	FontMetrics {
		id: fontMetrics
		font.family: control.font.family
		font.pointSize: AppSettings.fontSizeText
	}

	MouseArea {
		anchors.fill: parent
		pressAndHoldInterval: 300
		onPressAndHold: (mouse) => {
			runCmd.copyToClipBoard(control.text);
			mainwindow.showTextCopiedMessage();
		}
		onClicked: (mouse) => {
			mouse.accepted = false;
			control.forceActiveFocus();
		}
		onPressed: (mouse) => {
			mouse.accepted = true;
			control.forceActiveFocus();
		}
	}

	background: Rectangle {
		id: itemBack
		border.color: textColor
		color: backgroundColor
		radius: 6
		opacity: 0.5
	}

	Image {
		id: highlightIcon
		source: "qrc:/images/indicator.png"
		fillMode: Image.PreserveAspectFit
		width: 20
		height: 20
		visible: highlight
	}

	Keys.onPressed: (event) => {
		switch (event.key) {
			case Qt.Key_Enter:
			case Qt.Key_Return:
				event.accepted = true;
				enterOrReturnKeyPressed();
			break;
			default: return;
		}
	}

	SequentialAnimation {
		id: anim
		loops: Animation.Infinite

		PropertyAnimation {
			target: highlightIcon
			property: "x"
			to: 20
			duration: 600
			easing.type: Easing.InOutCubic
		}

		PropertyAnimation {
			target: highlightIcon
			property: "x"
			to: 0
			duration: 600
			easing.type: Easing.InOutCubic
		}
	}
}
