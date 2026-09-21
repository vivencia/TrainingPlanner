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

		TPPane {
			id: pane
			maxHeight: 300
			width: 200
			header: "Test a really long header"
			icon: "messages"
			customHeaderWidget: TPRadioButtonOrCheckBox {
				objectName: "checkbox"
				text: pane.header
				multiLine: true
				boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
				width: pane.headerWidth
			}
			x: 0
			y: 150
			ColumnLayout {
				width: pane.width
				height: 5 * (100 + spacing)
				anchors { top: pane.headerWidget.bottom; left: parent.left; topMargin: 10 }

				Repeater {
					model: 5
					Layout.fillHeight: true
					delegate: Rectangle {
						id: delegate
						required property int index
						height: 100
						width: parent.width
						border.color: "white"
						color: {
							switch (delegate.index) {
							case 0: return "blue";
							case 1: return "green";
							case 2: return "red";
							case 3: return "yellow";
							case 4: return "orange";
							}
						}
					}
				}
			}
		}

		TPButton {
			id: btnLogs
			text: "Show logs"
			onClicked: ItemManager.showLogs();
			anchors {
				top: pane.bottom
				horizontalCenter: parent.horizontalCenter
			}
		}
		TPButton {
			id: btnMessage
			text: "New Message"
			onClicked: ItemManager.displayWindowMessage(0, "Custom Title", "Custom Message");
			anchors {
				top: btnLogs.bottom
				horizontalCenter: parent.horizontalCenter
			}
		}
	}
}
