import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "HomePageElements"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPPage {
	id: homePage
	imageSource: ":/images/backgrounds/backimage-home.jpg"

	property bool loadOwnMesos: false
	property bool loadClientMesos: false
	property bool modelsLoaded: false
	property date minimumStartDate
	property MesocyclesModel mesoModel: null

	signal mesosViewChanged(bool own_mesos);

	header: TPToolBar {
		bottomPadding: 20
		height: headerHeight

		TPImage {
			id: imgAppIcon
			source: "app-icon"
			dropShadow: false
			width: appSettings.itemExtraLargeHeight
			height: width

			anchors {
				verticalCenter: lblMain.verticalCenter
				right: lblMain.left
			}
		}

		TPLabel {
			id: lblMain
			text: qsTr("Training Organizer")
			singleLine: true
			useBackground: true
			font: AppGlobals.extraLargeFont
			width: parent.width - imgAppIcon.width - 15
			height: parent.height

			anchors {
				verticalCenter: parent.verticalCenter
				verticalCenterOffset: (headerHeight - height)/2
				horizontalCenter: parent.horizontalCenter
				horizontalCenterOffset: imgAppIcon.width/2
			}
		}
	}

	SwipeView {
		id: mesosView
		currentIndex: userModel.mainUserConfigured ? (userModel.mainUserIsCoach ? 0 : 1) : -1
		interactive: userModel.mainUserIsCoach && userModel.mainUserIsClient
		anchors.fill: parent

		onCurrentIndexChanged: {
			if (modelsLoaded && currentIndex >= 0) {
				const own_meso = currentIndex === 1;
				homePage.colorLight = own_meso ? appSettings.primaryColor : appSettings.primaryDarkColor
				homePage.colorDark = own_meso ? appSettings.primaryLightColor : appSettings.primaryColor
				mesosViewChanged(own_meso);
			}
		}

		Loader {
			id: clientsMesosListLoader
			active: loadClientMesos
			asynchronous: true

			sourceComponent: MesosList {
				mesoSubModel: mesoModel.clientMesos
			}
		}

		Loader {
			id: ownMesosListLoader
			active: loadOwnMesos
			asynchronous: true

			sourceComponent: MesosList {
				mesoSubModel: mesoModel.ownMesos
			}
		}
	} //SwipeView

	PageIndicator {
		id: indicator
		count: mesosView.count
		currentIndex: mesosView.currentIndex
		visible: userModel.mainUserConfigured && (userModel.mainUserIsCoach && userModel.mainUserIsClient)

		delegate: Rectangle {
			width: appSettings.itemSmallHeight
			height: width
			radius: width/2
			opacity: index === indicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45
			color: index === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2

			Text {
				text: String(index + 1)
				color: appSettings.fontColor
				anchors.centerIn: parent
			}
		}

		anchors {
			bottom: parent.bottom
			bottomMargin: ownMesosListLoader.height * (Qt.platform.os !== "android" ? 0.22 : 0.28)
			horizontalCenter: mesosView.horizontalCenter
		}
	}

	function mesosViewIndex(): int {
		return mesosView.currentIndex;
	}

	function setMesosViewIndex(index: int) {
		mesosView.currentIndex = -1;
		modelsLoaded = true;
		mesosView.currentIndex = index;
	}
} //Page
