pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

TPPane {
	id: _control

	required property TPLogs logsModel

	TPListView {
		id: listView
		listModel: _control.logsModel
		height: Math.min(_control.maxHeight, contentHeight)

		anchors {
			top: _control.headerWidget.bottom;
			left: parent.left
			right: parent.right
			topMargin: 10
			margins: 5
		}

		delegate: TPListViewDelegate {
			id: delegate
			_index: index
			_itemVisible: itemVisible
			tpListView: listView

			required property int index
			required property bool itemVisible
			required property bool selected
			required property string logTitle
			required property string logMessage
			required property string logTime
			required property string logTooltip

			itemContent: Item {
				id: logEntryItem
				height: lblTitle.height + lblMessage.height + 10

				ToolTip {
					id: logMessageTip
					closePolicy: Popup.CloseOnPressOutside
					z: 2
				}

				Item {
					id: buttonsRec
					width: AppSettings.itemDefaultHeight

					anchors {
						top: parent.top
						right: parent.right
						bottom: parent.bottom
						margins: 2
						rightMargin: -10
					}

					TPButton {
						id: btnInfo
						image: "info_"
						imageHeight: AppSettings.itemSmallHeight
						width: imageHeight
						height: imageHeight
						onClicked: logMessageTip.show(delegate.logTooltip, 5000)
						anchors {
							horizontalCenter: parent.horizontalCenter
							top: parent.top
						}
					}
					TPButton {
						id: btnCopy
						image: "copy_"
						imageHeight: AppSettings.itemSmallHeight
						width: imageHeight
						height: imageHeight
						onClicked: _control.logsModel.copyLog(delegate.index)
						anchors {
							horizontalCenter: parent.horizontalCenter
							bottom: parent.bottom
						}
					}
				}

				TPRadioButtonOrCheckBox {
					id: lblTitle
					text: delegate.logTitle + "  [" + delegate.logTime + "]"
					elideMode: Text.ElideLeft
					boxType: listView.items_selectable ? TPRadioButtonOrCheckBox.TP_CHECKBOX : TPRadioButtonOrCheckBox.TP_NONEBOX
					indicatorPos: Qt.AlignRight | Qt.AlignVCenter
					enableCheckOutsideIndicator: false
					font.weight: Font.Bold
					font.pixelSize: AppSettings.fontSize
					onChecked: (check) => _control.logsModel.setIsSelected(delegate.index, check);

					anchors {
						top: parent.top
						left: parent.left
						right: buttonsRec.left
						margins: -5
					}
				}
				TPLabel {
					id: lblMessage
					text: delegate.logMessage
					horizontalAlignment: Text.AlignHCenter
					elide: Text.ElideMiddle
					anchors {
						top: lblTitle.bottom
						left: parent.left
						right: buttonsRec.left
						margins: -5
					}
				}
			} //itemContent
		} //delegateItem: TPListViewDelegate
	} //TPListView
}
