import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ColumnLayout {
	spacing: 20

	property bool bReady: false

	onBReadyChanged: {
		if (bReady)
			appSettings.appLocale = appTr.language();
	}

	Connections {
        target: appSettings
        function onAppLocaleChanged(): void {
			optEn.checked = appSettings.appLocale === "en_US";
			optBr.checked = appSettings.appLocale === "pt_BR";
			optDe.checked = appSettings.appLocale === "de_DE";
        }
    }

	TPRadioButtonOrCheckBox {
		id: optEn
		text: "Application Language: English"
		image: "us"
		multiLine: true
		imageWidth: imageHeight * 2
		checked: appSettings.appLocale === "en_US"
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("en_US");
			bReady = appTr.translatorOK();
		}
	}

	TPRadioButtonOrCheckBox {
		id: optBr
		text: "Linguagem do aplicativo: Português do Brasil"
		image: "brazil"
		multiLine: true
		imageWidth: imageHeight * 2
		checked: appSettings.appLocale === "pt_BR"
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("pt_BR");
			bReady = appTr.translatorOK();
		}
	}

	TPRadioButtonOrCheckBox {
		id: optDe
		text: "Sprache des Apps: Deutsch von Deutschland"
		image: "deutschland"
		multiLine: true
		imageWidth: imageHeight * 2
		checked: appSettings.appLocale === "de_DE"
		Layout.fillWidth: true

		onClicked: {
			appTr.switchToLanguage("de_DE");
			bReady = appTr.translatorOK();
		}
	}
}
