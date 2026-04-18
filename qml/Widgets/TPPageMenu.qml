pragma ComponentBehavior: Bound

import QtQuick

import TpQml

TPPopup {
	id: _control
	keepAbove: showIndicator
	enableEffects: showIndicator
	showTitleBar: false
	closeButtonVisible: false
	width: expanded ? entriesListView.width + smallWidth : smallWidth
	height: expanded ? Math.max(entriesListView.height, smallHeight) : smallHeight
	x: expanded ? parentPage.width - width : _saved_pos.x
	finalYPos: _saved_pos.y
	backgroundRec: showIndicator ? indicator : null

//public:
	required property var entriesList
	property bool showIndicator: true

	signal menuEntrySelected(btn_id: int);

//private:
	readonly property int smallWidth: AppSettings.itemSmallHeight
	readonly property int smallHeight: 4 * AppSettings.itemSmallHeight
	readonly property string _config_field_name: "pageMenu_" + parentPage.objectName
	readonly property point _saved_pos: AppSettings.getCustomValue(_config_field_name,
																   Qt.point(parentPage.width - smallWidth, realPageY() + 180))
	property bool expanded: !showIndicator
	property TPBackRec indicator

	Component.onCompleted: {
		if (showIndicator)
			open();
	}

	ListModel {
		id: entriesListModel
		Component.onCompleted: {
			for (let i = 0; i < _control.entriesList.length; i++)
				entriesListModel.append(_control.entriesList[i]);
		}
	}

	Behavior on width {
		animation: NumberAnimation {
			easing.type: Easing.InOutBack
		}
	}

	Loader {
		id: indicatorLoader
		active: _control.showIndicator
		width: _control.smallWidth
		height: _control.smallHeight

		anchors {
			top: parent.top
			left: parent.left
		}

		sourceComponent: TPBackRec {
			showBorder: true
			backColor: AppSettings.primaryDarkColor
			radius: 8

			Canvas {
				id: canvas
				width: AppSettings.itemSmallHeight / 2
				height: width
				contextType: "2d"

				property var _context: null

				anchors {
					horizontalCenter: parent.horizontalCenter
					verticalCenter: parent.verticalCenter
				}

				Connections {
					target: _control
					function onEnabledChanged() { canvas.requestPaint(); }
					function onExpandedChanged() { canvas.requestPaint(); }
					function on_VisibleChanged() {
						if (_control._visible)
							canvas.requestPaint();
					}
				}

				onPaint: {
					if (!context)
						_context = getContext("2d");
					else
						_context = context;

					_context.reset();
					if (!_control.expanded) {
						_context.moveTo(width, 0);
						_context.lineTo(width, height);
						_context.lineTo(0, height / 2);
					}
					else {
						_context.moveTo(0, 0);
						_context.lineTo(0, height);
						_context.lineTo(width, height / 2);
					}
					_context.closePath();
					_context.fillStyle = _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
					_context.fill();
				}
			} //Canvas

			Component.onCompleted: _control.indicator = this;

			TPMouseArea {
				movingWidget: _control.indicator
				movableWidget: _control
				lockMovingToYAxis: true
				enabled: _control.enabled

				onMouseClicked: _control.expanded = !_control.expanded;
				onMovingFinished: (x, y) => AppSettings.setCustomValue(_control._config_field_name, Qt.point(_control._saved_pos.x, y));
			}
		} //indicator
	} //Loader

	ListView {
		id: entriesListView
		model: entriesListModel
		spacing: 0
		delegateModelAccess: DelegateModel.ReadOnly
		reuseItems: true
		clip: true
		width: largest_entry_width
		height: (AppSettings.itemDefaultHeight + 5) * entriesListModel.count

		property int largest_entry_width

		Component.onCompleted: {
			if (_control.showIndicator) {
				anchors.top = parent.top;
				anchors.left = indicatorLoader.right;
			}
			else {
				anchors.horizontalCenter = parent.horizontalCenter;
				anchors.verticalCenter = parent.verticalCenter;
			}
		}

		delegate: Item {
			id: delegate
			width: entriesListView.largest_entry_width
			height: AppSettings.itemDefaultHeight
			enabled: entriesListModel.get(delegate.index).enabled

			required property int index

			TPImage {
				id: entry_img
				source: entriesListModel.get(delegate.index).image
				width: source !== "" ? AppSettings.itemDefaultHeight : 0
				height: AppSettings.itemDefaultHeight

				anchors {
					left: parent.left
					verticalCenter: parent.verticalCenter
				}
			}

			TPLabel {
				text: entriesListModel.get(delegate.index).label
				useBackground: true
				backgroundColor: delegate.index % 2 === 0 ? AppSettings.listEntryColor1 : AppSettings.listEntryColor2

				anchors {
					left: entry_img.right
					leftMargin: 5
					right: parent.right
					verticalCenter: parent.verticalCenter
				}

				Component.onCompleted: {
					if (contentWidth + entry_img.width > entriesListView.largest_entry_width) {
						let entry_width = contentWidth + entry_img.width;
						if (entry_width >= AppSettings.pageWidth * 0.8)
							entry_width = AppSettings.pageWidth * 0.8;
						entriesListView.largest_entry_width = entry_width;
					}
				}
			}

			TPMouseArea {
				movingWidget: this
				movableWidget: _control
				lockMovingToYAxis: _control.showIndicator
				enabled: delegate.enabled
				anchors.fill: parent

				onClicked: {
					entriesListView.currentIndex = delegate.index;
					springUp.start();
				}
				onMovingFinished: (x, y) => AppSettings.setCustomValue(_control._config_field_name, Qt.point(_control._saved_pos.x, y));
			}

			NumberAnimation {
				id: springUp
				target: delegate
				property: "height"
				from: delegate.height
				to: delegate.height * 1.2
				easing.type: Easing.InOutQuad

				onFinished: {
					_control.menuEntrySelected(entriesListModel.get(delegate.index).btn_id);
					_control.expanded = false;
					delegate.height = AppSettings.itemDefaultHeight
				}
			}
		} //ItemDelegate
	} //TPListView

	function clearEntries(): void {
		entriesListModel.clear();
	}

	function createEntry(label: string, image: string, btn_id: int, enabled: bool): void {
		let entry = {
			"label": label,
			"image": image,
			"btn_id": btn_id,
			"enabled": enabled
		}
		entriesListModel.append(entry);
	}

	function createEntries(labels: list<string>, images: list<string>, btn_ids: list<int>, enableds: list<bool>) {
		for (let i = 0; i < labels.length; i++) {
			let entry = {
				"label": labels[i],
				"image": i < images.length ? images[i] : "",
				"btn_id": i < btn_ids.length ? btn_ids[i] : i,
				"enabled": i <  enableds.length ? enableds[i] : true
			}
			entriesListModel.append(entry);
		}
	}

	function removeEntry(entry_idx: int): void {
		if (entry_idx < entriesListModel.count)
			entriesListModel.remove(entry_idx);
	}
	function removeEntryById(btn_id: int): void {
		for (let i = 0; i < entriesListModel.count; ++i) {
			if (entriesListModel.get(i).btn_id === btn_id) {
				entriesListModel.remove(i);
				break;
			}
		}
	}

	function enableEntry(entry_idx: int, enabled: bool): void {
		entriesListModel.set(entry_idx, { "enabled":enabled });
	}

	function enableEntryById(btn_id: int, enabled: bool): void {
		for (let i = 0; i < entriesListModel.count; ++i) {
			if (entriesListModel.get(i).btn_id === btn_id) {
				entriesListModel.set(i, { "enabled":enabled });
				break;
			}
		}
	}

	function changeEntryLabel(entry_idx: int, new_label: string): void {
		if (entry_idx < entriesListModel.count)
			entriesListModel.set(entry_idx, { "label": new_label });
	}
	function changeEntryLabelById(btn_id: int, new_label: string): void {
		for (let i = 0; i < entriesListModel.count; ++i) {
			if (entriesListModel.get(i).btn_id === btn_id) {
				entriesListModel.set(i, { "label": new_label });
				break;
			}
		}
	}
} //TPPopup
