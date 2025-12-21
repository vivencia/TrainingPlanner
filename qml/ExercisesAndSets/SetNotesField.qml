import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"

Column {
	id: control
	padding: 5

	property alias info: lblMain.text
	property alias text: setNotesArea.text
	property alias editable: setNotesArea.editable

	signal editFinished(string new_text);

	Row {
		Layout.fillWidth: true
		height: appSettings.itemDefaultHeight

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
		Layout.minimumHeight: appSettings.pageHeight * 0.15
		Layout.maximumHeight: appSettings.pageHeight * 0.15

		onTextAltered: (_text) => editFinished(_text);
	}
}
