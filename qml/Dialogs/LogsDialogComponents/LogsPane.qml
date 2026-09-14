pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

TPPane {
	id: _control
	maxHeight: AppSettings.pageHeight

	required property TPLogs logsModel

	TPListView {
		id: listView
		model: _control.logsModel		
		width: _control.width
		height: Math.min(_control.maxHeight, delegatesHeight)
		spacing: 5

		property int delegatesHeight: 0

		anchors {
			top: _control.headerWidget.bottom;
			left: parent.left
			topMargin: 10
			margins: 5
		}

		delegate: SwipeDelegate {
			id: delegate
			width: parent ? parent.width - 10 : 0
			height: logEntryItem.preferredHeight
			spacing: 0

			required property int index
			required property string logTitle
			required property string logMessage
			required property string logTime
			required property string logTooltip

			Connections {
				target: _control.logsModel
				function onEntryRemoved(entry: int): void {
					if (entry === delegate.index)
						listView.delegatesHeight -= height;
				}
			}

			Timer {
				id: bufferTimer
				interval: 500
				onTriggered: {
					listView.delegatesHeight += delegate.height * 1.2;
					delegate.setupLayout(); //Now that size reached its final value, layout the widgets inside TPRadioButtonOrCheckBox
				}
			}

			onHeightChanged: {
				if (bufferTimer.running)
					return;
				else
					bufferTimer.start();
			}

			ToolTip {
				id: logMessageTip
				closePolicy: Popup.CloseOnPressOutside
				z: 2
			}

			function setupLayout(): void {
				lblTitle.setupLayout();
			}

			contentItem: Item {
				id: logEntryItem
				readonly property int preferredHeight: lblTitle.height + lblMessage.height + 10

				Item {
					id: buttonsRec
					width: AppSettings.itemDefaultHeight

					anchors {
						top: parent.top
						right: parent.right
						bottom: parent.bottom
						margins: 5
						rightMargin: -10
					}

					TPButton {
						id: btnInfo
						imageSource: "info_"
						width: AppSettings.itemSmallHeight
						height: width
						onClicked: logMessageTip.show(delegate.logTooltip, 5000)
						anchors {
							horizontalCenter: parent.horizontalCenter
							top: parent.top
						}
					}
					TPButton {
						id: btnCopy
						imageSource: "copy_"
						width: AppSettings.itemSmallHeight
						height: width
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
					boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
					indicatorPos: Qt.AlignRight | Qt.AlignVCenter
					font.weight: Font.Bold
					font.pixelSize: AppSettings.fontSize
					onCheckedChanged: _control.logsModel.setSelected(delegate.index, checked);
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
			}//contentItem

			background: TPBackRec {
				id:	backgroundColor
				useGradient: true
				showBorder: true
				opacity: 0.9
			}

			swipe.right: Rectangle {
				width: parent.width
				height: parent.height
				clip: false
				color: SwipeDelegate.pressed ? "#555" : "#666"
				radius: 5

				TPImage {
					source: "remove"
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

			swipe.onCompleted: {
				if (_control.logsModel.isSelected(delegate.index))
					_control.logsModel.removeSelected();
				else
					_control.logsModel.removeEntry(delegate.index);
			}
		} //delegate: SwipeDelegate
	}
}
