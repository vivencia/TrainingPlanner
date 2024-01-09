// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

ToolBar {
	id: root
	width: parent.width

	required property StackView stackView
	property var mainCalendar: null
	property var mainTimer: null

	ToolButton {
		id: btnBack
		enabled: true //bNavButtonsEnabled && root.stackView.depth >= 2
		anchors.left: parent.left
		anchors.leftMargin: 5
		visible: root.stackView.depth >= 2
		anchors.verticalCenter: parent.verticalCenter
		display: AbstractButton.TextBesideIcon
		//text: root.stackView.depth > 2 ? qsTr("Back") : qsTr("Home")
		text: qsTr("Back")
		icon.source: "qrc:/images/"+lightIconFolder+"back.png"
		icon.height: 20
		icon.width: 20

		onClicked: {
			root.stackView.pop();
			backButtonPressed();
		}
	}

	ToolButton {
		id: btnSettings
		anchors.right: parent.right
		anchors.rightMargin: 5
		icon.source: "qrc:/images/"+lightIconFolder+"menu.png"
		icon.height: 20
		icon.width: 20
		onClicked: mainMenu.open();
	}

	ToolButton {
		id: btnCalendar
		anchors.right: btnSettings.left
		anchors.rightMargin: 5
		icon.source: "qrc:/images/"+lightIconFolder+"calendar.png"
		icon.height: 20
		icon.width: 20

		onClicked: {
			if (mainCalendar === null) {
				var component;
				component = Qt.createComponent("CalendarDialog.qml");
				if (component.status === Component.Ready) {
					mainCalendar = component.createObject(this, { showDate:today, simpleCalendar:true,
						initDate: new Date(2000, 0, 1), finalDate: new Date(2025, 11, 31) });
				}
			}
			mainCalendar.open();
		}
	}

	ToolButton {
		id: btnTimer
		anchors.right: btnCalendar.left
		anchors.rightMargin: 5
		icon.source: "qrc:/images/"+lightIconFolder+"time.png"
		icon.height: 20
		icon.width: 20

		onClicked: {
			if (mainTimer === null) {
				var component;
				component = Qt.createComponent("TimerDialog.qml");
				if (component.status === Component.Ready) {
					mainTimer = component.createObject(this, { simpleTimer:true });
				}
			}
			mainTimer.open();
		}
	}
}

