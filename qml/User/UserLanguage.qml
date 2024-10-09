import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

GridLayout {
	columns: 2
	rows: 3
	Layout.rightMargin: 10
	Layout.leftMargin: 10
	rowSpacing: controlsSpacing

	property bool bReady: false
	readonly property int controlsSpacing: 10
	readonly property int minimumHeight: optEn.height + optBr.height + optDe.height + 2*controlsSpacing
	readonly property int topMargin: (availableHeight - minimumHeight)/2

	TPRadioButton {
		id: optEn
		text: "Application Language: English"
		checked: false
		Layout.column: 0
		Layout.row: 0
		Layout.fillWidth: true
		Layout.topMargin: topMargin

		onClicked: {
			appTr.switchToLanguage("en_US");
			bReady = appTr.translatorOK();
		}
	}

	TPImage {
		source: "us.png"
		width: 60
		height: 32
		Layout.column: 1
		Layout.row: 0
		Layout.minimumWidth: width
		Layout.maximumWidth: width
		Layout.topMargin: topMargin
	}

	TPRadioButton {
		id: optBr
		text: "Linguagem do aplicativo: Português do Brasil"
		checked: false
		Layout.column: 0
		Layout.row: 1
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("pt_BR");
			bReady = appTr.translatorOK();
		}
	}

	TPImage {
		source: "brazil.png"
		width: 60
		height: 32
		Layout.column: 1
		Layout.row: 1
		Layout.minimumWidth: width
		Layout.maximumWidth: width
	}

	TPRadioButton {
		id: optDe
		text: "Sprache des Apps: Deutsch von Deutschland"
		checked: false
		Layout.column: 0
		Layout.row: 2
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("de_DE");
			bReady = appTr.translatorOK();
		}
	}

	TPImage {
		source: "deutschland.png"
		width: 60
		height: 32
		Layout.column: 1
		Layout.row: 2
		Layout.minimumWidth: width
		Layout.maximumWidth: width
	}
}
