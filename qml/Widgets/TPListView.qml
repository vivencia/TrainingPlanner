pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml

Item {
	id: _control
	height: minimumHeight

//public:
	property bool showHeader: true
	property bool canFilter: showHeader
	property bool canSearch: showHeader
	property bool canSelectItems: showHeader
	property bool canSort: showHeader
	readonly property alias vBar: _vBar
	readonly property int minimumHeight: 15 + (showHeader ? _headerLoader.height : 0)
										+ (_listView.count > 0 ? _listView.itemAtIndex(0).height + 2*_listView.spacing : 0)
	property alias delegate: _listView.delegate
	property alias model: _listView.model
	property alias currentIndex: _listView.currentIndex
	property alias spacing: _listView.spacing
	property alias contentHeight: _listView.contentHeight

//protected:
	property bool items_selectable: false

	Loader {
		id: _headerLoader
		asynchronous: true
		active: _control.showHeader
		height: _header ? _header.height : 0

		property TPListViewHeader _header: null

		sourceComponent: TPListViewHeader {
			listView: _control as TPListView
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
		boundsBehavior: ListView.StopAtBounds
		contentWidth: width
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

		ScrollBar.vertical: ScrollBar {
			id: _vBar
			policy: ScrollBar.AsNeeded
			active: true
		}

		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AsNeeded
		}

		anchors {
			top: _control.showHeader ? _headerLoader.bottom : parent.top
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			topMargin: 15
		}
	} //ListView

	function showFilterDialog(): void {

	}
}
