pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets
import TpQml.User
import TpQml.Pages

Drawer {
	id: _mainMenu
	height: AppSettings.windowHeight
	implicitWidth: AppSettings.windowWidth * 0.7
	spacing: 0
	padding: 0
	edge: Qt.RightEdge

	Connections {
		target: AppUserModel
		function onUserModified(row: int, field: int): void {
			if (row === 0) {
				switch (field) {
				case 1: lblAvatar.text = AppUserModel.userName(0); break;
				case 20: imgAvatar.source = AppUserModel.avatar(0, false); break;
				default: return;
				}
			}
		}
		function onUserIdChanged(): void {
			imgAvatar.source = AppUserModel.avatar(0);
			lblAvatar.text = AppUserModel.userName(0);
		}
	}

	Connections {
		target: ItemManager.AppPagesManager
		function onAppSettingsButtonEnabled(enabled: string) : void {
			btnSettings.enabled = enabled;
		}
		function onUserSettingsButtonEnabled(enabled: string) : void {
			imgAvatar.enabled = enabled;
		}
	}

	background: TPBackRec {
		useGradient: true
		opacity: 0.8
	}

	contentItem {
		Keys.onPressed: (event) => {
			if (event.key === ItemManager.AppPagesManager.backKey()) {
				event.accepted = true;
				_mainMenu.close();
			}
		}
	}

	ColumnLayout {
		id: drawerLayout
		spacing: 5
		opacity: _mainMenu.opacity
		height: _mainMenu.height * 0.65

		anchors {
			left: _mainMenu.contentItem.left
			right: _mainMenu.contentItem.right
			top: _mainMenu.contentItem.top
			leftMargin: 5
			rightMargin: 5
			topMargin: 0
			bottomMargin: 5
		}

		TPImage {
			id: imgLogo
			source: "app-icon"
			dropShadow: false
			Layout.alignment: Qt.AlignCenter
			Layout.preferredWidth: _mainMenu.height * 0.25
			Layout.preferredHeight: _mainMenu.height * 0.25

			TPImage {
				id: imgOnline
				source: AppUserModel.canConnectToServer ? "online" : "offline"
				width: AppSettings.itemLargeHeight
				height: width
				visible: AppUserModel.onlineAccount

				anchors {
					top: imgLogo.top
					horizontalCenter: btnSettings.horizontalCenter
				}

				ToolTip {
					id: onlineInfo
				}

				MouseArea {
					hoverEnabled: true
					anchors.fill: imgOnline
					onEntered: onlineInfo.show(AppOsInterface.connectionMessage, -1);
					onExited: onlineInfo.hide();
				}
			}

			TPButton {
				id: btnSettings
				imageSource: "settings"
				width: AppSettings.itemExtraLargeHeight
				height: width

				onClicked: {
					ItemManager.getSettingsPage();
					_mainMenu.close();
				}

				anchors {
					top: AppUserModel.onlineAccount ? imgOnline.bottom : imgLogo.top
					left: imgLogo.left
					leftMargin: - (_mainMenu.width - imgLogo.width) / 2
				}
			}

			TPButton {
				id: btnMessages
				imageSource: "messages"
				visible: AppUserModel.onlineAccount
				checkable: true
				checked: !AppSettings.showOnlineMessagesDialog
				width: AppSettings.itemLargeHeight
				height: width

				onCheck: ItemManager.showOnlineMessagesManagerDialog(!checked);

				anchors {
					top: btnSettings.bottom
					horizontalCenter: btnSettings.horizontalCenter
				}
			}
		}

		TPLabel {
			text: "TrainingPlanner by VivenciaSoftware - " + AppSettings.appVersion
			wrapMode: Text.WordWrap
			font: AppGlobals.smallFont
			horizontalAlignment: Text.AlignHCenter
			Layout.maximumWidth: parent.width - 20
			Layout.leftMargin: 5
			Layout.rightMargin: 30
		}

		TPImage {
			id: imgAvatar
			source: AppUserModel.avatar(0)
			dropShadow: true
			keepAspectRatio: true
			imageSizeFollowControlSize: true
			fullWindowView: false
			Layout.alignment: Qt.AlignCenter
			Layout.preferredWidth: parent.height * 0.25
			Layout.preferredHeight: parent.height * 0.25

			MouseArea {
				anchors.fill: imgAvatar

				onClicked: {
					ItemManager.getUserPage();
					_mainMenu.close();
				}
			}

			TPButton {
				imageSource: "switch-user.png"
				width: AppSettings.itemDefaultHeight
				height: AppSettings.itemDefaultHeight
				visible: { return Qt.platform.os !== "android"}

				onClicked: _mainMenu.showSwitchUserDialog();

				anchors {
					left: imgAvatar.right
					leftMargin: 10
					bottom: imgAvatar.bottom
				}
			}
		}

		TPLabel {
			id: lblAvatar
			elide: Text.ElideMiddle
			text: AppUserModel.userName(0);
			horizontalAlignment: Text.AlignHCenter
			Layout.preferredWidth: parent.width
			Layout.alignment: Qt.AlignHCenter
		}

		Rectangle {
			color: AppSettings.fontColor
			Layout.preferredHeight: 3
			Layout.fillWidth: true
		}

		TPListView {
			id: pagesList
			model: ItemManager.AppPagesManager
			Layout.fillWidth: true
			Layout.minimumHeight: _mainMenu.height * 0.35

			delegate: SwipeDelegate {
				id: delegate
				enabled: buttonEnabled
				width: pagesList.width
				height: AppSettings.itemLargeHeight

				required property int index
				required property string displayText
				required property string buttonEnabled

				contentItem: TPLabel {
					text: delegate.displayText
					elide: Text.ElideMiddle
					wrapMode: Text.NoWrap
					horizontalAlignment: Text.AlignHCenter
				}

				background: Rectangle {
					id:	backgroundColor
					color: AppSettings.primaryDarkColor
					radius: 6
					opacity: 1
				}

				onClicked: ItemManager.AppPagesManager.openMainMenuShortCut(index);

				swipe.right: Rectangle {
					width: parent.width
					height: parent.height
					clip: false
					color: SwipeDelegate.pressed ? "#555" : "#666"
					radius: 5

					TPImage {
						source: "close.png"
						width: AppSettings.itemDefaultHeight
						height: width
						opacity: 2 * -delegate.swipe.position

						anchors {
							right: parent.right
							rightMargin: 20
							verticalCenter: parent.verticalCenter
						}
					}
				} //swipe.right

				swipe.onCompleted: ItemManager.AppPagesManager.closePage(index);
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
			left: _mainMenu.contentItem.left
			right: _mainMenu.contentItem.right
			bottom: _mainMenu.contentItem.bottom
		}

		onClicked: ItemManager.exitApp();
	}

	Loader {
		id: allUsersDialogLoader
		asynchronous: true
		active: false

		property AllUsers _all_users
		sourceComponent: AllUsers {
			parentPage: ItemManager.AppPagesManager.homePage() as TPPage
			onClosed: allUsersDialogLoader.active = false;
			Component.onCompleted: allUsersDialogLoader._all_users = this;
		}

		onLoaded: _all_users.showInWindow(-Qt.AlignCenter);
	}
	function showSwitchUserDialog(): void {
		allUsersDialogLoader.active = true;
		AppUserModel.getAllOnlineUsers();
	}
} //Drawer
