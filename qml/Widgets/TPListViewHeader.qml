import QtQuick

import TpQml
import TpQml.Widgets

TPBackRec {
	id: _control
	enabled: listView.count > 0
	showBorder: true
	height: {
		let ret = 0;
		if (showSearch)
			ret = 3*_margins + lblSearch.height + txtSearch.height;
		else
			ret = 2*_margins + (showFilter || showSort ? AppSettings.itemSmallHeight : 0);
		if (showSelectionOptions)
			ret += 2*_margins + chkEnableSelection.height;
		return ret;
	}

//public:
	required property ListView listView
	property bool showFilter: true
	property bool showSearch: true
	property bool showSort: true
	property bool showSelectionOptions: true

//private:
	readonly property TPListModel listModel: listView.model as TPListModel
	property int _margins: 5

	TPLabel {
		id: lblSearch
		text: qsTr("Search: ")
		width: _control.width * 0.3
		height: AppSettings.itemDefaultHeight
		visible: _control.showSearch

		anchors {
			left: parent.left
			top: parent.top
			margins: _control._margins
		}
	}

	TPButtonGroup {
		id: sortGroup
	}

	TPButton {
		id: btnSortDown
		buttonGroup: sortGroup
		checkable: true
		image: "sort_down.png"
		imageHeight: AppSettings.itemSmallHeight
		width: 2*imageHeight + 5
		height: imageHeight
		visible: _control.showSort
		onChecked: (check) => _control.listModel.sortDown = check;

		anchors {
			right: parent.right
			top: parent.top
			margins: _control._margins
		}
	}

	TPButton {
		id: btnSortUp
		buttonGroup: sortGroup
		checkable: true
		image: "sort_up.png"
		imageHeight: AppSettings.itemSmallHeight
		width: 2*imageHeight + 5
		height: imageHeight
		visible: _control.showSort
		onChecked: (check) => _control.listModel.sortUp = check;

		anchors {
			right: btnSortDown.left
			top: parent.top
			margins: _control._margins
		}
	}

	TPButton {
		id: btnFilter
		checkable: true
		isChecked: _control.listModel.filterApplied
		image: "filter.png"
		imageHeight: AppSettings.itemSmallHeight
		width: 2*imageHeight + 5
		height: imageHeight
		onButtonClicked: (btn_id) => _control.listView.showFilterDialog();

		anchors {
			right: _control.showSort ? btnSortUp.left : parent.right
			top: parent.top
			margins: _control._margins
		}
	}

	TPTextInput {
		id: txtSearch
		showClearTextButton: true
		visible: _control.showSearch

		anchors {
			top: lblSearch.bottom
			left: parent.left
			right: parent.right
			margins: _control._margins
		}

		onTextChanged: _control.listModel.search(text);
	} // txtSearch

	TPRadioButtonOrCheckBox {
		id: chkEnableSelection
		text: qsTr("Selectable")
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		visible: _control.showSelectionOptions
		width: _control.width / 2 - 5
		onChecked: (check) => _control.listView.items_selectable = check;

		anchors {
			bottom: parent.bottom
			left: parent.left
			margins: _control._margins
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkSelectAll
		text: qsTr("Select All")
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		visible: _control.showSelectionOptions
		enabled: _control.enabled && chkEnableSelection.checked
		onClicked: _control.listModel.selectAll();

		anchors {
			bottom: parent.bottom
			left: chkEnableSelection.right
			right: parent.right
			margins: _control._margins
		}
	}
}
