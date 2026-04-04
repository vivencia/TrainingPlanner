pragma Singleton
import QtQuick

QtObject {
	id: _globals

	readonly property font regularFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.Medium,
	    hintingPreference: Font.PreferFullHinting,
		pixelSize: AppSettings.fontSize
	})

	readonly property FontMetrics fontMetricsRegular: FontMetrics {
		font.family: _globals.regularFont.family
		font.pixelSize: _globals.regularFont.pixelSize
		font.weight: _globals.regularFont.weight
	}

	readonly property font smallFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.DemiBold,
	    hintingPreference: Font.PreferFullHinting,
		pixelSize: AppSettings.smallFontSize
	})

	readonly property font largeFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.DemiBold,
	    hintingPreference: Font.PreferFullHinting,
		pixelSize: AppSettings.largeFontSize
	})

	readonly property font extraLargeFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.Bold,
	    hintingPreference: Font.PreferFullHinting,
		pixelSize: AppSettings.extraLargeFontSize
	})

	readonly property ListModel setTypesModel: ListModel {
		ListElement { text: qsTr("Regular"); value: 0; enabled: true; }
		ListElement { text: qsTr("Pyramid"); value: 1; enabled: true; }
		ListElement { text: qsTr("Inverted Pyramid"); value: 2; enabled: true; }
		ListElement { text: qsTr("Drop Set"); value: 3; enabled: true; }
		ListElement	{ text: qsTr("Cluster Set"); value: 4; enabled: true; }
		ListElement { text: qsTr("Myo Reps"); value: 5; enabled: true; }
	}

	readonly property ListModel splitModel: ListModel {
		ListElement { text: "A"; value: "A"; enabled: true; }
		ListElement { text: "B"; value: "B"; enabled: true; }
		ListElement { text: "C"; value: "C"; enabled: true; }
		ListElement { text: "D"; value: "D"; enabled: true; }
		ListElement { text: "E"; value: "E"; enabled: true; }
		ListElement { text: "F"; value: "F"; enabled: true; }
		ListElement { text: "R"; value: "R"; enabled: true; }
	}
}
