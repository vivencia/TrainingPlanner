import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ColumnLayout {
	Layout.rightMargin: 10
	Layout.leftMargin: 10
	spacing: 20

	property bool bReady: false
	readonly property int minimumHeight: optEn.height + optBr.height + optDe.height + 2*spacing

	Connections {
        target: appSettings
        function onAppLocaleChanged(): void {
			optEn.checked = appSettings.appLocale === "en_US";
			optBr.checked = appSettings.appLocale === "pt_BR";
			optDe.checked = appSettings.appLocale === "de_DE";
        }
    }

	TPRadioButton {
		id: optEn
		text: "Application Language: English"
		image: "us"
		multiLine: true
		imageWidth: 60
		imageHeight: 32
		checked: appSettings.appLocale === "en_US"
		Layout.fillWidth: true
		Layout.topMargin: availableHeight === appSettings.pageHeight ? 10 : (availableHeight - minimumHeight)/2 //When placed on a page or when placed on a dialog

		onClicked: {
			appTr.switchToLanguage("en_US");
			bReady = appTr.translatorOK();
		}
	}

	TPRadioButton {
		id: optBr
		text: "Linguagem do aplicativo: Português do Brasil"
		image: "brazil"
		multiLine: true
		imageWidth: 60
		imageHeight: 32
		checked: appSettings.appLocale === "pt_BR"
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("pt_BR");
			bReady = appTr.translatorOK();
		}
	}

	TPRadioButton {
		id: optDe
		text: "Sprache des Apps: Deutsch von Deutschland"
		image: "deutschland"
		multiLine: true
		imageWidth: 60
		imageHeight: 32
		checked: appSettings.appLocale === "de_DE"
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("de_DE");
			bReady = appTr.translatorOK();
		}
	}
}
