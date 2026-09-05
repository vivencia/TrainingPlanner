pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets
import TpQml.Pages

TPPage {
	id: homePage
	objectName: "homePage"
	imageSource: ":/images/backgrounds/backimage-home.jpg"

	property bool loadMesosFromCoaches: false
	property bool loadMesosForSelf: false
	property bool loadMesosForClients: false
	property MesocyclesModel mesoModel: null

	header: TPToolBar {
		bottomPadding: 20
		height: homePage.headerHeight

		TPImage {
			id: imgAppIcon
			source: "app-icon"
			dropShadow: false
			width: AppSettings.itemExtraLargeHeight
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
				verticalCenterOffset: (homePage.headerHeight - height)/2
				horizontalCenter: parent.horizontalCenter
				horizontalCenterOffset: imgAppIcon.width/2
			}
		}
	}

	TPSwipeView {
		id: mesosView
		currentIndex: homePage.mesoModel.currentMesosView()
		interactive: AppUserModel.mainUserIsCoach && AppUserModel.mainUserIsClient
		indicatorsColors: [homePage.mesoModel.homePageViewModelViaIndex(0).backgroundColor,
										homePage.mesoModel.homePageViewModelViaIndex(1).backgroundColor,
										homePage.mesoModel.homePageViewModelViaIndex(2).backgroundColor]
		indicatorBottomMargin: height * (Qt.platform.os !== "android" ? 0.2 : 0.25)
		anchors.fill: parent
		onCurrentIndexChanged: homePage.mesoModel.setCurrentMesosView(currentIndex);

		Loader {
			id: mesosFromCoachesLoader
			active: homePage.loadMesosFromCoaches
			asynchronous: true

			property int index

			sourceComponent: MesosList {
				mesoSubModel: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FROM_COACH)
				index: mesosFromCoachesLoader.index
			}
		}

		Loader {
			id: mesosForSelfLoader
			active: homePage.loadMesosForSelf
			asynchronous: true

			property int index

			sourceComponent: MesosList {
				mesoSubModel: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FOR_SELF)
				index: mesosForSelfLoader.index
			}
		}

		Loader {
			id: mesosForClientsLoader
			active: homePage.loadMesosForClients
			asynchronous: true

			property int index

			sourceComponent: MesosList {
				mesoSubModel: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FOR_CLIENT)
				index: mesosForClientsLoader.index
			}
		}
	} //TPSwipeView
} //Page
