pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import TpQml
import TpQml.Widgets

import "./AllUsersElements"

TPPopup {
	id: dlgSwitchUser
	width: AppSettings.pageWidth - 20
	height: AppSettings.pageHeight / 2

	ColumnLayout {
		spacing: 10
		anchors.fill: parent
		anchors.topMargin: dlgSwitchUser.btnClose.height + 10

		 HorizontalHeaderView {
			 id: horizontalHeader
			 syncView: allUsersList
			 clip: true
		 }

		Row {
			spacing: 0
			padding: 0
			Layout.fillWidth: true
			Layout.preferredHeight: 0.7 * parent.height

			VerticalHeaderView {
				 id: verticalHeader
				 syncView: allUsersList
				 clip: true
			}

			TableView {
				id: allUsersList
				contentHeight: dlgSwitchUser.availableHeight
				contentWidth: rowWidth
				clip: true
				reuseItems: true
				boundsBehavior: Flickable.StopAtBounds
				selectionBehavior: TableView.SelectRows
				selectionMode: TableView.SingleSelection
				selectionModel: ItemSelectionModel {
					model: allUsersList.model
				}
				model: AppUserModel.allUsers
				width: parent.width * 0.9
				height: parent.height

				property int rowWidth: 0

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: allUsersList.contentHeight > allUsersList.height
				}
				ScrollBar.horizontal: ScrollBar {
					policy: ScrollBar.AlwaysOn
					active: true; visible: true
				}

				delegate: TableDelegate {
					userData: allData
					required property int row
					required property int column
					required property string allData
					Component.onCompleted: allUsersList.rowWidth += cellWidth;
				} //ItemDelegate
			} //allUsersList
		} //Row

		Row {
			id: buttonsRow
			spacing: 10
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.topMargin: -5
			Layout.leftMargin: 15

			readonly property int buttonSize: parent.width * 0.3 - 5

			TPButton {
				imageSource: "switch-user.png"
				text: qsTr("Switch")
				enabled: allUsersList.currentRow >= 0 ? AppUserModel.allUsers.userId !== AppUserModel.userId : false
				width: buttonsRow.buttonSize
				height: AppSettings.itemDefaultHeight

				onClicked: {
					AppUserModel.switchUser();
					dlgSwitchUser.close();
				}
			}

			TPButton {
				imageSource: "remove"
				text: qsTr("Remove")
				enabled: allUsersList.currentRow >= 0 ? AppUserModel.allUsers.userId !== AppUserModel.userId : false
				width: buttonsRow.buttonSize
				height: AppSettings.itemDefaultHeight

				onClicked: {
					AppUserModel.removeOtherUser();
					dlgSwitchUser.close();
				}
			}

			TPButton {
				imageSource: "add-new"
				text: qsTr("New user")
				width: buttonsRow.buttonSize
				height: AppSettings.itemDefaultHeight

				onClicked: {
					AppUserModel.createNewUser();
					dlgSwitchUser.close();
				}
			}
		}
	}//Layout
} //TPPopup
