// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

ColumnLayout {
	id: _control
	spacing: 0

	property string topText: "20*"
	property string weatherIcon: "sunny"
	property string bottomText: "Mostly cloudy"
	property string bottomBottomText: "Mostly cloudy"

	TPLabel {
		id: lblTopText
		text: _control.topText
		singleLine: false
		horizontalAlignment: Text.AlignHCenter
		Layout.fillWidth: true
		Layout.maximumHeight: parent.height * 0.3
	}

	TPImage {
		source: "weather/weather-" + _control.weatherIcon + ".svg"
		smooth: true
		Layout.preferredWidth: parent.height * 0.3
		Layout.preferredHeight: parent.height * 0.3
		Layout.alignment: Qt.AlignCenter
	}

	TPLabel {
		id: lblBottomText
		text: _control.bottomText
		font: AppGlobals.smallFont
		horizontalAlignment: Text.AlignHCenter
		singleLine: false
		Layout.fillWidth: true
		Layout.maximumHeight: parent.height * 0.2
	}

	TPLabel {
		id: lblBottomBottomText
		text: _control.bottomBottomText
		font: AppGlobals.smallFont
		horizontalAlignment: Text.AlignHCenter
		singleLine: false
		Layout.fillWidth: true
		Layout.maximumHeight: parent.height * 0.2
	}
}
