import QtQuick

import TpQml

TPPopup {
	objectName: "scrollButtons"
	id: _control
	height: 2 * _button_size
	width: _button_size + 5
	keepAbove: true
	closeButtonVisible: false
	showTitleBar: false
	enableEffects: false
	showBorder: false
	useGradient: false
	focus: false
	show_position: Qt.AlignBaseline
	configFieldName: "navButton_" + parentPage.objectName
	defaultCoordinates: Qt.point(AppSettings.pageWidth - width - 20, parentPage.height - height - 20)
	defaultBackgroundColor: "transparent"
	mouseItem: contentItem

	signal scrollTo(int pos);

	property bool showUpButton: false
	property bool showDownButton: true

//private:
	readonly property int _button_size: AppSettings.itemLargeHeight

	onMouseItemClicked: (mouse) => {
		if (mouse.y <= _control._button_size) {
			btnUp._bPressed = true;
			btnUp.onMouseReleased(mouse);
		} else {
			btnDown._bPressed = true;
			btnDown.onMouseReleased(mouse);
		}
	}

	TPButton {
		id: btnUp
		imageSource: "upward"
		visible: _control.visible && _control.showUpButton
		focus: false
		width: _control._button_size
		height: _control._button_size
		radius: _control._button_size

		anchors {
			top: parent.top
			left: parent.left
		}

		onClicked: _control.scrollTo(0);
	}

	TPButton {
		id: btnDown
		imageSource: "downward"
		visible: _control.visible && _control.showDownButton
		focus: false
		width: _control._button_size
		height: _control._button_size
		radius: _control._button_size

		anchors {
			top: btnUp.bottom
			left: parent.left
		}

		onClicked: _control.scrollTo(1);
	}
}
