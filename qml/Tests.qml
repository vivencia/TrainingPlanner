pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Pdf
import QtQuick.Layouts

import TpQml
import TpQml.Pages
import TpQml.Widgets
import TpQml.Exercises
import TpQml.Dialogs
import TpQml.User

ApplicationWindow {
	id: mainwindow
	visible: true
	title: "TraininPlanner Tests"
	objectName: "mainwindow"
	width: AppSettings.windowWidth
	height: AppSettings.windowHeight
	flags: Qt.platform.os === "android" ? Qt.Window | Qt.FramelessWindowHint : Qt.Window | Qt.CustomizeWindowHint & ~Qt.WindowMaximizeButtonHint

	signal fileDialogClosed(filepath: string);
	signal tpFileOpenInquiryResult(do_import: bool);

	TPPage {
		id: homePage
		objectName: "homePage"

		anchors.fill: parent
		signal mesosViewChanged(bool own_mesos);

		property MesoManager mesoManager: null

		Connections {
			target: ItemManager
			function onCppDataForQMLReady() : void {
			}
		}

		TPButton {
			id: btnLogs
			text: "Show logs"
			onClicked: ItemManager.showLogs();
			anchors {
				bottom: parent.bottom
				horizontalCenter: parent.horizontalCenter
			}
		}

		TPComboBox {
			id: cbo1
			specialIndex: 0
			currentIndex: -1
			model: ["Special Item", "--", "Item1", "Item2", "Item3", "--", "Item4", "Item5", "Item6"]
			width: parent.width * 0.8
			onItemActivated: (real_index, index, value) => console.log("Activated: ", real_index, index, value);
			anchors {
				verticalCenter: parent.verticalCenter
				horizontalCenter: parent.horizontalCenter
			}
		}

		TPComboBox {
			id: cbo2
			specialIndex: 0
			currentIndex: -1
			model: ItemManager.debugLog.fieldsNames

			width: parent.width * 0.8
			onItemActivated: (real_index, index, value) => console.log("Activated: ", real_index, index, value);
			anchors {
				top: cbo1.bottom
				topMargin: 20
				horizontalCenter: parent.horizontalCenter
			}
		}
	}
}
