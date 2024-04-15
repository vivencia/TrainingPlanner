// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

pragma Singleton

import Qt.labs.settings //QtCore for 6.7

Settings {
	signal appFontSizeChanged()

	property string appLocale
	property string weightUnit
	property string themeStyle
	property string colorScheme
	property string primaryDarkColor
	property string primaryColor
	property string primaryLightColor
	property string exercisesListVersion
	property int themeStyleIndex
	property int fontSize
	property int fontSizeLists
	property int fontSizeText
	property int fontSizeTitle

	onFontSizeChanged: appFontSizeChanged();
}
