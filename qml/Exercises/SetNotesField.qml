import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: _control
	spacing: 5

	property alias info: lblMain.text
	property alias text: setNotesArea.text
	property alias editable: setNotesArea.editable

	signal editFinished(string new_text);

	Row {
		Layout.fillWidth: true
		Layout.preferredHeight: AppSettings.itemDefaultHeight
		spacing: 5

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
				if (setNotesArea.visible)
					setNotesArea.forceActiveFocus();
				else {
					if (setNotesArea.modified)
						_control.editFinished(setNotesArea.contentsText());
				}
			}
		}
	}

	TPMultiLineEdit {
		id: setNotesArea
		visible: false
		Layout.fillWidth: true

		onTextAltered: (_text) => _control.editFinished(_text);
	}
}
