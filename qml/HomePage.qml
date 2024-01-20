// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

import "jsfunctions.js" as JSF

Page {
	id: homePage
	property ListModel mainModel: mainMesosModel
	property int currentMesoIndex
	property date minimumStartDate;
	property var firstTimeTip: null
	property bool bFirstTime: false

	header: IconLabel {
		text: qsTr("  Training Program")
		color: "white"
		font.weight: Font.ExtraBold
		font.pixelSize: AppSettings.titleFontSizePixelSize
		icon.source: "qrc:/images/"+lightIconFolder+"mesocycle.png"
		font.styleName: "Semibold"
		topPadding: 20
		bottomPadding: 20
	}

	background: Rectangle {
		color: primaryDarkColor
		opacity: 0.7
		Image {
			anchors.fill: parent
			source: "qrc:/images/app_logo.png"
			fillMode: Image.PreserveAspectFit
			opacity: 0.6
		}
	}

	ListView {
		id: mesosListView
		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		clip: true
		spacing: 10

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: ScrollBar.AsNeeded
		}

		model: ListModel {
			id: mainMesosModel

			Component.onCompleted: {
				let mesos = Database.getMesos();
				if (mesos.length !== 0) {
					for (let meso of mesos)
						append(meso);
				}
				else {
					var component = Qt.createComponent("FirstTimeHomePageTip.qml");
					if (component.status === Component.Ready) {
						firstTimeTip = component.createObject(homePage, { message:qsTr("Start here"), yPos:homePageToolBar.y - homePageToolBar.height, xPos:(homePage.width - width) / 2 });
						firstTimeTip.visible = true;
						firstTimeTip.startAnim();
						homePage.StackView.activating.connect(pageActivation);
						bFirstTime = true;
					}
				}
			}

			onCountChanged: {
				if (count >= 1 && bFirstTime)
					bFirstTime = false;
			}
		}

		delegate: SwipeDelegate {
			id: mesoDelegate
			width: ListView.view.width
			height: mesoContent.implicitHeight

			required property int index
			required property int mesoId
			required property string mesoName
			required property string mesoNote
			required property date mesoStartDate
			required property date mesoEndDate
			required property int nWeeks
			required property string mesoSplit
			property string mesoDrugs
			required property bool realMeso

			Rectangle {
				id: recRemoveMeso
				width: 80
				height: parent.height
				color: "red"
				radius: 6
				visible: false
				z: 1

				Rectangle {
					anchors.centerIn: parent
					width: 80
					height: 80
					color: "transparent"

					Text {
						text: qsTr("Remove Mesocycle")
						anchors.bottom: parent.bottom
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.bottomMargin: 5
					}

					Image {
						source: "qrc:/images/"+lightIconFolder+"remove.png"
						height: 40
						width: 40
						anchors.top: parent.top
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.topMargin: 5
					}

					MouseArea {
						anchors.fill: parent
						onClicked: msgDlg.open();
					}
				}

				MessageDialog {
					id: msgDlg
					text: qsTr("\n\nRemove Mesocycle?\n\n")
					informativeText: qsTr("This action cannot be undone. Note: removing a Mesocycle does not remove the records of the days within it.")
					buttons: MessageDialog.Yes | MessageDialog.No

					onButtonClicked: function (button, role) {
						switch (button) {
							case MessageDialog.Yes:
								Database.deleteMeso(mesoId);
								mainMesosModel.remove(mesoDelegate.index, 1);
								dateTimer.triggered(); //Update tabBar and the meso model index it uses
								accept();
							break;
							case MessageDialog.No:
								reject();
							break;
						}
					}

					onRejected: recRemoveMeso.visible = false;
				}
			} //Rectangle recRemoveMeso

			MouseArea {
				id: swipeDetector
				anchors.fill: parent
				preventStealing: true
				property int xStart: 0
				property int xPrev: 0
				property bool tracing: false
				property int swipeWidth
				property bool bFromRight

				onClicked: {
					showMeso();
					swipeWidth = 0;
					recRemoveMeso.visible = false;
				}

				onPressAndHold: (mouse) => {
					tracing = true;
					if (!recRemoveMeso.visible) {
						swipeWidth = 0;
						bFromRight = mouse.x >= width / 2;
						recRemoveMeso.anchors.left = undefined;
						recRemoveMeso.anchors.right = undefined;
						if (bFromRight)
							recRemoveMeso.anchors.right = parent.right;
						else
							recRemoveMeso.anchors.left = parent.left;
						recRemoveMeso.width = 0;
						recRemoveMeso.visible = true;
					}
				}

				onPositionChanged: (mouse) => {
					if ( !tracing ) return;
					if (bFromRight)
						swipeWidth = parent.width - mouse.x;
					else
						swipeWidth = mouse.x;

					if (swipeWidth < parent.width * 0.8) {
						if (swipeWidth <= parent.width * 0.2)
							swipeWidth = 0;
						recRemoveMeso.width = swipeWidth;
					}
				}

				onReleased: (mouse) => {
					if (tracing) {
						tracing = false;
						if (swipeWidth <= 0) recRemoveMeso.visible = false;
					}
				}
			} //MouseArea

			background: Rectangle {
				radius: 6
				opacity: 0.8
				color: primaryColor
			}

			Column {
				id: mesoContent
				topPadding: 10
				bottomPadding: 10
				leftPadding: 10

				Label {
					text: qsTr("Name: <b>") + mesoName + "</b>"
					color: "white"
					width: mesoDelegate.width
					elide: Text.ElideRight
				}
				Label {
					text: realMeso ?
							qsTr("Start of mesocycle: <b>") + JSF.formatDateToDisplay(mesoStartDate, AppSettings.appLocale) + "</b>" :
							qsTr("Program start date: <b>") + JSF.formatDateToDisplay(mesoStartDate, AppSettings.appLocale) + "</b>"
					color: "white"
				}
				Label {
					text: realMeso ?
							qsTr("End of mesocycle: <b>") + JSF.formatDateToDisplay(mesoEndDate, AppSettings.appLocale) + "</b>" :
							qsTr("Open-ended program - no end date set")
					color: "white"
				}
				Label {
					text: qsTr("Weeks in mesocycle: <b>") + nWeeks.toString() + "</b>"
					color: "white"
					visible: realMeso
				}
				Label {
					text: qsTr("Training Split: <b>") + mesoSplit + "</b>"
					color: "white"
				}
			}

			Connections {
				target: mesoDelegate
				function onClicked() {
					currentMesoIndex = index;
				}
			}
		} //delegate
	} //ListView

