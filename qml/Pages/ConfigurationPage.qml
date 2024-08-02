import QtQuick
import QtQuick.Controls

import ".."
import "../TPWidgets"

TPPage {
	id: configPage
	objectName: "configurationPage"

	property alias currentPage: splitView.currentItem
	property alias startPageIndex: splitView.currentIndex

	SwipeView {
		id: splitView
		objectName: "splitSwipeView"
		currentIndex: 0
		interactive: true
		height: parent.height
		anchors.fill: parent

		SettingsPage {
			id: settingsPage
		}
		UserPage {
			id: userPage
		}
	} //SwipeView

	PageIndicator {
		id: indicator
		count: splitView.count
		currentIndex: splitView.currentIndex
		height: 20

		delegate: Rectangle {
			implicitWidth: 12
			implicitHeight: 12
			radius: width/2
			color: AppSettings.fontColor

			opacity: index === indicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45

			required property int index

			Behavior on opacity {
				OpacityAnimator {
					duration: 200
				}
			}
		}

		anchors {
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}
	}

	footer: ToolBar {
		id: splitToolBar
		width: parent.width
		height: footerHeight

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
			id: btnApplyChanges
			text: qsTr("Apply")
			enabled: currentPage.bModified
			width: 80
			flat: false
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenter: parent.horizontalCenter

			onClicked: {
				currentPage.apply();
				if (AppSettings.firstTime)
				{
					AppSettings.firstTime = false;
					mainwindow.checkInitialArguments();
					mainwindow.bBackButtonEnabled = true;
				}
			}
		}
	}
}
