// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../../TPWidgets"
import "../.."

ColumnLayout {
	spacing: 5

	property string topText: "Mon"
	property string middleIcon: "sunny"
	property string bottomText: "22*/23*"

	TPLabel {
		text: topText
		horizontalAlignment: Text.AlignHCenter
		font.pixelSize: 16
		Layout.fillWidth: true
	}

	TPImage {
		source: "weather/weather-" + middleIcon + ".svg"
		smooth: true
		width: 50
		height: 50
		Layout.preferredHeight: 50
		Layout.preferredWidth: 50
		Layout.alignment: Qt.AlignCenter
	}

	TPLabel {
		text: bottomText
		horizontalAlignment: Text.AlignHCenter
		font.pixelSize: 18
		Layout.fillWidth: true
	}
}
