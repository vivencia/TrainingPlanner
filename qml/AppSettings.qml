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
	property int lastViewedMesoId
	property bool alwaysAskConfirmation

	onFontSizeChanged: appFontSizeChanged();

	property ListModel setTypesModel: ListModel {
		ListElement { text: qsTr("Regular"); value: 0; }
		ListElement { text: qsTr("Pyramid"); value: 1; }
		ListElement { text: qsTr("Drop Set"); value: 2; }
		ListElement	{ text: qsTr("Cluster Set"); value: 3; }
		ListElement { text: qsTr("Giant Set"); value: 4; }
		ListElement { text: qsTr("Myo Reps"); value: 5; }
		ListElement { text: qsTr("Inverted Pyramid"); value: 6; }
	}
}
