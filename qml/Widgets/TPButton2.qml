pragma ComponentBehavior: Bound

import QtQuick

import TpQml

TPRadioButtonOrCheckBox {
	id: _control
	showBorder: !flat
	radius: rounded ? height : 8
	color: backgroundColor
	useDropShadow: true
	useGradient: enabled && text.length > 0
	boxType: !checkable ? TPRadioButtonOrCheckBox.TP_NONEBOX : TPRadioButtonOrCheckBox.TP_CHECKBOX
	indicatorPos: !textUnderIcon ? Qt.AlignRight|Qt.AlignVCenter : Qt.AlignTop|Qt.AlignRight
	imagePos: !textUnderIcon ? Qt.AlignLeft|Qt.AlignVCenter : Qt.AlignTop|Qt.AlignLeft
	enableClicks: true
	enableCheckOutsideIndicator: false

//public:
	property string backgroundColor: text.length > 0 ? AppSettings.paneBackgroundColor : "transparent"
	property bool textUnderIcon: false
	property bool flat: text.length === 0
	property bool iconOnTheLeft: false
	property bool rounded: true
	property bool checkable: false
	property int clickId: -1

	signal buttonClicked(int clickid)

	onClicked: anim.start();

	SequentialAnimation {
		id: anim
		alwaysRunToEnd: true

		// Expand the button
		PropertyAnimation {
			target: _control
			property: "scale"
			to: 1.5
			duration: 200
			easing.type: Easing.InOutCubic
		}

		// Shrink back to normal
		PropertyAnimation {
			target: _control
			property: "scale"
			to: 1.0
			duration: 200
			easing.type: Easing.InOutCubic
		}

		onFinished: _control.buttonClicked(_control.clickId);
	}
} //Rectangle
