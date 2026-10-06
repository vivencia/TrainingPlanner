import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Dialogs
import TpQml.Widgets

TPPopup {
	id: _dialog
	keepAbove: true
	dim: true
	width: AppSettings.pageWidth * 0.8
	height: titleBar.height + topRow.height + warningRow.height + chkDontAskAgain.height + buttonsRow.height
	useShape: true
	showBorder: true
	showTitleBar: true
	open_in_window: true
	canSlideToClose: true

//public:
	property int requestid
	property string identifier
	property string message
	property string title
	property list<string> dontAskAgainList
	property list<int> closeActionList

	TPLabel {
		id: lblTitle
		text: _dialog.title
		horizontalAlignment: Text.AlignHCenter
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: 5
			rightMargin: _dialog.titleBar.titleBarButtons.width
		}
	}

	RowLayout {
		id: topRow
		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			margins: 5
			topMargin: 10
		}

		TPImage {
			id: imgElement
			source: "remove"
			Layout.preferredWidth: AppSettings.itemExtraLargeHeight
			Layout.preferredHeight: AppSettings.itemExtraLargeHeight
			Layout.alignment: Qt.AlignVCenter
		}

		TPLabel {
			id: lblMessage
			text: _dialog.message
			singleLine: false
			horizontalAlignment: Text.AlignJustify
			visible: _dialog.message.length > 0
			Layout.preferredWidth: _dialog.width - imgElement.width - 20
		}
	}

	RowLayout {
		id: warningRow
		spacing: 10
		height: AppSettings.itemDefaultHeight

		anchors {
			top: topRow.bottom
			left: parent.left
			right: parent.right
			margins: 5
			topMargin: 10
		}

		TPImage {
			source: "warning"
			preferredWidth: AppSettings.itemDefaultHeight
			preferredHeight: AppSettings.itemDefaultHeight
		}

		TPLabel {
			text: qsTr("This action cannot be undone")
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkDontAskAgain
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		text: qsTr("Don't ask again for ") + _dialog.identifier
		helpString: qsTr("For the duration of this session, any subsequent removal request will repeat whatever you choose now without confirmation")

		anchors {
			top: warningRow.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		onChecked: (check) => {
			if (check)
				_dialog.dontAskAgainList.push(_dialog.identifier);
			else
				_dialog.dontAskAgainList.pop();
		}
	}

	Row {
		id: buttonsRow
		spacing: (_dialog.width - btn1.width - btn2.width) / 3

		anchors {
			bottom: parent.bottom
			left: parent.left
			right: parent.right
			margins: 5
			leftMargin: spacing
		}

		TPButton {
			id: btn1
			text: qsTr("Yes")
			Layout.alignment: Qt.AlignHCenter
			onClicked: _dialog.buttonClicked(TPPopup.DEFAULT_ACTION);
		}

		TPButton {
			id: btn2
			text: qsTr("Cancel")
			Layout.alignment: Qt.AlignHCenter
			onClicked: _dialog.buttonClicked(TPPopup.BTN_CLOSE);
		}
	}

	function buttonClicked(close_action: int): void {
		if (chkDontAskAgain.isChecked)
			closeActionList.push(close_action);
		closePopup(close_action);
	}
}
