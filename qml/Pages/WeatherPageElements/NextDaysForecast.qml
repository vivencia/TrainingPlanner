// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: _control
	spacing: 5

	property string topText: "Mon"
	property string middleIcon: "sunny"
	property string bottomText: "22*/23*"

	TPLabel {
		text: _control.topText
		horizontalAlignment: Text.AlignHCenter
		font.pixelSize: 16
		Layout.fillWidth: true
	}

	TPImage {
		source: "weather/weather-" + _control.middleIcon + ".svg"
		smooth: true
		Layout.preferredWidth: AppSettings.itemExtraLargeHeight
		Layout.preferredHeight: AppSettings.itemExtraLargeHeight
		Layout.alignment: Qt.AlignCenter
	}

	TPLabel {
		text: _control.bottomText
		horizontalAlignment: Text.AlignHCenter
		font.pixelSize: 18
		Layout.fillWidth: true
	}
}
