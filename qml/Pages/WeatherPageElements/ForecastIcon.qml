// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import "../../TPWidgets"
import "../.."

Item {
    id: top

    property string topText: "Mon"
    property string middleIcon: "sunny"
    property string bottomText: "22*/23*"

    implicitHeight: dayText.implicitHeight + icon.height + tempText.implicitHeight + 20

    TPLabel {
        id: dayText
        text: top.topText
        font.pointSize: 16
        width: parent.width

        anchors {
			top: parent.top
			margins: 20
			horizontalCenter: parent.horizontalCenter
		}
    }

    WeatherIcon {
        id: icon
        weatherIcon: top.middleIcon
        height: 50
        width: height

        anchors {
			top: dayText.bottom
			margins: 20
			horizontalCenter: parent.horizontalCenter
		}
    }

    TPLabel {
        id: tempText
        text: top.bottomText
        font.pointSize: 18
        width: top.width

        anchors {
			bottom: parent.bottom
			margins: 20
			horizontalCenter: parent.horizontalCenter
		}
    }
}
