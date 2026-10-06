pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml

import "ListViewComponents"

Item {
	id: _control
	height: minimumHeight

//public:
	required property TPListModel listModel

	property bool showHeader: true
	property bool canFilter: showHeader
	property bool canSearch: showHeader
	property bool canSelectItems: showHeader
	property bool canSort: showHeader
	property alias delegate: _listView.delegate
	property alias currentIndex: _listView.currentIndex
	property alias spacing: _listView.spacing
	//readonly property alias vBar: _vBar
	readonly property int minimumHeight: 15 + _headerLoader.height +
										(_listView.count > 0 ? _listView.itemAtIndex(0).height + 2*_listView.spacing : 0)
	readonly property int contentHeight: _listView.contentHeight + _headerLoader.height

//protected:
	property bool items_selectable: false
	property int delegates_height: 0

	Loader {
		id: _headerLoader
		asynchronous: true
		active: _control.showHeader
		height: _header ? _header.height : 0

		property TPListViewHeader _header: null

		sourceComponent: TPListViewHeader {
			tpListView: _control as TPListView
			showFilter: _control.canFilter
			showSearch: _control.canSearch
			showSort: _control.canSort
			showSelectionOptions: _control.canSelectItems
			enableShadow: true

			Component.onCompleted: _headerLoader._header = this;
		}

		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
	}

	ListView {
		id: _listView
		model: _control.listModel
		//boundsBehavior: ListView.StopAtBounds
		contentWidth: width
		contentHeight: contentItem.childrenRect.height * 1.1
		reuseItems: true
		highlight: highlight_component
		highlightFollowsCurrentItem: false
		clip: true
		focus: true
		spacing: 2

		Component {
			id:	highlight_component
			Rectangle {
				width: ListView.view.width - 10
				height: AppSettings.itemDefaultHeight
				x: 5
				color: AppSettings.primaryColor
				radius: 8
				y: ListView.view.currentItem ? ListView.view.currentItem.y + 2 : 2

				Behavior on y {
					SpringAnimation {
						spring: 3
						damping: 0.2
					}
				}
			}
		}

		/*ScrollBar.vertical: ScrollBar {
			id: _vBar
			policy: ScrollBar.AsNeeded
			active: true
			interactive: Qt.platform.os !== "android"

			anchors {
				top: parent.top
				bottom: parent.bottom
				right: parent.right
			}
		}*/

		anchors {
			top: _control.showHeader ? _headerLoader.bottom : parent.top
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			topMargin: 15
		}
	} //ListView
}
