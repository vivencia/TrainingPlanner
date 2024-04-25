import QtQuick
import QtQuick.Controls

TextField {
	property string textColor: "white"
	property string backgroundColor: AppSettings.primaryDarkColor//"black"//"#c3cad5"
	property bool highlight: false

	id: control
	font.pointSize: AppSettings.fontSizeText
	font.weight: Font.Bold //Font.ExtraBold
	color: textColor
	//wrapMode: Text.WordWrap
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

	TPBalloonTip {
		id: textCopiedInfo
		height: 40
		message: qsTr("Text copied to the clipboard")
	}

	MouseArea {
		anchors.fill: parent
		pressAndHoldInterval: 300
		onPressAndHold: (mouse) => {
			runCmd.copyToClipBoard(control.text);
			textCopiedInfo.showTimed(3000, 0);
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
		border.color: "black"
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
