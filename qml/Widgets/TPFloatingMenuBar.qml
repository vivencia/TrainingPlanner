pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import TpQml

TPPopup {
	id: _control
	keepAbove: false
	closeButtonVisible: false
	width: Math.max(lblTitle.width, mainLayout.childrenRect.width + 20)
	height: mainLayout.childrenRect.height + lblTitle.height + 20

//public:
	property string titleHeader
	property var entriesList: []

	signal menuEntrySelected(id: int);

	ListModel {
		id: entriesList

		Component.onCompleted: {
			for (let i = 0; i < _control.entriesList.length; i++)
				append(_control.entriesList[i]);
		}
	}

	TPLabel {
		id: lblTitle
		text: _control.titleHeader
		horizontalAlignment: Text.AlignHCenter
		visible: _control.titleHeader.length > 0
		height: visible ? AppSettings.itemDefaultHeight : 0
		font: AppGlobals.smallFont

		anchors {
			top: parent.top
			horizontalCenter: parent.horizontalCenter
		}
	}

	Column {
		id: mainLayout
		padding: 5
		spacing: 5
		opacity: _control.opacity

		anchors {
			top: _control.titleHeader.length > 0 ? lblTitle.bottom : parent.top
			left: parent.left
			margins: 5
			topMargin: 10
		}

		Repeater {
			id: entriesRepeater
			model: entriesList.count

			TPButton {
				text: entriesList.get(index).Label
				imageSource: entriesList.get(index).Image
				clickId: entriesList.get(index).ClickId
				visible: entriesList.get(index).Visible
				rounded: false
				autoSize: true
				Layout.alignment: Qt.AlignHCenter
				Layout.preferredWidth: Math.min(AppSettings.pageWidth * 0.5, width)

				required property int index

				onClicked: {
					_control.menuEntrySelected(clickId);
					_control.close();
				}
			}
		}
	}
}
