import QtQuick
import QtQuick.Controls

import TpQml

TableViewDelegate {
	id: _control
	text: userData
	implicitHeight: AppSettings.itemDefaultHeight

	required property string userData
	property int cellWidth

	onSelectedChanged: {
		AppUserModel.allUsers.setSelected(row, selected);
		AppUserModel.allUsers.currentRow = row;
	}

	Component.onCompleted: cellWidth = width;
}
