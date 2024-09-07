import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"

ColumnLayout {
	spacing: 10

	property bool bReady: false

	TPRadioButton {
		id: optEn
		text: "Application Language: English"
		checked: false
		Layout.fillWidth: true

		onClicked: {
			optBr.checked = !checked;
			appTr.switchToLanguage("en_US");
			bReady = appTr.translatorOK();
		}
	}

	TPRadioButton {
		id: optBr
		text: "Linguagem do aplicativo: Português do Brasil"
		checked: false
		Layout.fillWidth: true

		onClicked: {
			optEn.checked = !checked;
			appTr.switchToLanguage("pt_BR");
			bReady = appTr.translatorOK();
		}
	}
}
