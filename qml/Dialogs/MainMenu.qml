import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../"
import "../TPWidgets"
import "../User"

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

Drawer {
	id: mainMenu
	height: mainwindow.height
	implicitWidth: mainwindow.width * 0.7
	spacing: 0
	padding: 0
	edge: Qt.RightEdge

	required property Page rootPage

	Connections {
		target: userModel
		function onUserModified(row: int, field: int): void {
			if (row === 0) {
				switch (field) {
					case 1: lblAvatar.text = userModel.userName(0); break;
					case 20: imgAvatar.source = userModel.avatar(0, false); break;
					default: break;
				}
			}
		}
	}

	background: Rectangle {
		id: backgrundRec
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: userSettings.paneBackgroundColor; }
			GradientStop { position: 0.25; color: userSettings.primaryDarkColor; }
			GradientStop { position: 0.50; color: userSettings.primaryLightColor; }
			GradientStop { position: 0.75; color: userSettings.primaryColor; }
		}
		opacity: 0.8
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === mainwindow.backKey) {
				event.accepted = true;
				close();
			}
		}
	}

	ColumnLayout {
		id: drawerLayout
		spacing: 5
		opacity: parent.opacity
		height: mainMenu.height * 0.65

		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			leftMargin: 5
			rightMargin: 5
			topMargin: 0
			bottomMargin: 5
		}

		TPImage {
			id: imgLogo
			source: "app_icon"
			dropShadow: false
			width: parent.height * 0.25
			height: width
			Layout.alignment: Qt.AlignCenter
			Layout.preferredWidth: width
			Layout.preferredHeight: height

			TPImage {
				id: imgOnline
				source: osInfo.tpServerOK ? "online" : "offline"
				width: userSettings.itemLargeHeight
				height: width
				visible: userModel.onlineUser

				anchors {
					top: parent.top
					horizontalCenter: btnSettings.horizontalCenter
				}

				ToolTip {
					id: onlineInfo
				}

				MouseArea {
					hoverEnabled: true
					anchors.fill: parent
					onEntered: onlineInfo.show(osInfo.connectionMessage, -1);
					onExited: onlineInfo.hide();
				}
			}

			TPButton {
				id: btnSettings
				imageSource: "settings"
				width: userSettings.itemExtraLargeHeight
				height: width
				enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
					stackView.currentItem
					!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
				}

				onClicked: {
					itemManager.getSettingsPage(0);
					close();
				}

				anchors {
					top: userModel.onlineUser ? imgOnline.bottom : parent.top
					left: parent.left
					leftMargin: - (mainMenu.width - parent.width) / 2
				}
			}
		}

		TPLabel {
			text: "TrainingPlanner by VivenciaSoftware - " + appSettings.appVersion
			wrapMode: Text.WordWrap
			font: AppGlobals.smallFont
			horizontalAlignment: Text.AlignHCenter
			Layout.maximumWidth: parent.width - 20
			Layout.leftMargin: 5
			Layout.rightMargin: 30
		}

		TPImage {
			id: imgAvatar
			dropShadow: true
			source: userModel.avatar(0)
			width: parent.height * 0.25
			height: width
			Layout.alignment: Qt.AlignCenter
			Layout.preferredWidth: width
			Layout.preferredHeight: height

			MouseArea {
				anchors.fill: parent
				enabled: { // Force the binding to re-evaluate so that the check is run each time the page changes.
					stackView.currentItem
					!stackView.find((item, index) => { return item.objectName === "configurationPage"; })
				}

				onClicked: {
					itemManager.getSettingsPage(1);
					close();
				}
			}

			TPButton {
				imageSource: "switch_user.png"
				width: userSettings.itemDefaultHeight
				height: width
				visible: { return Qt.platform.os !== "android"}

				onClicked: showSwitchUserDialog();

				anchors {
					left: parent.right
					bottom: parent.bottom
				}
			}
		}

		TPLabel {
			id: lblAvatar
			elide: Text.ElideMiddle
			text: userModel.userName(0);
			horizontalAlignment: Text.AlignHCenter
			width: parent.width
			Layout.fillWidth: true
			Layout.leftMargin: 10
			Layout.rightMargin: 5
		}

		Rectangle {
			color: userSettings.fontColor
			height: 3
			Layout.fillWidth: true
		}

		ListView {
			id: pagesList
			model: pagesListModel
			clip: true
			spacing: 2
			boundsBehavior: Flickable.StopAtBounds
			contentHeight: availableHeight
			contentWidth: availableWidth
			Layout.fillWidth: true
			Layout.minimumHeight: mainMenu.height * 0.35

			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AsNeeded
				active: true
			}

			delegate: SwipeDelegate {
				id: delegate
				width: pagesList.width
				height: userSettings.itemLargeHeight

				contentItem: TPLabel {
					id: listItem
					text: displayText
					elide: Text.ElideMiddle
					wrapMode: Text.NoWrap
					horizontalAlignment: Text.AlignHCenter
				}

				background: Rectangle {
					id:	backgroundColor
					color: userSettings.primaryDarkColor
					radius: 6
					opacity: 1
				}

				onClicked: pagesListModel.openMainMenuShortCut(index);

				swipe.right: Rectangle {
					width: parent.width
					height: parent.height
					clip: false
					color: SwipeDelegate.pressed ? "#555" : "#666"
					radius: 5

					TPImage {
						source: "close.png"
						width: userSettings.itemDefaultHeight
						height: width
						opacity: 2 * -delegate.swipe.position

						anchors {
							right: parent.right
							rightMargin: 20
							verticalCenter: parent.verticalCenter
						}
					}
				} //swipe.right

				swipe.onCompleted: pagesListModel.closePage(index);
			} //delegate: SwipeDelegate
		} //ListView
	} //ColumnLayout

	TPButton {
		id: btnExit
		text: qsTr("Exit")
		imageSource: "application-exit.png"
		iconOnTheLeft: true
		rounded: false

		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
		}

		onClicked: itemManager.exitApp();
	}

	property AllUsers allUsersDialog: null
	function showSwitchUserDialog(): void {
		userModel.getAllOnlineUsers();

		if (allUsersDialog === null) {
			function createDialog() {
				let component = Qt.createComponent("qrc:/qml/User/AllUsers.qml", Qt.Asynchronous);

				function finishCreation() {
					allUsersDialog = component.createObject(contentItem, { parentPage: mainMenu.rootPage });
				}

				if (component.status === Component.Ready)
					finishCreation();
				else
					component.statusChanged.connect(finishCreation);
			}
			createDialog();
		}
		allUsersDialog.show1(-1);
	}
} //Drawer
