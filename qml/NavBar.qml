// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ToolBar {
	id: root
	width: parent.width
	height: 40

	property var mainCalendar: null
	property var mainTimer: null

	TPButton {
		id: btnBack
		enabled: stackView.depth >= 2
		anchors.left: parent.left
		anchors.leftMargin: 5
		visible: stackView.depth >= 2
		anchors.verticalCenter: parent.verticalCenter
		text: qsTr("BACK")
		imageSource: "qrc:/images/"+lightIconFolder+"back.png"

		onClicked: {
			stackView.pop();
			backButtonPressed();
		}
	}

	RoundButton {
		id: btnSettings
		anchors.right: parent.right
		anchors.rightMargin: 5

		Image {
			source: "qrc:/images/"+darkIconFolder+"menu.png"
			width: 20
			height: 20
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
			fillMode: Image.PreserveAspectFit
		}

		onClicked: mainMenu.open();
	}

	RoundButton {
		id: btnCalendar
		anchors.right: btnSettings.left
		anchors.rightMargin: 5

		Image {
			source: "qrc:/images/"+darkIconFolder+"calendar.png"
			width: 20
			height: 20
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
			fillMode: Image.PreserveAspectFit
		}

		onClicked: {
			if (mainCalendar === null) {
				var component = Qt.createComponent("CalendarDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					mainCalendar = component.createObject(mainwindow, { showDate:new Date(), simpleCalendar:true,
						initDate: new Date(2000, 0, 1), finalDate: new Date(2025, 11, 31) });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			mainCalendar.open();
		}
	}

	RoundButton {
		id: btnTimer
		anchors.right: btnCalendar.left
		anchors.rightMargin: 5

		Image {
			source: "qrc:/images/"+darkIconFolder+"time.png"
			width: 20
			height: 20
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
			fillMode: Image.PreserveAspectFit
		}

		onClicked: {
			if (mainTimer === null) {
				var component = Qt.createComponent("TimerDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					mainTimer = component.createObject(mainwindow, { simpleTimer:true });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			mainTimer.open();
		}
	}

	Component.onDestruction: {
		if (mainCalendar !== null)
			mainCalendar.destroy();
		if (mainTimer !== null)
			mainTimer.destroy();
	}
}
