pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Widgets
import TpQml.Pages

TPPage {
	id: homePage
	objectName: "homePage"
	imageSource: ":/images/backgrounds/backimage-home.jpg"

//public:
	property MesocyclesModel mesoModel

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
		indicatorBottomMargin: height * (Qt.platform.os !== "android" ? 0.2 : 0.25)
		anchors.fill: parent
		onCurrentIndexChanged: homePage.mesoModel.setCurrentMesosView(currentIndex);

		Loader {
			id: mesosFromCoachesLoader
			active: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FROM_COACH) !== null
			asynchronous: true

			sourceComponent: MesosList {
				mesoFilteredModel: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FROM_COACH)
				Component.onCompleted: mesosView.indicatorsColors.push(mesoFilteredModel.backgroundColor);
			}
		}

		Loader {
			id: mesosForSelfLoader
			active: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FOR_SELF) !== null
			asynchronous: true

			sourceComponent: MesosList {
				mesoFilteredModel: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FOR_SELF)
				Component.onCompleted: mesosView.indicatorsColors.push(mesoFilteredModel.backgroundColor);
			}
		}

		Loader {
			id: mesosForClientsLoader
			active: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FOR_CLIENT) !== null
			asynchronous: true

			sourceComponent: MesosList {
				mesoFilteredModel: homePage.mesoModel.homePageViewModel(MesocyclesModel.MT_MESO_FOR_CLIENT)
				Component.onCompleted: mesosView.indicatorsColors.push(mesoFilteredModel.backgroundColor);
			}
		}
	} //TPSwipeView
} //Page
