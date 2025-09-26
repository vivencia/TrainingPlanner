import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.vivenciasoftware.TrainingPlanner.qmlcomponents

import "../TPWidgets"

TPPopup {
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
				//reuseItems: true
				clip: true
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
					for (let i = 0; i < model.count; ++i) {
						itemAtIndex(index(currentRow, i)).selected = true;
						if (cur_row >= 0)
							itemAtIndex(index(cur_row, i)).selected = false;
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
					property bool selected: false

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

		TPButton {
			id: btnSwitchToUser
			imageSource: "switch_user.png"
			text: qsTr("Switch")
			autoSize: true
			enabled: allUsersList.currentRow >= 0 ? userModel.allUsers.userId !== userModel.userId : false
			Layout.alignment: Qt.AlignCenter
			Layout.minimumWidth: preferredWidth
			Layout.topMargin: -5

			onClicked: userModel.switchUser();
		}
	}//Layout
} //TPPopup
