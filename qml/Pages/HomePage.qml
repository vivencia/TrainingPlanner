import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "HomePageElements"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: homePage

	property date minimumStartDate;

	header: TPToolBar {
		bottomPadding: 20
		height: headerHeight

		TPImage {
			id: imgAppIcon
			source: "app_icon"
			dropShadow: false
			width: 50
			height: 50

			anchors {
				top: parent.top
				left: parent.left
				leftMargin: 5
			}
		}

		TPLabel {
			text: qsTr("Training Organizer")
			font: AppGlobals.extraLargeFont
			width: parent.width - imgAppIcon.width - 15
			height: parent.height

			anchors {
				top: parent.top
				topMargin: 5
				left: imgAppIcon.right
				leftMargin: 5
				right: parent.right
				rightMargin: 10
			}
		}
	}

	SwipeView {
		id: mesoView
		currentIndex: userModel.mainUserConfigured ? (userModel.isClient(0) ? 0 : 1) : -1
		interactive: userModel.isCoach(0) && userModel.isClient(0)
		anchors.fill: parent

		onCurrentIndexChanged: {
			homePage.colorLight = currentIndex === 0 ? appSettings.primaryDarkColor : appSettings.primaryColor
			homePage.colorDark = currentIndex === 0 ? appSettings.primaryColor : appSettings.primaryLightColor
		}

		readonly property list<string> pageTitle: [qsTr("My Programs"), qsTr("Clients' Programs")]

		Loader {
			id: ownMesosListLoader
			active: userModel.mainUserConfigured && userModel.isClient(0)
			asynchronous: true

			sourceComponent: MesosList {
				mesoModel: mesocyclesModel.ownMesos;
				mainUserPrograms: true;
			}
		}

		Loader {
			id: clientsMesosListLoader
			active: userModel.mainUserConfigured && userModel.isCoach(0)
			asynchronous: true

			sourceComponent: MesosList {
				mesoModel: mesocyclesModel.clientMesos;
				mainUserPrograms: false
			}
		}
	} //SwipeView

	PageIndicator {
		id: indicator
		count: mesoView.count
		currentIndex: mesoView.currentIndex
		visible: userModel.mainUserConfigured && (userModel.isCoach(0) && userModel.isClient(0))
		height: 25
		width: parent.width

		delegate: TPLabel {
			text: mesoView.pageTitle[index]
			width: parent.width/2
			elide: Text.ElideMiddle
			horizontalAlignment: Text.AlignHCenter

			required property int index

			background: Rectangle {
				radius: width/2
				opacity: index === indicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45
				color: index === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2
			}

			Behavior on opacity {
				OpacityAnimator {
					duration: 200
				}
			}
		}

		anchors {
			bottom: parent.bottom
			bottomMargin: ownMesosListLoader.height * (Qt.platform.os !== "android" ? 0.22 : 0.28)
			horizontalCenter: parent.horizontalCenter
		}
	}
} //Page

