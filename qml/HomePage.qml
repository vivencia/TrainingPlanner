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

	header: IconLabel {
		text: qsTr("Mesocycles")
		icon.source: "qrc:/images/"+darkIconFolder+"mesocycle.png"
		font.styleName: "Semibold"
		topPadding: 20
		bottomPadding: 20
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
				for (let meso of mesos)
					append(meso);
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
			required property string mesoDrugs

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
				opacity: 0.3
				color: paneBackgroundColor
			}

			Column {
				id: mesoContent
				topPadding: 10
				bottomPadding: 10
				leftPadding: 10

				Label {
					text: qsTr("Name: <b>") + mesoName + "</b>"
				}
				Label {
					text: qsTr("Start of mesocycle: <b>") + mesoStartDate.toDateString() + "</b>"
				}
				Label {
					text: qsTr("End of mesocycle: <b>") + mesoEndDate.toDateString() + "</b>"
				}
				Label {
					text: qsTr("Weeks in mesocycle: <b>") + nWeeks.toString() + "</b>"
				}
				Label {
					text: qsTr("Training Split: <b>") + mesoSplit + "</b>"
				}
			}

			Connections {
				target: mesoDelegate
				function onClicked() {
					currentMesoIndex = index;
				}
			}
		}
	}

footer: ToolBar {
		id: homePageToolBar
		width: parent.width

		ToolButton {
			id: btnAddMeso
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			anchors.rightMargin: 5
			text: qsTr("New Mesocycle")
			display: AbstractButton.TextUnderIcon
			font.capitalization: Font.MixedCase
			icon.source: "qrc:/images/"+darkIconFolder+"mesocycle-add.png"
			icon.height: 20
			icon.width: 20

			onClicked: {
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

				homePage.StackView.view.push("MesoCycle.qml",  {
						"mesosModel": mainMesosModel,
						"mesoId": -1,
						"mesoName": "Novo mesociclo",
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
	} //ToolButton
} // footer

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

		const weekOne = JSF.weekNumber(meso.mesoStartDate);
		const weekTwo = JSF.weekNumber(meso.mesoEndDate);

		homePage.StackView.view.push("MesoCycle.qml", {
			"mesosModel": mainMesosModel,
			"idxModel": currentMesoIndex,
			"mesoId": meso.mesoId,
			"mesoName": meso.mesoName,
			"mesoStartDate": meso.mesoStartDate,
			"mesoEndDate": meso.mesoEndDate,
			"mesoNote": meso.mesoNote,
			"nWeeks": meso.nWeeks,
			"mesoSplit": meso.mesoSplit,
			"mesoDrugs": meso.mesoDrugs,
			"minimumMesoStartDate": Database.getPreviousMesoEndDate(meso.mesoId),
			"maximumMesoEndDate": Database.getNextMesoStartDate(meso.mesoId),
			"week1": weekOne,
			"week2": weekTwo,
			"calendarStartDate": startDate
		});
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
} //Page
