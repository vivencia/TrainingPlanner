pragma Singleton

import QtCore
import QtQuick

QtObject {

	readonly property font regularFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.Medium,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    pointSize: appSettings.fontSize
	})

	readonly property FontMetrics fontMetricsRegular: FontMetrics {
		font.family: regularFont.family
		font.pointSize: regularFont.pointSize
		font.weight: regularFont.weight
	}

	readonly property font listFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.DemiBold,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    pointSize: appSettings.fontSizeLists
	})

	readonly property FontMetrics fontMetricsList: FontMetrics {
		font.family: listFont.family
		font.pointSize: listFont.pointSize
		font.weight: listFont.weight
	}

	readonly property font textFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.DemiBold,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    pointSize: appSettings.fontSizeText
	})

	readonly property FontMetrics fontMetricsText: FontMetrics {
		font.family: textFont.family
		font.pointSize: textFont.pointSize
		font.weight: textFont.weight
	}

	readonly property font titleFont: Qt.font({
	    family: Qt.fontFamilies()[0],
	    weight: Font.Bold,
	    italic: false,
	    styleStrategy: Font.PreferAntialias,
	    pointSize: appSettings.fontSizeTitle
	})

	readonly property FontMetrics fontMetricsTitle: FontMetrics {
		font.family: titleFont.family
		font.pointSize: titleFont.pointSize
		font.weight: titleFont.weight
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
}
