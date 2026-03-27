pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: _control
	spacing: 20

	readonly property bool bReady: languages_group.selectedOption !== -1
	readonly property list<string> country_flags: ["us", "brazil", "deutschland"]

	TPButtonGroup {
		id: languages_group
	}

	Repeater {
		model: AppSettings.availableLanguages.length
		delegate: TPRadioButtonOrCheckBox {
			text: AppSettings.availableLanguagesLabel(index)
			checked: AppSettings.userLocaleIdx === index
			image: _control.country_flags[index]
			multiLine: true
			buttonGroup: languages_group
			imageWidth: AppSettings.itemExtraLargeHeight
			imageHeight: AppSettings.itemDefaultHeight
			Layout.fillWidth: true

			required property int index

			onClicked: AppTr.switchToLanguage(AppSettings.availableLanguages[index], true);
		}
	}
}
