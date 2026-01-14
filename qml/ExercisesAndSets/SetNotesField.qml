import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

ColumnLayout {
	id: control
	spacing: 5

	property alias info: lblMain.text
	property alias text: setNotesArea.text
	property alias editable: setNotesArea.editable

	signal editFinished(string new_text);

	Row {
		Layout.fillWidth: true
		height: appSettings.itemDefaultHeight
		spacing: 5

		TPLabel {
			id: lblMain
			text: qsTr("Notes:")
			width: control.width * 0.9
		}

		TPButton {
			imageSource: setNotesArea.visible ? "fold-up.png" : "fold-down.png"
			hasDropShadow: false
			width: appSettings.itemSmallHeight
			height: width

			onClicked: {
				setNotesArea.visible = !setNotesArea.visible;
				if (setNotesArea.visible)
					setNotesArea.forceActiveFocus();
			}
		}
	}

	TPMultiLineEdit {
		id: setNotesArea
		visible: false
		Layout.fillWidth: true

		onTextAltered: (_text) => editFinished(_text);
	}
}
