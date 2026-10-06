pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

TPPopup {
	id: _dialog
	objectName: title.replace(/\s+/g, '') //strip all white spaces
	keepAbove: true
	showTitleBar: true
	resizeable: true
	useShape: true
	showBorder: true
	show_position: Qt.AlignBaseline
	defaultCoordinates: Qt.point((AppSettings.pageWidth - width)/2, (AppSettings.windowHeight - height)/2)
	normal_size: Qt.size((lblTitle.preferredWidth + titleBarWidth) * 1.1,
									Math.min(_listView.contentHeight + 2*titleBarHeight, AppSettings.pageHeight * 0.5))
	savePopupState: true

//public:
	required property TPFilterModel filtersModel
	property string title

	TPLabel {
		id: lblTitle
		text: _dialog.title
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
			margins: 5
			rightMargin: _dialog.titleBar.titleBarButtons.width
		}
	}

	ListView {
		id: _listView
		model: _dialog.filtersModel
		delegateModelAccess: DelegateModel.ReadWrite
		boundsBehavior: ListView.StopAtBounds
		reuseItems: true
		clip: true
		focus: true
		spacing: 2

		anchors {
			top: lblTitle.bottom
			left: parent.left
			right: parent.right
			bottom: btnClose.top
			margins: 5
			topMargin: 10
		}

		ScrollBar.vertical: ScrollBar {
			policy: ScrollBar.AsNeeded
			interactive: Qt.platform.os !== "android"

			anchors {
				top: parent.top
				bottom: parent.bottom
				right: parent.right
			}
		}

		ScrollBar.horizontal: ScrollBar {
			policy: ScrollBar.AsNeeded
			interactive: Qt.platform.os !== "android"
			anchors {
				bottom: parent.bottom
				left: parent.left
				right: parent.right
			}
		}

		delegate: Item {
			id: delegate
			width: parent.width
			height: _control1.height + _control2.height

			required property int index
			required property int field
			required property string display
			required property string icon
			required property list<string> values
			required property bool itemVisible
			required property bool itemEnabled
			required property bool selected

			onSelectedChanged: {
				_control1.isChecked = selected;
				if (!selected)
					_control2.clearAllCheckedPopupItems();
			}

			TPRadioButtonOrCheckBox {
				id: _control1
				text: delegate.display
				height: delegate.visible ? preferredHeight : 0
				enabled: delegate.itemEnabled
				indicatorPos: Qt.AlignRight | Qt.AlignVCenter
				imagePos: Qt.AlignLeft | Qt.AlignVCenter
				image: delegate.icon
				font.pixelSize: AppSettings.fontSize
				onChecked: (check) => {
					if (!delegate.selected)
						delegate.selected = true;
				}

				anchors {
					top: parent.top
					left: parent.left
					right: parent.right
					bottomMargin: 5
				}
			}

			TPComboBox {
				id: _control2
				model: delegate.values
				textWhenCurIndexIsInvalid: qsTr("Select what to filter")
				checkable: true
				visible: _control1.isChecked
				height: _control1.isChecked ? AppSettings.itemDefaultHeight : 0

				anchors {
					top: _control1.bottom
					left: parent.left
					right: parent.right
					bottomMargin: 5
				}

				onItemChecked: (real_index,index,checked) => {
					if (checked)
						_dialog.filtersModel.addValue(delegate.index, index);
					else
						_dialog.filtersModel.delValue(delegate.index, index);
				}
			}
		}
	}

	TPButton {
		id: btnClose
		text: "OK"
		onClicked: _dialog.closePopup(TPPopup.BTN_CLOSE);
		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
		}
	}
}
