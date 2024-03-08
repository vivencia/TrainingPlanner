// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Page {
	id: homePage
	property int currentMesoIndex: -1
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

	Image {
		anchors.fill: parent
		source: "qrc:/images/app_logo.png"
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		opacity: 0.6
	}
	background: Rectangle {
		color: primaryDarkColor
		opacity: 0.7
	}

	ListView {
		id: mesosListView
		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
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
						onClicked: msgDlg.show(parent.y + parent.height);
					}
				}

				TPBalloonTip {
					id: msgDlg
					title: qsTr("Remove Mesocycle?")
					message: qsTr("This action cannot be undone. Note: removing a Mesocycle does not remove the records of the days within it.")
					button1Text: qsTr("Yes")
					button2Text: qsTr("No")
					imageSource: "qrc:/images/"+darkIconFolder+"remove.png"

					onButton1Clicked: {
						mesocyclesModel.remove(mesoDelegate.index, 1);
						pageActivation();
					}

					onButton2Clicked: recRemoveMeso.visible = false;
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
					currentMesoIndex = index;
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
				color: listEntryColor2
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
							qsTr("Start of mesocycle: <b>") + runCmd.formatDate(mesoStartDate) + "</b>" :
							qsTr("Program start date: <b>") + runCmd.formatDate(mesoStartDate) + "</b>"
					color: "white"
				}
				Label {
					text: realMeso ?
							qsTr("End of mesocycle: <b>") + runCmd.formatDate(mesoEndDate) + "</b>" :
							qsTr("Open-ended program - no end date set")
					color: "white"
				}
				Label {
					text: qsTr("Weeks in mesocycle: <b>") + mesoWeeks + "</b>"
					color: "white"
					visible: realMeso
				}
				Label {
					text: qsTr("Training Split: <b>") + mesoSplit + "</b>"
					color: "white"
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

	Component.onCompleted: {
		homePage.StackView.activating.connect(pageActivation);
		homePage.StackView.onDeactivating.connect(pageDeActivation);
	}

	function newAction(opt) {
		function pushPageOntoStack(object, id)
		{
			appDB.getItem.disconnect(pushPageOntoStack);
			appStackView.push(object, StackView.DontLoad);
		}

		appDB.getItem.connect(pushPageOntoStack);
		appDB.createNewMesocycle(opt, opt === 1 ? qsTr("New Mesocycle") : qsTr("New Training Plan"));
	}

	function showMeso() {
		function pushPageOntoStack(object, id)
		{
			appDB.getItem.disconnect(pushPageOntoStack);
			appStackView.push(object, StackView.DontLoad);
		}

		appDB.getItem.connect(pushPageOntoStack);
		appDB.getMesocycle(currentMesoIndex);
	}

	function pageActivation() {
		mesosListView.model = mesocyclesModel;
		if (mesocyclesModel.count === 0) {
			createFirstTimeTipComponent();
			firstTimeTip.y = homePageToolBar.y;
			firstTimeTip.x = (homePage.width-firstTimeTip.width)/2;
			firstTimeTip.visible = true;
			bFirstTime = true;
		}

		//mesocyclesModel.count === 0 is a first iteration of tips
		//showTip is a second iteration of tips. So it should be true under this condition and false if the first iteration condition is true
		const showTip = false; // mesocyclesModel.count !== 0 ? !Database.isTrainingDayTableEmpty(mesocyclesModel.mesoId) : false;

		if (mesocyclesModel.count === 0 || showTip) {
			if (firstTimeTip) {
				firstTimeTip.message = qsTr("Start here");
				firstTimeTip.visible = true;
			}
			else
				createFirstTimeTipComponent();

			if (mesocyclesModel.count === 0) {
				firstTimeTip.y = homePageToolBar.y;
				firstTimeTip.x = (homePage.width-firstTimeTip.width)/2;
				firstTimeTip.visible = true;
			}
			else {
				if (showTip) {
					firstTimeTip.y = homePageToolBar.y + homePageToolBar.height;
					firstTimeTip.x = homePage.width-firstTimeTip.width;
					firstTimeTip.visible = true;
				}
			}
		}
	}

	function pageDeActivation() {
		if (firstTimeTip)
			firstTimeTip.visible = false;
	}

	function createFirstTimeTipComponent() {
		var component = Qt.createComponent("FirstTimeHomePageTip.qml");
		if (component.status === Component.Ready) {
			firstTimeTip = component.createObject(homePage, { message:qsTr("Start either here or here"), showTwoImages: true });
		}
	}
} //Page
