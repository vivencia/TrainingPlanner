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

//public:
	property bool canSelectItems: false
	property bool allItemsSelected: false

//protected:
	property bool items_selectable: false

	ScrollBar.vertical: ScrollBar {
		id: _vBar
		policy: ScrollBar.AsNeeded
		active: true
	}

	ScrollBar.horizontal: ScrollBar {
		policy: ScrollBar.AsNeeded
	}

	function search(text: string): void {

	}

	function selectAll(): void {

	}

	function showFilterDialog(): void {

	}
}
