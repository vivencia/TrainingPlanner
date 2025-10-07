import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../TPWidgets"

TPPopup {
	id: dlgSwitchUser
	width: appSettings.pageWidth - 20
	height: appSettings.pageHeight / 2

	ColumnLayout {
		spacing: 10
		anchors.fill: parent
		anchors.topMargin: btnClose.height + 10

		 HorizontalHeaderView {
			 id: horizontalHeader
			 syncView: allUsersList
			 clip: true
		 }

		Row
		{
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
				contentHeight: availableHeight
				contentWidth: rowWidth
				clip: true
				reuseItems: true
				boundsBehavior: Flickable.StopAtBounds
				selectionBehavior: TableView.SelectRows
				selectionMode: TableView.SingleSelection
				selectionModel: ItemSelectionModel {
					model: allUsersList.model
				}
				model: userModel.allUsers
				width: parent.width * 0.9
				height: parent.height

				property int rowWidth: 0
				property int cur_row: -1

				ScrollBar.vertical: ScrollBar {
					policy: ScrollBar.AsNeeded
					active: true; visible: allUsersList.contentHeight > allUsersList.height
				}
				ScrollBar.horizontal: ScrollBar {
					policy: ScrollBar.AlwaysOn
					active: true; visible: true
				}

				onCurrentRowChanged: {
					let _item = itemAtIndex(index(currentRow, 0));
					if (_item)
						_item.setSelected(currentRow, true);
					if (cur_row >= 0) {
						_item = itemAtIndex(index(cur_row, 0));
						if (_item)
							_item.setSelected(cur_row, false);
					}
					cur_row = currentRow;
					model.currentRow = currentRow;
				}

				delegate: Rectangle {
					id: delegate
					border.width: current ? 2 : 1
					color: selected ? appSettings.entrySelectedColor :
						(row % 2 === 0 ? appSettings.listEntryColor1 : appSettings.listEntryColor2)
					implicitWidth: lblData.contentWidth > 0 ? lblData.contentWidth * 1.2 : 25
					implicitHeight: appSettings.itemDefaultHeight

					required property bool current

					function setSelected(_row: int, _selected: bool): void {
						userModel.allUsers.setSelected(_row, _selected);
					}

					TPLabel {
						id: lblData
						text: allData
						leftPadding: 5
						bottomPadding: 2

						Component.onCompleted: {
							if (row === 0)
								allUsersList.rowWidth += contentWidth * 1.2;
						}
					}
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
				imageSource: "switch_user.png"
				text: qsTr("Switch")
				enabled: allUsersList.currentRow >= 0 ? userModel.allUsers.userId !== userModel.userId : false
				width: buttonsRow.buttonSize
				height: appSettings.itemDefaultHeight

				onClicked: {
					userModel.switchUser();
					dlgSwitchUser.close();
				}
			}

			TPButton {
				imageSource: "remove"
				text: qsTr("Remove")
				enabled: allUsersList.currentRow >= 0 ? userModel.allUsers.userId !== userModel.userId : false
				width: buttonsRow.buttonSize
				height: appSettings.itemDefaultHeight

				onClicked: {
					userModel.removeOtherUser();
					dlgSwitchUser.close();
				}
			}

			TPButton {
				imageSource: "add-new"
				text: qsTr("New user")
				width: buttonsRow.buttonSize
				height: appSettings.itemDefaultHeight

				onClicked: {
					userModel.createNewUser();
					dlgSwitchUser.close();
				}
			}
		}
	}//Layout
} //TPPopup
