pragma ComponentBehavior: Bound

import QtQuick

import TpQml

TPPopup {
	objectName: "TPPageMenu"
	id: _menu
	keepAbove: showIndicator
	showTitleBar: false
	closeButtonVisible: false
	backgroundRec: null
	width: backRec.width + smallWidth
	height: Math.max(backRec.height, smallHeight)
	configFieldName: "pageMenu_" + parentPage.objectName
	defaultCoordinates: Qt.point(defaultX, realPageY() + 180)
	lockMovingToYAxis: showIndicator
	show_position: showIndicator ? Qt.AlignBaseline : Qt.AlignBottom
	mouseItem: indicator

//public:
	required property var entriesList
	property bool showIndicator: true

	signal menuEntrySelected(btn_id: int);

//protected:
	readonly property int smallWidth: AppSettings.itemSmallHeight
	readonly property int smallHeight: 4 * AppSettings.itemSmallHeight
	readonly property int defaultX: parentPage.width - smallWidth
	property bool expanded: !showIndicator
	property TPBackRec indicator

	signal _entryVisible(entry_idx: int, visibility: bool);

	onMouseItemClicked: (mouse) => {
		expanded = !expanded;
		x = expanded ? defaultX - entriesListView.width : defaultX
	}

	ListModel {
		id: entriesListModel

		property int visibleCount
		property list<bool> entryVisible

		Component.onCompleted: {
			visibleCount = _menu.entriesList.length;
			for (let i = 0; i < visibleCount; i++) {
				entriesListModel.append(_menu.entriesList[i]);
				entryVisible[i] = true;
			}
		}
	}

	Loader {
		id: indicatorLoader
		active: _menu.showIndicator
		width: _menu.smallWidth
		height: _menu.smallHeight

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
					target: _menu
					function onEnabledChanged() { canvas.requestPaint(); }
					function onExpandedChanged() { canvas.requestPaint(); }
					function onVisibleChanged() {
						if (_menu.visible)
							canvas.requestPaint();
					}
				}

				onPaint: {
					if (!context)
						_context = getContext("2d");
					else
						_context = context;

					_context.reset();
					if (!_menu.expanded) {
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
					_context.fillStyle = _menu.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
					_context.fill();
				}
			} //Canvas

			Component.onCompleted: _menu.indicator = this;
		} //indicator
	} //Loader

	TPBackRec {
		id: backRec
		enableShadow: true
		width: largest_entry_width
		height: AppSettings.itemDefaultHeight * entriesListModel.visibleCount
		backColor: AppSettings.paneBackgroundColor
		radius: 8

		property int largest_entry_width: AppSettings.pageWidth * 0.6

		states: [
			State {
				when: _menu.showIndicator

				AnchorChanges {
					target: backRec
					anchors.top: parent.top
					anchors.left:indicatorLoader.right
				}
			},
			State {
				when: !_menu.showIndicator

				AnchorChanges {
					target: backRec
					anchors.horizontalCenter: parent.horizontalCenter;
					anchors.verticalCenter: parent.verticalCenter;
				}
			}
		]

		ListView {
			id: entriesListView
			model: entriesListModel
			spacing: 0
			delegateModelAccess: DelegateModel.ReadOnly
			reuseItems: true
			clip: true
			anchors.fill: parent

			delegate: Item {
				id: delegate
				width: backRec.largest_entry_width
				height: AppSettings.itemDefaultHeight
				enabled: entriesListModel.get(index).enabled

				required property int index

				Component.onCompleted: _menu._entryVisible.connect(setVisible);

				function setVisible(entry_idx: int, visibility: bool): void {
					if (entry_idx === delegate.index)
						visible = visibility;
				}

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
						if (contentWidth + entry_img.width > backRec.largest_entry_width) {
							let entry_width = contentWidth + entry_img.width;
							if (entry_width >= AppSettings.pageWidth * 0.8)
								entry_width = AppSettings.pageWidth * 0.8;
							backRec.largest_entry_width = entry_width;
						}
					}
				}

				MouseArea {
					enabled: delegate.enabled
					anchors.fill: parent
					onClicked: {
						entriesListView.currentIndex = delegate.index;
						springUp.start();
					}
				}

				NumberAnimation {
					id: springUp
					target: delegate
					property: "height"
					from: delegate.height
					to: delegate.height * 1.2
					easing.type: Easing.InOutQuad

					onFinished: {
						_menu.menuEntrySelected(entriesListModel.get(delegate.index).btn_id);
						_menu.expanded = false;
						delegate.height = AppSettings.itemDefaultHeight
					}
				}
			} //ItemDelegate
		} //TPListView
	} //TPBackRec

	function tpOpen(): void {
		if (showIndicator) {
			open_in_window = true;
			show_position = Qt.AlignBaseline;
		}
		else {
			open_in_window = false;
			show_position = Qt.AlignBottom;
		}
		tpopen__();
	}

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
		entriesListModel.entryVisible.push(true);
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
			entriesListModel.entryVisible.push(true);
		}
	}

	function removeEntry(entry_idx: int): void {
		if (entry_idx < entriesListModel.count) {
			entriesListModel.remove(entry_idx);
			let new_entry_visible = [];
			for (let i = 0; i < entriesListModel.count; ++i) {
				if (i !== entry_idx)
					new_entry_visible.push(entriesListModel.entryVisible[i]);
			}
			entriesListModel.entryVisible = new_entry_visible;
		}
	}
	function removeEntryById(btn_id: int): void {
		for (let i = 0; i < entriesListModel.count; ++i) {
			if (entriesListModel.get(i).btn_id === btn_id) {
				removeEntry(i);
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

	function setVisible(entry_idx: int, visibility: bool): void {
		if (entry_idx < entriesListModel.count) {
			if (entriesListModel.entryVisible[entry_idx] !== visibility) {
				_entryVisible(entry_idx, visibility);
				entriesListModel.entryVisible[entry_idx] = visibility;
				entriesListModel.visibleCount += visibility ? 1 : -1;
			}
		}
	}

	function setVisibleById(btn_id: int, visibility: bool): void {
		for (let i = 0; i < entriesListModel.count; ++i) {
			if (entriesListModel.get(i).btn_id === btn_id) {
				setVisible(i, visibility);
				break;
			}
		}
	}
} //TPPopup
