// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

import "../TPWidgets"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

ToolBar {
	id: root
	width: parent.width
	height: 40
	spacing: 0
	padding: 0

	property var mainCalendar: null
	property var mainTimer: null

	TPButton {
		id: btnBack
		imageSource: "back.png"
		hasDropShadow: false
		enabled: stackView.depth >= 2 && mainwindow.bBackButtonEnabled
		visible: stackView.depth >= 2

		anchors {
			left: parent.left
			leftMargin: 5
			verticalCenter: parent.verticalCenter
		}

		onClicked: popFromStack();
	}

	TPButton {
		id: btnHome
		imageSource: "home.png"
		hasDropShadow: false
		enabled: btnBack.enabled
		visible: btnBack.visible

		anchors {
			left: btnBack.right
			verticalCenter: parent.verticalCenter
		}

		onClicked: {
			pageDeActivated_main(stackView.currentItem);
			stackView.pop(stackView.get(0));
			pageActivated_main(stackView.currentItem);
		}
	}

	RoundButton {
		id: btnMainMenu
		padding: 5
		anchors {
			top: parent.top
			topMargin: 5
			right: parent.right
			rightMargin: 5
		}

		TPImage {
			source: "mainmenu"
			dropShadow: false
			width: 30
			height: 30
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}

		onClicked: mainMenu.open();
	}

	RoundButton {
		id: btnCalendar
		padding: 5
		anchors {
			top: parent.top
			topMargin: 5
			right: btnMainMenu.left
			rightMargin: 10
		}

		TPImage {
			source: "calendar"
			dropShadow: false
			width: 30
			height: 30
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}

		onClicked: {
			if (mainCalendar === null) {
				var component = Qt.createComponent("qrc:/qml/Dialogs/CalendarDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					mainCalendar = component.createObject(mainwindow, { parentPage: homePage, showDate:new Date(),
						simpleCalendar:true, initDate: new Date(2000, 0, 1), finalDate: new Date(2025, 11, 31) });
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
		padding: 5
		anchors {
			top: parent.top
			topMargin: 5
			right: btnCalendar.left
			rightMargin: 10
		}

		TPImage {
			source: "timer"
			dropShadow: false
			width: 30
			height: 30
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter
		}

		onClicked: {
			if (mainTimer === null) {
				var component = Qt.createComponent("qrc:/qml/Dialogs/TimerDialog.qml", Qt.Asynchronous);

				function finishCreation() {
					mainTimer = component.createObject(mainwindow, { parentPage: homePage, simpleTimer:true });
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
