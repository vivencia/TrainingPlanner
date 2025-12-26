import QtQuick
import QtQuick.Controls

import ".."
import "../TPWidgets"

TPPage {
	id: configPage
	objectName: "configurationPage"

	property alias currentPage: splitView.currentItem
	property alias startPageIndex: splitView.currentIndex

	onPageDeActivated: userPage.whenPageDeActivated();

	SwipeView {
		id: splitView
		objectName: "splitSwipeView"
		currentIndex: 0
		interactive: true
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
			color: appSettings.fontColor

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
}
