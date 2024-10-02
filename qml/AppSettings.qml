// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

pragma Singleton

import QtCore
import QtQuick

Settings {
	signal appFontSizeChanged()

	property string appVersion
	property string appLocale
	property string weightUnit
	property string themeStyle
	property string colorScheme
	property string primaryDarkColor
	property string primaryColor
	property string primaryLightColor
	property string paneBackgroundColor
	property string entrySelectedColor
	property string exercisesListVersion
	property string backupFolder
	property string fontColor
	property string disabledFontColor
	property string iconFolder
	property int fontSize
	property int fontSizeLists
	property int fontSizeText
	property int fontSizeTitle
	property int lastViewedMesoIdx
	property bool alwaysAskConfirmation

	onFontSizeChanged: appFontSizeChanged();

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
