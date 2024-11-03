// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Effects

import "../../TPWidgets"
import "../.."

Item {
	id: current

	property string topText: "20*"
	property string weatherIcon: "sunny"
	property string bottomText: "Mostly cloudy"
	property string bottomBottomText: "Mostly cloudy"

	property real smallSide: (current.width < current.height ? current.width : current.height)

	TPLabel {
		id: text1
		text: current.topText
		singleLine: false
		fontColor: "white"
		font: AppGlobals.extraLargeFont
		horizontalAlignment: Text.AlignHCenter
		width: appSettings.pageWidth

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

		anchors {
			top: text1.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}
	}

	TPLabel {
		id: text2
		text: current.bottomText
		fontColor: "white"
		font: AppGlobals.regularFont
		horizontalAlignment: Text.AlignHCenter
		width: appSettings.pageWidth

		anchors {
			top: img.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}
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

	TPLabel {
		id: text3
		text: current.bottomBottomText
		font: AppGlobals.customFont(0, Font.bold, false, 16)
		_textWidth: appSettings.pageWidth - 20
		_textHeight: 15
		leftPadding: 10
		fontColor: "white"
		singleLine: false
		width: appSettings.pageWidth

		anchors {
			top: text2.bottom
			topMargin: 5
			horizontalCenter: parent.horizontalCenter
		}
	}

	MultiEffect {
		source: text3
		anchors.fill: text3
		shadowEnabled: true
		shadowBlur: 0.5
		shadowHorizontalOffset: 0
		shadowVerticalOffset: 2
		shadowOpacity: 0.6
	}
}
