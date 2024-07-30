import QtQuick
import QtQuick.Controls
import QtQuick.Effects

import "../"

Popup {
	id: tppopup
	objectName: "TPPopup"
	closePolicy: bKeepAbove ? Popup.NoAutoClose : Popup.CloseOnPressOutside
	//modal: bKeepAbove
	parent: Overlay.overlay //global Overlay object. Assures that the dialog is always displayed in relation to global coordinates
	spacing: 0
	padding: 0
	focus: true

	required property Page parentPage
	property bool bKeepAbove
	property bool bVisible: false
	property int finalYPos: y
	property int startYPos: 0

	Component.onCompleted: {
		if (bKeepAbove) {
			parentPage.pageDeActivated.connect(function() { bVisible = tppopup.visible; tppopup.visible = false; });
			parentPage.pageActivated.connect(function() { if (bVisible) tppopup.visible = true; });
		}
	}

	Rectangle {
		id: backRec
		implicitHeight: height
		implicitWidth: width
		radius: 8
		layer.enabled: true
		visible: false
		color: AppSettings.primaryDarkColor
	}

	background: backRec

	MultiEffect {
		id: backgroundEffect
		visible: true
		source: backRec
		anchors.fill: backRec
		shadowEnabled: true
		shadowOpacity: 0.5
		blurMax: 16
		shadowBlur: 1
		shadowHorizontalOffset: 5
		shadowVerticalOffset: 5
		shadowColor: "black"
		shadowScale: 1
		opacity: 0.9
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
	}

	enter: Transition {
		NumberAnimation {
			property: "y"
			from: startYPos
			to: finalYPos
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			property: "opacity"
			from: 0
			to: 0.9
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}

	exit: Transition {
		id: closeTransition
		NumberAnimation {
			property: "y"
			from: finalYPos
			to: startYPos
			duration: 500
			easing.type: Easing.InOutCubic
		}
		NumberAnimation {
			property: "opacity"
			from: 0.9
			to: 0
			duration: 500
			easing.type: Easing.InOutCubic
		}
	}
}
