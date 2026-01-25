import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ColumnLayout {
	spacing: 20

	readonly property bool bReady: languages_group.selectedOption !== -1
	readonly property list<string> country_flags: ["us", "brazil", "deutschland"]

	TPButtonGroup {
		id: languages_group
	}

	Repeater {
		model: appSettings.availableLanguages.length

		delegate: TPRadioButtonOrCheckBox {

			required property int index

			text: appSettings.availableLanguagesLabel(index)
			checked: appSettings.userLocaleIdx === index
			image: country_flags[index]
			multiLine: true
			buttonGroup: languages_group
			imageWidth: imageHeight * 2
			Layout.fillWidth: true

			onClicked: appTr.switchToLanguage(appSettings.availableLanguages[index], true);
		}
	}
}
