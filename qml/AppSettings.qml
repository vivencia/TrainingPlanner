// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

pragma Singleton

import Qt.labs.settings

Settings {
	property string appLocale
	property string weightUnit
	property string themeStyle
	property int themeStyleIndex
	property int themeStyleColorIndex
	property int fontSize
	property int fontSizeLists
	property int fontSizeText
	property int fontSizeTitle
	property int fontSizePixelSize
	property int titleFontSizePixelSize
	property int hugeFontSizePixelSize
	property real exercisesListVersion
}
