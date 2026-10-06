import QtQuick

import TpQml
import TpQml.Widgets

TPBackRec {
	id: _control
	enabled: listModel ? (_control.listModel.enableFilters ? true : listModel.count > 0) : false
	height: {
		let ret = 0;
		if (showSearch)
			ret = 3*_margins + txtSearch.height;
		if (showFilter|| showSort)
			ret += 2*_margins + AppSettings.itemSmallHeight;
		if (showSelectionOptions)
			ret += 2*_margins + chkEnableSelection.height;
		return ret;
	}

//public:
	required property TPListView tpListView
	property bool showFilter: true
	property bool showSearch: true
	property bool showSort: true
	property bool showSelectionOptions: true

//private:
	readonly property TPListModel listModel: tpListView.listModel
	property int _margins: 5

	TPButtonGroup {
		id: sortGroup
	}

	TPComboBox {
		id: cboSortFields
		model: _control.listModel.fieldsNames
		specialIndex: 0
		currentIndex: _control.listModel.sortField
		visible: _control.showSort
		width: parent.width * 0.3
		onItemActivated: (real_index, index, value) => {
			_control.listModel.sortField = real_index;
			//need to force an update because isChecked is not responding to (or not receiving) the TPListModel.sortChanged() signal
			if (real_index == -1) {
				btnSortDown.isChecked = false;
				btnSortUp.isChecked = false;
			}
		}

		anchors {
			right: parent.right
			top: parent.top
			margins: _control._margins
		}
	}

	TPRadioButtonOrCheckBox {
		id: btnSortDown
		buttonGroup: sortGroup
		isChecked: _control.listModel.sortDirection === TPListModel.SORT_DOWN
		image: "sort_down.png"
		imageHeight: AppSettings.itemSmallHeight
		width: 2*imageHeight + 5
		height: imageHeight
		visible: _control.showSort
		enabled: _control.listModel.sortField !== -1
		onChecked: (check) => {
			if (check)
				_control.listModel.sortDirection = TPListModel.SORT_DOWN;
		}

		anchors {
			right: cboSortFields.left
			top: parent.top
			margins: _control._margins
		}
	}

	TPRadioButtonOrCheckBox {
		id: btnSortUp
		buttonGroup: sortGroup
		isChecked: _control.listModel.sortDirection === TPListModel.SORT_UP
		image: "sort_up.png"
		imageHeight: AppSettings.itemSmallHeight
		width: 2*imageHeight + 5
		height: imageHeight
		visible: _control.showSort
		enabled: _control.listModel.sortField !== -1
		onChecked: (check) => {
			if (check)
				_control.listModel.sortDirection = TPListModel.SORT_UP;
		}

		anchors {
			right: btnSortDown.left
			top: parent.top
			margins: _control._margins
		}
	}

	TPButton2 {
		id: btnFilter
		text: qsTr("Filter")
		rounded: false
		checkable: true
		isChecked: _control.listModel.enableFilters
		image: "filter.png"
		imageHeight: AppSettings.itemSmallHeight
		width: parent.width * 0.35
		height: imageHeight
		onButtonClicked: (btn_id) => _control.listModel.showFiltersDialog();
		onChecked: (check) => _control.listModel.enableFilters = check;

		anchors {
			left: parent.left
			top: parent.top
			margins: _control._margins

		}
	}

	TPTextInput {
		id: txtSearch
		showClearTextButton: true
		showSearchIcon: true
		visible: _control.showSearch

		anchors {
			top: _control.showFilter ? btnFilter.bottom : (_control.showSort ? cboSortFields.bottom : _control.top)
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
		onChecked: (check) => _control.tpListView.items_selectable = check;

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
		enabled: _control.enabled && chkEnableSelection.isChecked
		onClicked: _control.listModel.selectAll();

		anchors {
			bottom: parent.bottom
			left: chkEnableSelection.right
			right: parent.right
			margins: _control._margins
		}
	}
}
