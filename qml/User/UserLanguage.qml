import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".."
import "../TPWidgets"
import TpQml

ColumnLayout {
	id: mainLayout
	spacing: 20

	readonly property bool bReady: languages_group.selectedOption !== -1
	readonly property list<string> country_flags: ["us", "brazil", "deutschland"]

	TPButtonGroup {
		id: languages_group
	}

	Repeater {
		model: AppSettings.availableLanguages.length
		delegate: TPRadioButtonOrCheckBox {
			required property int index

			text: AppSettings.availableLanguagesLabel(index)
			checked: AppSettings.userLocaleIdx === index
			image: mainLayout.country_flags[index]
			multiLine: true
			buttonGroup: languages_group
			imageWidth: AppSettings.itemExtraLargeHeight
			imageHeight: AppSettings.itemDefaultHeight
			Layout.fillWidth: true

			onClicked: appTr.switchToLanguage(AppSettings.availableLanguages[index], true);
		}
	}
}
