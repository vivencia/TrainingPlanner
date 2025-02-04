import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
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

	Loader {
		id: mesosListLoader
		active: appSettings.mainUserConfigured
		asynchronous: true
		source: "qrc:/qml/Pages/HomePageElements/MesosList.qml"

		anchors {
			fill: parent
			margins: 10
		}
	}

	footer: Loader {
		active: appSettings.mainUserConfigured
		asynchronous: true
		source: "qrc:/qml/Pages/HomePageElements/Footer.qml"
	} // footer
} //Page

