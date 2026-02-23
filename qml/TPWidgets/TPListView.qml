import QtQuick
import QtQuick.Controls

ListView {
	readonly property ScrollBar vBar: _vBar

	boundsBehavior: ListView.StopAtBounds
	delegateModelAccess: DelegateModel.ReadOnly
	reuseItems: true
	clip: true
	focus: true
	spacing: 2

	ScrollBar.vertical: ScrollBar {
		id: _vBar
		policy: ScrollBar.AsNeeded
		active: true
	}

	ScrollBar.horizontal: ScrollBar {
		policy: ScrollBar.AsNeeded
	}
}
