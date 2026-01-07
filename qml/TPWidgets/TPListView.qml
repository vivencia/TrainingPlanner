import QtQuick
import QtQuick.Controls

ListView {
	readonly property ScrollBar vBar: _vBar

	boundsBehavior: ListView.StopAtBounds
	delegateModelAccess: DelegateModel.ReadOnly
	snapMode: ListView.SnapToItem
	reuseItems: true
	clip: true
	focus: true
	contentHeight: availableHeight
	contentWidth: availableWidth
	spacing: 2

	ScrollBar.vertical: ScrollBar {
		id: _vBar
		policy: ScrollBar.AsNeeded
		active: true
		interactive: true
	}

	ScrollBar.horizontal: ScrollBar {
		policy: ScrollBar.AsNeeded
		interactive: true
	}
}
