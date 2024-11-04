// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: top

    property string topText: "Mon"
    property string middleIcon: "sunny"
    property string bottomText: "22*/23*"

    implicitHeight: dayText.implicitHeight + icon.height + tempText.implicitHeight + 20

    Text {
        id: dayText
        horizontalAlignment: Text.AlignHCenter
        width: parent.width
        text: top.topText
        color: "white"
        font.pixelSize: 16

        anchors {
			top: parent.top
			margins: 5
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
			horizontalCenter: parent.horizontalCenter
		}
    }

    Text {
        id: tempText
        text: top.bottomText
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 16
        width: top.width

        anchors {
			bottom: parent.bottom
			bottomMargin: 10
			horizontalCenter: parent.horizontalCenter
		}
    }
}