footer: ToolBar {
		id: homePageToolBar
		width: parent.width
		height: 55

		background: Rectangle {
			color: primaryDarkColor
			opacity: 0.7
		}

		ButtonFlat {
			id: btnAddOpenSchedule
			text: qsTr("New open-ended schedule")
			textUnderIcon: true
			imageSource: "qrc:/images/"+lightIconFolder+"open-schedule.png"
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.leftMargin: 5

			onClicked: newAction(0);
		}

		ButtonFlat {
			id: btnAddMeso
			text: qsTr("New Mesocycle")
			imageSource: "qrc:/images/"+lightIconFolder+"mesocycle-add.png"
			textUnderIcon: true
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			anchors.rightMargin: 5

			onClicked: newAction(1);
		}
} // footer

	function newAction(opt) {
		if (firstTimeTip)
			firstTimeTip.visible = false;

		var startDate, endDate;
		if (mainMesosModel.count === 0) {
			minimumStartDate = new Date(2023, 0, 2); //first monday of year
			startDate = today;
			endDate = JSF.createFutureDate(startDate, 0, 2, 0);
		}
		else {
			getMesoStartDate();
			startDate = minimumStartDate;
			endDate = JSF.createFutureDate(minimumStartDate, 0, 2, 0);
		}
		const weekOne = JSF.weekNumber(startDate);
		const weekTwo = JSF.weekNumber(endDate);

		if (opt === 1) {
			homePage.StackView.view.push("MesoCycle.qml",  {
				mesosModel: mainMesosModel,
				mesoId: -1,
				mesoName: "Novo mesociclo",
				mesoStartDate: startDate,
				mesoEndDate: endDate,
				mesoNote: "",
				nWeeks: JSF.calculateNumberOfWeeks(weekOne, weekTwo),
				mesoSplit:"ABCRDER",
				mesoDrugs: " ",
				minimumMesoStartDate: minimumStartDate,
				maximumMesoEndDate: JSF.createFutureDate(startDate,0,6,0),
				fixedMesoEndDate: endDate,
				week1: weekOne,
				week2: weekTwo,
				calendarStartDate: startDate
			});
		}
		else {
			homePage.StackView.view.push("OpenEndedPlan.qml", {
				mesosModel: mainMesosModel,
				mesoId: -1,
				mesoSplit: "ABC",
				mesoStartDate: startDate,
				minimumMesoStartDate: minimumStartDate,
				maximumMesoEndDate: new Date(2026,11,31),
				calendarStartDate: startDate,
				bFirstTime: bFirstTime,
				firstTimeTip: firstTimeTip
			});
		}
	}

	function showMeso() {
		let meso = mainMesosModel.get(currentMesoIndex);
		var startDate;
		if (currentMesoIndex === 0) {
			minimumStartDate = new Date(2023, 0, 2); //first monday of year
			startDate = today;
		}
		else {
			startDate = meso.mesoStartDate;
		}

		if (meso.realMeso) {
			const weekOne = JSF.weekNumber(meso.mesoStartDate);
			const weekTwo = JSF.weekNumber(meso.mesoEndDate);

			homePage.StackView.view.push("MesoCycle.qml", {
				mesosModel: mainMesosModel,
				idxModel: currentMesoIndex,
				mesoId: meso.mesoId,
				mesoName: meso.mesoName,
				mesoStartDate: meso.mesoStartDate,
				mesoEndDate: meso.mesoEndDate,
				mesoNote: meso.mesoNote,
				nWeeks: meso.nWeeks,
				mesoSplit: meso.mesoSplit,
				mesoDrugs: meso.mesoDrugs,
				minimumMesoStartDate: Database.getPreviousMesoEndDate(meso.mesoId),
				maximumMesoEndDate: Database.getNextMesoStartDate(meso.mesoId),
				week1: weekOne,
				week2: weekTwo,
				calendarStartDate: startDate
			});
		}
		else {
			homePage.StackView.view.push("OpenEndedPlan.qml", {
				mesosModel: mainMesosModel,
				idxModel: currentMesoIndex,
				mesoId: meso.mesoId,
				mesoSplit: meso.mesoSplit,
				mesoStartDate: meso.mesoStartDate,
				minimumMesoStartDate: Database.getPreviousMesoEndDate(meso.mesoId),
				maximumMesoEndDate: new Date(2026,11,31),
				calendarStartDate: startDate
			});
		}
	}

	function getMesoStartDate() {
		var date = Database.getLastMesoEndDate();
		var day = date.getDate();
		var month = date.getMonth();
		var year = date.getFullYear();

		var daysToNextMonday = [1, 2, 3, 4, 5, 6, 0];
		day = day + daysToNextMonday[date.getDay()]; //Always start at next monday

		var totalDays = JSF.getMonthTotalDays(month, year);
		if (day > totalDays) {
			day -= totalDays;
			month++;
			if (month > 11) {
				month = 0;
				year++;
			}
		}
		minimumStartDate = new Date(year,month,day);
	}

	function pageActivation() {
		if (firstTimeTip && mainMesosModel.count === 0) {
			firstTimeTip.message = qsTr("Start here");
			firstTimeTip.yPos = homePageToolBar.y;
			firstTimeTip.xPos = (homePage.width - width) / 2;
			firstTimeTip.visible = true;
		}
	}
} //Page
