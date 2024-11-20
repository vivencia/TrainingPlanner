// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Item {
    id: container

    property string weatherIcon: "sunny"

    TPImage {
        id: mask
        source: "weather/weather-" + container.weatherIcon + ".svg"
        smooth: true
        anchors.fill: parent
    }
}
