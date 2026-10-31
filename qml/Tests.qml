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

		/*TPListView {
		id: listView
		listModel: ItemManager.debugLog
		parentPage: homePage

			anchors {
				top: parent.top
				left: parent.left
				right: parent.right
				bottom: txt.top
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
						onClicked: ItemManager.debugLog.copyLog(delegate.index)
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
					onChecked: (check) => ItemManager.debugLog.setIsSelected(delegate.index, check);

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
						bottom: parent.bottom
						left: parent.left
						right: buttonsRec.left
						margins: -5
					}
				}
			} //itemContent
		} //delegateItem: TPListViewDelegate
	} //TPListView

		TPTextInput {
			id: txt
			anchors {
				bottom: btnAddLog.top
				left: parent.left
				right: parent.right
			}
		}

		TPButton {
			id: btnAddLog
			text: "Add"
			property int opt: 0
			onClicked: {
				if (txt.text.length > 0) {
					ItemManager.log(TPLogs.LT_DEBUG, "TPListModel Tests", "Cabeçalho", txt.text);
					txt.clear();
				}
			}

			anchors {
				bottom: parent.bottom
				horizontalCenter: parent.horizontalCenter
			}
		}*/
	}
}
