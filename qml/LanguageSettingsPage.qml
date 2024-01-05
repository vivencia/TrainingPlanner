import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	id: languagePage
	property var appLanguages: ["English", "Português", "Deutsch"]
	property var appLocales: ["en_US", "pt_BR", "de_DE"]

	ColumnLayout {
		anchors.fill: parent

		RowLayout {
			width: parent.width - 60
			spacing: 12
			Layout.alignment: Qt.AlignHCenter
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 10
			Layout.topMargin: 10

			Label {
				text: qsTr("Application Language")
			}

			ComboBox {
				id: cboSetType
				model: appLanguages
				currentIndex: {
					switch (AppSettings.appLocale) {
						case appLocales[0]: return 0;
						case appLocales[1]: return 1;
						case appLocales[2]: return 2;
					}
				}

				onActivated: (index) => {
					btnApplyChanges.enabled = true;
					lblWarning.visible = false;
				}
			}
		}

		GroupBox {
			title: qsTr("Measuring Units");
			font.pixelSize: AppSettings.fontSizeText
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 10
			Layout.topMargin: 10

			ColumnLayout {
				anchors.fill: parent

				RadioButton {
					id: optInternationalUnits
					text: qsTr("International Units")
					checked: AppSettings.weightUnit === "(kg)"
					Layout.fillWidth: true
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked) {
							btnApplyChanges.enabled = true;
							lblWarning.visible = false;
						}
					}
				}

				RadioButton {
					id: optImperialUnits
					text: qsTr("Imperial Units")
					checked: AppSettings.weightUnit === "(lb)"
					Layout.fillWidth: true
					Layout.leftMargin: 10

					onCheckedChanged: {
						if (checked) {
							btnApplyChanges.enabled = true;
							lblWarning.visible = false;
						}
					}
				}
			}
		}

		Label {
			id: lblWarning
			text: qsTr("The App must be restarted in order to reflect the changes")
			visible: false
			wrapMode: Text.WordWrap

			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter
			Layout.leftMargin: 10
			Layout.rightMargin: 10
			Layout.topMargin: 10
		}

		ButtonFlat {
			id: btnApplyChanges
			text: qsTr("Apply")
			enabled: false
			Layout.alignment: Qt.AlignHCenter

			onClicked: {
				if (optInternationalUnits.checked)
					AppSettings.weightUnit = "(kg)";
				else
					AppSettings.weightUnit = "(lb)";

				AppSettings.appLocale = appLocales[cboSetType.currentIndex];

				lblWarning.visible = true;
				btnApplyChanges.enabled = false;
			}
		}
	} //ColumnLayout
}
