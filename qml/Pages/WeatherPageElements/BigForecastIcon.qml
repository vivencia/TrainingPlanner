// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../../TPWidgets"
import "../.."

ColumnLayout {
	spacing: 0

	property string topText: "20*"
	property string weatherIcon: "sunny"
	property string bottomText: "Mostly cloudy"
	property string bottomBottomText: "Mostly cloudy"

	readonly property real smallSide: (width < height ? width : height)

	TPLabel {
		id: lblTopText
		text: topText
		font: AppGlobals.extraLargeFont
		singleLine: false
		horizontalAlignment: Text.AlignHCenter
		Layout.fillWidth: true
		Layout.leftMargin: 10
		Layout.rightMargin: 10
		Layout.maximumHeight: height
	}

	TPImage {
		source: "weather/weather-" + weatherIcon + ".svg"
		smooth: true
		width: smallSide * 0.3
		height: width
		Layout.maximumWidth: width
		Layout.maximumHeight: height
		Layout.alignment: Qt.AlignCenter
	}

	TPLabel {
		id: lblBottomText
		text: bottomText
		horizontalAlignment: Text.AlignHCenter
		singleLine: false
		Layout.fillWidth: true
	}

	TPLabel {
		id: lblBottomBottomText
		text: bottomBottomText
		horizontalAlignment: Text.AlignHCenter
		singleLine: false
		Layout.fillWidth: true
	}
}
