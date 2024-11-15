pragma Singleton

import QtCore
import QtQuick

QtObject {

	function customFont(ffamily: int, fweight: int, fitalic: bool, fsize: int) {
		return Qt.font({
			family: Qt.fontFamilies()[ffamily],
			weight: fweight,
			italic: fitalic,
			styleStrategy: Font.PreferAntialias,
			pixelSize: fsize
		});
	}

	readonly property font regularFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.Medium,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    hintingPreference: Font.PreferFullHinting,
	    pixelSize: appSettings.fontSize
	})

	readonly property FontMetrics fontMetricsRegular: FontMetrics {
		font.family: regularFont.family
		font.pixelSize: regularFont.pixelSize
		font.weight: regularFont.weight
	}

	readonly property font smallFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.DemiBold,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    hintingPreference: Font.PreferFullHinting,
	    pixelSize: appSettings.smallFontSize
	})

	readonly property FontMetrics fontMetricsSmall: FontMetrics {
		font.family: smallFont.family
		font.pixelSize: smallFont.pixelSize
		font.weight: smallFont.weight
	}

	readonly property font largeFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.DemiBold,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    hintingPreference: Font.PreferFullHinting,
	    pixelSize: appSettings.largeFontSize
	})

	readonly property FontMetrics fontMetricsLarge: FontMetrics {
		font.family: largeFont.family
		font.pixelSize: largeFont.pixelSize
		font.weight: largeFont.weight
	}

	readonly property font extraLargeFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.Bold,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    hintingPreference: Font.PreferFullHinting,
	    pixelSize: appSettings.extraLargeFontSize
	})

	readonly property FontMetrics fontMetricsExtraLarge: FontMetrics {
		font.family: extraLargeFont.family
		font.pixelSize: extraLargeFont.pixelSize
		font.weight: extraLargeFont.weight
	}

	property ListModel setTypesModel: ListModel {
		ListElement { text: qsTr("Regular"); value: 0; enabled: true; }
		ListElement { text: qsTr("Pyramid"); value: 1; enabled: true; }
		ListElement { text: qsTr("Drop Set"); value: 2; enabled: true; }
		ListElement	{ text: qsTr("Cluster Set"); value: 3; enabled: true; }
		ListElement { text: qsTr("Giant Set"); value: 4; enabled: true; }
		ListElement { text: qsTr("Myo Reps"); value: 5; enabled: true; }
		ListElement { text: qsTr("Inverted Pyramid"); value: 6; enabled: true; }
	}

	property ListModel splitModel: ListModel {
		ListElement { text: "A"; value: "A"; enabled: true; }
		ListElement { text: "B"; value: "B"; enabled: true; }
		ListElement { text: "C"; value: "C"; enabled: true; }
		ListElement { text: "D"; value: "D"; enabled: true; }
		ListElement { text: "E"; value: "E"; enabled: true; }
		ListElement { text: "F"; value: "F"; enabled: true; }
		ListElement { text: "R"; value: "R"; enabled: true; }
	}
}
