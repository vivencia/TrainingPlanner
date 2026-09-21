import QtQuick

import TpQml
import TpQml.Widgets

Item {
	id: _control
	required property TPListView listView

	TPLabel {
		id: lblSearch
		text: qsTr("Search: ")
		width: _control.width * 0.3
		anchors {
			left: parent.left
			top: parent.top
			margins: 5
		}
	}

	TPButton {
		id: btnChooseFilters
		imageSource: "filter.png"
		width: AppSettings.itemSmallHeight
		height: width

		anchors {
			right: parent.right
			verticalCenter: lblSearch.verticalCenter
			margins: 5
		}

		onClicked: _control.listView.showFilterDialog();
	}

	TPTextInput {
		id: txtSearch
		showClearTextButton: true
		readOnly: !_control.enabled
		enabled: listView.count > 0

		anchors {
			top: lblSearch.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}

		onTextChanged: _control.listView.search(text);
	} // txtSearch

	TPRadioButtonOrCheckBox {
		id: chkEnableSelection
		text: qsTr("Selectable")
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		checked: _control.listView.items_selectable
		visible: _control.listView.canSelectItems
		width: _control.width / 2
		onClicked: _control.listView.items_selectable = checked;

		anchors {
			top: txtSearch.bottom
			left: parent.left
			right: parent.right
			margins: 5
		}
	}

	TPRadioButtonOrCheckBox {
		id: chkSelectAll
		text: qsTr("Select All")
		boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX
		checked: _control.listView.allItemsSelected
		visible: _control.listView.canSelectItems
		enabled: chkEnableSelection.checked
		onClicked: _control.listView.selectAll();

		anchors {
			top: txtSearch.bottom
			left: chkEnableSelection.right
			right: parent.right
			margins: 5
		}
	}
}
