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

	TPLabel {
		id: lblTopText
		text: topText
		singleLine: false
		horizontalAlignment: Text.AlignHCenter
		Layout.fillWidth: true
		Layout.maximumHeight: parent.height * 0.3
	}

	TPImage {
		source: "weather/weather-" + weatherIcon + ".svg"
		smooth: true
		width: parent.height * 0.3
		height: width
		Layout.preferredWidth: width
		Layout.preferredHeight: height
		Layout.alignment: Qt.AlignCenter
	}

	TPLabel {
		id: lblBottomText
		text: bottomText
		font: AppGlobals.smallFont
		horizontalAlignment: Text.AlignHCenter
		singleLine: false
		Layout.fillWidth: true
		Layout.maximumHeight: parent.height * 0.2
	}

	TPLabel {
		id: lblBottomBottomText
		text: bottomBottomText
		font: AppGlobals.smallFont
		horizontalAlignment: Text.AlignHCenter
		singleLine: false
		Layout.fillWidth: true
		Layout.maximumHeight: parent.height * 0.2
	}
}
