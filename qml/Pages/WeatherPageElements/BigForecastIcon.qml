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
		wrapMode: Text.WordWrap
		font: AppGlobals.extraLargeFont
		fontColor: "black"
		horizontalAlignment: Text.AlignHCenter
		heightAvailable: 0.1*appSettings.pageHeight
		singleLine: false

		anchors {
			top: parent.top
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
		}

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

	TPButton {
		imageSource: "reload"
		imageSize: 50
		fixedSize: true
		width: 50
		height: 50

		anchors {
			top: parent.top
			topMargin: 40
			right: parent.right
			rightMargin: 10
		}

		onClicked: weatherInfo.refreshWeather();
	}

	WeatherIcon {
		id: img
		weatherIcon: current.weatherIcon
		width: current.smallSide * 0.4
		height: current.smallSide * 0.4

		anchors {
			top: text1.bottom
			topMargin: {
				Qt.platform.os === "android" ? -30 : 0;
			}
			horizontalCenter: parent.horizontalCenter
		}
	}

	TPLabel {
		id: text2
		text: current.bottomText
		font: AppGlobals.regularFont
		fontColor: "black"
		horizontalAlignment: Text.AlignHCenter
		width: parent.width

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
		fontColor: "black"
		horizontalAlignment: Text.AlignHCenter
		singleLine: false

		anchors {
			top: text2.bottom
			topMargin: 15
			left: parent.left
			leftMargin: 5
			right: parent.right
			rightMargin: 5
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
