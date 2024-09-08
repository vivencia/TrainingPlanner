import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"

ColumnLayout {
	spacing: controlsSpacing
	Layout.rightMargin: 10
	Layout.leftMargin: 10

	property bool bReady: false
	readonly property int moduleHeight: availableHeight
	readonly property int nControls: 2
	readonly property int controlsHeight: 25
	readonly property int controlsSpacing: 10

	TPRadioButton {
		id: optEn
		text: "Application Language: English"
		checked: false
		Layout.fillWidth: true
		Layout.topMargin: (moduleHeight - nControls*controlsHeight - controlsSpacing)/2

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
