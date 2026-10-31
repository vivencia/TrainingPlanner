pragma ComponentBehavior: Bound

import QtQuick

import TpQml
import TpQml.Widgets

TPListView {
	id: listView
	listModel: AppExercisesList

	Component.onCompleted: {
		if (AppExercisesList.count === 0)
			AppExercisesList.initExercisesList();
	}

	delegate: TPListViewDelegate {
		id: delegate
		_index: index
		_itemVisible: itemVisible
		tpListView: listView

		required property int index
		required property bool itemVisible
		required property bool selected

		required property string mainName
		required property string subName
		required property string muscularGroup

		itemContent: TPRadioButtonOrCheckBox {
			text: String(delegate.index+1) + ":  " + delegate.mainName + "<br>" + delegate.subName
			elideMode: Text.ElideLeft
			boxType: listView.items_selectable ? TPRadioButtonOrCheckBox.TP_CHECKBOX : TPRadioButtonOrCheckBox.TP_NONEBOX
			indicatorPos: Qt.AlignRight | Qt.AlignVCenter
			enableCheckOutsideIndicator: false
			multiLine: true
			font.weight: Font.Bold
			font.pixelSize: AppSettings.fontSize
			onChecked: (check) => delegate.selected = check;
		}
	} //delegateItem: TPListViewDelegate
}
