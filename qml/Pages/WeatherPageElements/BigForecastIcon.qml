// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Effects

import "../../TPWidgets"
import "../.."

Item {
	id: current

	property string topText: "20*"
	property string bottomText: "Mostly cloudy"
	property string weatherIcon: "sunny"
	property real smallSide: (current.width < current.height ? current.width : current.height)

	TPLabel {
		id: text1
		text: current.topText
		font: AppGlobals.titleFont
		singleLine: true
		fontColor: "white"
		width: appSettings.pageWidth
		horizontalAlignment: Text.AlignHCenter

		anchors.top: parent.top
		anchors.horizontalCenter: parent.horizontalCenter
	}

	MultiEffect {
		source: text1
		anchors.fill: text1
		shadowEnabled: true
		shadowBlur: 0.5
		shadowHorizontalOffset: 0
		shadowVerticalOffset: 2
		shadowOpacity: 0.6
	}

	WeatherIcon {
		id: img
		weatherIcon: current.weatherIcon
		width: current.smallSide * 0.5
		height: current.smallSide * 0.5
		anchors.top: text1.bottom
		anchors.topMargin: 5
		anchors.horizontalCenter: parent.horizontalCenter
	}

	TPLabel {
		id: text2
		text: current.bottomText
		font: AppGlobals.titleFont
		horizontalAlignment: Text.AlignHCenter
		fontColor: "white"
		anchors.top: img.bottom
		anchors.topMargin: 5
		anchors.horizontalCenter: parent.horizontalCenter
	}

	MultiEffect {
		source: text2
		anchors.fill: text2
		shadowEnabled: true
		shadowBlur: 0.5
		shadowHorizontalOffset: 0
		shadowVerticalOffset: 2
		shadowOpacity: 0.6
	}
}
