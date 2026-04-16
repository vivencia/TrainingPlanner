pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import TpQml

TPPopup {
	id: _control
	keepAbove: true
	showTitleBar: false
	closeButtonVisible: false
	backgroundRec: indicator
	width: expanded ? entriesListView.width + smallWidth : smallWidth
	height: expanded ? Math.max(entriesListView.height, smallHeight) : smallHeight
	x: expanded ? parentPage.width - width : _saved_pos.x
	finalYPos: _saved_pos.y

//public:
	required property ListModel entriesListModel
	required property list<bool> entry_enabled

	signal menuEntrySelected(btn_id: int);

//private:
	readonly property int smallWidth: AppSettings.itemSmallHeight
	readonly property int smallHeight: 4 * AppSettings.itemSmallHeight
	readonly property string _config_field_name: "pageMenu_" + parentPage.objectName
	readonly property point _saved_pos: AppSettings.getCustomValue(_config_field_name,
																   Qt.point(parentPage.width - smallWidth, parentPage.y + 180))
	property bool expanded: false

	Component.onCompleted: open();

	on_VisibleChanged: {
		if (_visible)
			canvas.requestPaint();
	}

	Behavior on width {
		animation: NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	TPBackRec {
		id: indicator
		showBorder: true
		width: _control.smallWidth
		height: _control.smallHeight

		anchors {
			top: parent.top
			left: parent.left
		}

		Canvas {
			id: canvas
			width: AppSettings.itemSmallHeight / 2
			height: width
			contextType: "2d"

			anchors {
				horizontalCenter: parent.horizontalCenter
				verticalCenter: parent.verticalCenter
			}

			Connections {
				target: _control
				function onEnabledChanged() { canvas.requestPaint(); }
				function onExpandedChanged() { canvas.requestPaint(); }
			}

			onPaint: {
				if (context) {
					context.reset();
					if (!_control.expanded) {
						context.moveTo(width, 0);
						context.lineTo(width, height);
						context.lineTo(0, height / 2);
					}
					else {
						context.moveTo(0, 0);
						context.lineTo(0, height);
						context.lineTo(width, height / 2);
					}
					context.closePath();
					context.fillStyle = _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
					context.fill();
				}
			}
		} //Canvas

		TPMouseArea {
			movingWidget: indicator
			movableWidget: _control
			lockMovingToYAxis: true
			enabled: _control.enabled

			onMouseClicked: _control.expanded = !_control.expanded;
			onMovingFinished: (x, y) => AppSettings.setCustomValue(_control._config_field_name, Qt.point(_control._saved_pos.x, y));
		}
	} //indicator

	TPListView {
		id: entriesListView
		model: _control.entriesListModel
		width: largest_entry_width
		height: (AppSettings.itemDefaultHeight + 2) * _control.entriesListModel.count

		property int largest_entry_width

		anchors {
			top: parent.top
			left: indicator.right
		}

		delegate: ItemDelegate {
			id: delegate
			spacing: 0
			padding: 0
			width: entriesListView.largest_entry_width
			height: AppSettings.itemDefaultHeight

			required property int index

			contentItem: Item {
				TPImage {
					id: entry_img
					source: _control.entriesListModel.get(delegate.index).image
					width: source !== "" ? AppSettings.itemDefaultHeight : 0
					height: AppSettings.itemDefaultHeight

					anchors {
						left: parent.left
						verticalCenter: parent.verticalCenter
					}
				}

				TPLabel {
					text: _control.entriesListModel.get(delegate.index).label
					enabled: _control.entry_enabled[delegate.index]

					anchors {
						left: entry_img.right
						leftMargin: 5
						verticalCenter: parent.verticalCenter
					}

					Component.onCompleted: {
						if (contentWidth + entry_img.width > entriesListView.largest_entry_width)
							entriesListView.largest_entry_width = contentWidth + entry_img.width + 5;
					}
				}
			}

			background: TPBackRec {
				showBorder: true
				color: delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2
				radius: 8
			}

			NumberAnimation {
				id: springUp
				target: delegate
				property: "height"
				from: delegate.height
				to: delegate.height * 1.2
				easing.type: Easing.InOutQuad

				onFinished: {
					_control.menuEntrySelected(_control.entriesListModel.get(delegate.index).btn_id);
					_control.expanded = false;
					delegate.height = AppSettings.itemDefaultHeight
				}
			}

			onClicked: springUp.start();
		} //ItemDelegate
	} //ListView
} //TPPopup
