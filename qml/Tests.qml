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

		TreeView {
			id: treeView
			anchors.fill: parent
			anchors.margins: 10
			clip: true

			model: AppMessages.messagesModel

			// Built-in styled delegate (recommended)
			//delegate: TreeViewDelegate {
				// You can customize text, icons, etc. here
			//}

			// Alternative: fully custom delegate
			delegate: Item {
				implicitWidth: padding + label.x + label.implicitWidth + padding
				implicitHeight: label.implicitHeight * 1.5

				required property TPMessage tpMessage
				required property TreeView treeView
				required property bool isTreeNode
				required property bool expanded
				required property bool hasChildren
				required property int depth
				required property int row
				required property bool current

				readonly property real indentation: 20
				readonly property real padding: 5

				Rectangle { // Background rectangle enabled to show the alternative row colors
					id: background
					opacity: 0.8
					anchors.fill: parent

					color: {
						if (delegateItem.model.row === delegateItem.treeView.currentRow) {
							return Qt.lighter(palette.highlight, 1.2)
						} else {
							if (delegateItem.treeView.alternatingRows && delegateItem.model.row % 2 !== 0) {
								return (Application.styleHints.colorScheme === Qt.Light) ?
										 Qt.darker(palette.alternateBase, 1.25) :
										 Qt.lighter(palette.alternateBase, 2.)
							} else {
							   return palette.base
							}
						}
					}
					Rectangle { // The selection indicator shown on the left side of the highlighted row
						width: delegateItem.padding
						height: parent.height
						visible: !delegateItem.model.column
						color: {
							if (delegateItem.model.row === delegateItem.treeView.currentRow) {
								return (Application.styleHints.colorScheme === Qt.Light) ?
										 Qt.darker(palette.highlight, 1.25) :
										 Qt.lighter(palette.highlight, 2.)
							} else {
								return "transparent"
							}
						}
					}
				}

				Label {
					id: indicator
					x: padding + (depth * indentation)
					anchors.verticalCenter: parent.verticalCenter
					text: hasChildren ? (expanded ? "▼" : "▶") : ""
					visible: isTreeNode && hasChildren
				}

				Label {
					id: label
					x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
					anchors.verticalCenter: parent.verticalCenter
					text: tpMessage.text
				}
			}
		}
	}
}
