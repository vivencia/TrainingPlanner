import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

Item {
	id: _control

	property alias info: lblMain.text
	property alias text: setNotesArea.text
	property alias editable: setNotesArea.editable
	property alias preferredHeight: setNotesArea.preferredHeight

	signal editFinished(string new_text);

	Row {
		id: topBar
		height: AppSettings.itemDefaultHeight
		spacing: 5

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}

		TPLabel {
			id: lblMain
			text: qsTr("Notes:")
			width: _control.width * 0.9
		}

		TPButton {
			imageSource: setNotesArea.visible ? "fold-up.png" : "fold-down.png"
			hasDropShadow: false
			width: AppSettings.itemSmallHeight
			height: width

			onClicked: {
				setNotesArea.visible = !setNotesArea.visible;
				if (setNotesArea.visible) {
					setNotesArea.forceActiveFocus();
				} else {
					if (setNotesArea.modified)
						_control.editFinished(setNotesArea.contentsText());
				}
			}
		}
	}

	TPMultiLineEdit {
		id: setNotesArea
		visible: false

		anchors {
			fill: parent
			margins: 5
			topMargin: topBar.height + 5
		}

		textEdited: _control.editFinished(text);
	}
}
