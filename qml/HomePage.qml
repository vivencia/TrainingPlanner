// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
	id: homePage
	property date minimumStartDate;

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: AppSettings.primaryDarkColor
		opacity: 0.7
	}

	header: ToolBar {
		topPadding: 5
		bottomPadding: 20
		leftPadding: 10
		rightPadding: 10
		height: 60

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}


		RowLayout {
			anchors.fill: parent

			Image {
				source: "qrc:/images/"+AppSettings.iconFolder+"mesocycle.png"
				width: 50
				height: 50
				Layout.alignment: Qt.AlignCenter
			}

			Label {
				text: qsTr("Training Program")
				color: AppSettings.fontColor
				font.weight: Font.ExtraBold
				font.pointSize: AppSettings.fontSizeTitle
				wrapMode: Text.WordWrap
				Layout.alignment: Qt.AlignCenter
			}
		}
	}

	ListView {
		id: mesosListView
		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		anchors.topMargin: 40
		spacing: 10

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			active: ScrollBar.AsNeeded
		}

		delegate: SwipeDelegate {
			id: mesoDelegate
			width: ListView.view.width
			height: mesoContent.implicitHeight

			Rectangle {
				id: recRemoveMeso
				color: "red"
				opacity: 0.7
				radius: 6
				visible: false
				z: 1
				anchors {
					right: parent.right
					top: parent.top
					bottom: parent.bottom
				}

				Text {
					text: qsTr("Remove Mesocycle")
					font.pointSize: AppSettings.fontSize
					font.bold: true
					color: AppSettings.fontColor
					visible: parent.width > width
					anchors {
						bottom: parent.bottom
						horizontalCenter: parent.horizontalCenter
						bottomMargin: 10
					}
				}

				Image {
					source: "qrc:/images/"+AppSettings.iconFolder+"remove.png"
					height: 40
					width: 40
					visible: parent.width > 50
					anchors {
						top: parent.top
						horizontalCenter: parent.horizontalCenter
						topMargin: 10
					}
				}

				MouseArea {
					anchors.fill: parent
					onClicked: {
						msgDlg.show(parent.y + parent.height);
						recRemoveMeso.visible = false;
					}
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove Mesocycle?")
					message: qsTr("This action cannot be undone. Note: removing a Mesocycle does not remove the records of the days within it.")
					button1Text: qsTr("Yes")
					button2Text: qsTr("No")
					imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

					onButton1Clicked: appDB.removeMesocycle();
				}
			} //Rectangle recRemoveMeso

			MouseArea {
				id: swipeDetector
				anchors.fill: parent
				preventStealing: true
				pressAndHoldInterval: 300
				property int xPrev: 0
				property bool tracing: false

				onClicked: {
					if (!recRemoveMeso.visible)
						appDB.getMesocycle(index);
					else
						recRemoveMeso.visible = false;
				}

				onPressAndHold: (mouse) => {
					xPrev = mouse.x;
					if (!recRemoveMeso.visible) {
						if (xPrev >= width/3) {
							tracing = true;
							recRemoveMeso.width = width - xPrev;
							recRemoveMeso.visible = true;
						}
					}
				}

				onPositionChanged: (mouse) => {
					if (!tracing) return;
					if (mouse.x <= 0)
						recRemoveMeso.width = mesosListView.width;
					else {
						recRemoveMeso.width += (xPrev - mouse.x);
						if (mouse.x > xPrev) {
							if (recRemoveMeso.width <= 30)
								recRemoveMeso.visible = false;
						}
						xPrev = mouse.x;
					}
				}

				onReleased: (mouse) => {
					if (tracing)
						tracing = false;
				}
			} //MouseArea

			background: Rectangle {
				radius: 6
				opacity: 0.8
				color: index === mesocyclesModel.currentRow ? AppSettings.entrySelectedColor : listEntryColor2
			}

			Column {
				id: mesoContent
				topPadding: 10
				bottomPadding: 10
				leftPadding: 10

				Label {
					text: qsTr("Name: <b>") + mesoName + "</b>"
					color: AppSettings.fontColor
					width: mesoDelegate.width
					elide: Text.ElideRight
				}
				Label {
					text: realMeso ?
							qsTr("Start of mesocycle: <b>") + runCmd.formatDate(mesoStartDate) + "</b>" :
							qsTr("Program start date: <b>") + runCmd.formatDate(mesoStartDate) + "</b>"
					color: AppSettings.fontColor
				}
				Label {
					text: realMeso ?
							qsTr("End of mesocycle: <b>") + runCmd.formatDate(mesoEndDate) + "</b>" :
							qsTr("Open-ended program - no end date set")
					color: AppSettings.fontColor
				}
				Label {
					text: qsTr("Weeks in mesocycle: <b>") + mesoWeeks + "</b>"
					color: AppSettings.fontColor
					visible: realMeso
				}
				Label {
					text: qsTr("Training Split: <b>") + mesoSplit + "</b>"
					color: AppSettings.fontColor
				}
			}
		} //delegate
	} //ListView

	footer: ToolBar {
		id: homePageToolBar
		width: parent.width
		height: 55

		background: Rectangle {
			gradient: Gradient {
				orientation: Gradient.Horizontal
				GradientStop { position: 0.0; color: AppSettings.paneBackgroundColor; }
				GradientStop { position: 0.25; color: AppSettings.primaryLightColor; }
				GradientStop { position: 0.50; color: AppSettings.primaryColor; }
				GradientStop { position: 0.75; color: AppSettings.primaryDarkColor; }
			}
			opacity: 0.8
		}

		TPButton {
			id: btnAddOpenSchedule
			text: qsTr("New open-ended schedule")
			textUnderIcon: true
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"open-schedule.png"
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter
			anchors.leftMargin: 5

			onClicked: newAction(0);
		}

		TPButton {
			id: btnAddMeso
			text: qsTr("New Mesocycle")
			imageSource: "qrc:/images/"+AppSettings.iconFolder+"mesocycle-add.png"
			textUnderIcon: true
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter
			anchors.rightMargin: 5

			onClicked: newAction(1);
		}
	} // footer

	function newAction(opt) {
		function pushPageOntoStack(object, id)
		{
			if (id === 175) {
				appDB.getPage.disconnect(pushPageOntoStack);
				stackView.push(object);
			}
		}

		appDB.getPage.connect(pushPageOntoStack);
		appDB.createNewMesocycle(opt, opt === 1 ? qsTr("New Mesocycle") : qsTr("New Training Plan"));
	}

	function setViewModel() {
		mesosListView.model = mesocyclesModel;
	}
} //Page
