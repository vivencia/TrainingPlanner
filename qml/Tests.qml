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
		imageSource: ":/images/backgrounds/backimage-home.jpg"
		anchors.fill: parent
		signal mesosViewChanged(bool own_mesos);

		property MesoManager mesoManager: null

		Connections {
			target: ItemManager
			function onCppDataForQMLReady() : void {
			}
		}

		TPFileViewer {
			id: viewer
			property alias url: fileops.fileName
			anchors.centerIn: parent
			fileOps: FileOperations {
				id: fileops
				parentPage: homePage
				//fileName: "/home/guilhermef/.local/share/Vivencia Software/TrainingPlanner/1759256421787/1759170252407/mesocycles/Hipertrofia 1.txt"
				//fileName: "/home/guilhermef/.local/share/Vivencia Software/TrainingPlanner/1759170252407/1759256421787/mesocycles/Hipertrofia 1.pdf"
				fileName: "user.ini"
				useControls: true
				canAddFile: false
				canDownloadOrGenerate: true
			}
		}

		TPButton {
			anchors.bottom: parent.bottom
			anchors.horizontalCenter: parent.horizontalCenter
			text: "Set Filename"
			onClicked: viewer.url = "/home/guilhermef/.local/share/Vivencia Software/TrainingPlanner/1759256421787/1759170252407/mesocycles/Hipertrofia 1.txt";
		}
	}
}
