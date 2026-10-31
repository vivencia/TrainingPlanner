pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Pages
import "ListViewComponents"

Item {
	id: _control
	height: minimumHeight

//public:
	required property TPListModel listModel
	required property TPPage parentPage
	property bool showHeader: true
	property bool canFilter: showHeader
	property bool canSearch: showHeader
	property bool canSelectItems: showHeader
	property bool canSort: showHeader
	property alias delegate: _listView.delegate
	property alias spacing: _listView.spacing
	property alias currentIndex: _listView.currentIndex
	readonly property int minimumHeight: _headerLoader.height + (listModel.visibleCount > 0
								? 40 + AppSettings.itemLargeHeight + delegates_height / listModel.visibleCount : 0)
	readonly property int contentHeight: _listView.contentHeight + _headerLoader.height

//protected:
	property bool items_selectable: false
	property int delegates_height: 0

	Component.onCompleted: listModel.parentPage = _control.parentPage;

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
		contentWidth: width
		contentHeight: _control.delegates_height
		currentIndex: _control.listModel.currentRow
		reuseItems: true
		highlight: highlight_component
		highlightFollowsCurrentItem: false
		clip: true
		focus: true
		spacing: 2

		onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)

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

		anchors {
			top: _control.showHeader ? _headerLoader.bottom : parent.top
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			topMargin: 15
		}
	} //ListView

	function itemAtIndex(index: int): Item {
		return _listView.itemAtIndex(index);
	}

	function setCurrentIndex(index: int): void {
		listModel.currentRow = index;
	}
}
