import QtQuick

import TpQml

TPBackRec {
	id: _control
	backColor: "transparent"
	height: AppSettings.itemDefaultHeight + 10

//public:
	property alias image: img.source
	property alias text: label.text
	property alias elideMode: label.elide
	property alias font: label.font //When overriding any font property, all the font properties are reset; so other properties must be reapplied
	property int imageHeight: AppSettings.itemDefaultHeight
	property int imageWidth: image.length > 0 ? imageHeight : 0
	property int indicatorPos: Qt.AlignLeft | Qt.AlignVCenter
	property int imagePos: Qt.AlignLeft | Qt.AlignVCenter
	property bool imageAfterIndicator: true
	property bool checked: false
	property bool multiLine: false
	property bool actionable: enabled
	property int boxType: TPRadioButtonOrCheckBox.TP_RADIOBOX
	property TPButtonGroup buttonGroup: null

	enum BoxType { TP_RADIOBOX, TP_CHECKBOX, TP_NONEBOX }
	signal clicked()

//private:
	enum BoxControls { INDICATOR_ONLY, INDICATOR_PLUS_IMAGE, IMAGE_ONLY, IMAGE_PLUS_INDICATOR, TEXT_ONLY }
	property Item _top_control: null
	property Item _bottom_control: null
	property Item _left_control: null
	property Item _right_control: null
	property bool _component_completed: false

	readonly property int _control_subtype: {
		if (img.OK) {
			if (boxType !== TPRadioButtonOrCheckBox.TP_NONEBOX)
				return imageAfterIndicator ? TPRadioButtonOrCheckBox.INDICATOR_PLUS_IMAGE : TPRadioButtonOrCheckBox.IMAGE_PLUS_INDICATOR;
			else
				return TPRadioButtonOrCheckBox.IMAGE_ONLY;
		} else {
			if (boxType !== TPRadioButtonOrCheckBox.TP_NONEBOX) {
				return TPRadioButtonOrCheckBox.INDICATOR_ONLY;
			} else {
				return TPRadioButtonOrCheckBox.TEXT_ONLY;
			}
		}
	}

	onWidthChanged: {
		if (_component_completed && width > AppSettings.itemDefaultHeight)
			setupLayout();
	}

	TPLabel {
		id: label
		singleLine: !_control.multiLine
		padding: 5
	}

	Rectangle {
		id: indicator
		width: _control.boxType !== TPRadioButtonOrCheckBox.TP_NONE ? AppSettings.itemSmallHeight : 0
		height: width
		radius: _control.boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX ? implicitWidth / 2 : 4
		color: "transparent"
		border.color: _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
		visible: _control.boxType !== TPRadioButtonOrCheckBox.TP_NONE
		anchors.margins: 2

		Rectangle {
			id: recChecked
			width: indicator.height * 0.5
			height: width
			radius: _control.boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX ? width / 2 : indicator.radius / 2
			x: (indicator.width - width) / 2
			y: x
			border.color: _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
			visible: _control.checked
		}
	}

	TPImage {
		id: img
		height: _control.imageHeight
		width: _control.imageWidth
		dropShadow: false
		visible: source.length > 0
		anchors.margins: 2
	}

	TapHandler { // Evaluates clicks/taps on top without stealing the events from underneath items
		onTapped: _control.mouseClicked();
	}

	Component.onCompleted: {
		if (boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX && buttonGroup) {
			buttonGroup.addButton(this);
			if (checked)
				buttonGroup.setChecked(this, true);
		}
		_component_completed = true;
		if (width > AppSettings.itemDefaultHeight)
			setupLayout();
	}

	Component.onDestruction: {
		if (boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX && buttonGroup)
			buttonGroup.removeButton(this);
	}

	function setupLayout(): void {
		switch (_control_subtype) {
			case TPRadioButtonOrCheckBox.INDICATOR_ONLY:
			case TPRadioButtonOrCheckBox.INDICATOR_PLUS_IMAGE:
				anchorOneControl(indicatorPos, indicator);
				if (_control_subtype !== TPRadioButtonOrCheckBox.INDICATOR_ONLY)
					anchorControls_part2(indicatorPos, indicator, imagePos, img);
				break;
			case TPRadioButtonOrCheckBox.IMAGE_ONLY:
			case TPRadioButtonOrCheckBox.IMAGE_PLUS_INDICATOR:
				anchorOneControl(imagePos, img);
				if (_control_subtype !== TPRadioButtonOrCheckBox.IMAGE_ONLY)
					anchorControls_part2(imagePos, img, indicatorPos, indicator);
				break;
			case TPRadioButtonOrCheckBox.TEXT_ONLY:
				break;
		}
		anchorLabel();
	}

	function anchorOneControl(pos: int, control: Item): void {
		if (pos & Qt.AlignVCenter) {
			control.anchors.verticalCenter = _control.verticalCenter
		} else if (pos & Qt.AlignTop) {
			control.anchors.top = _control.top;
			_top_control = control;
		} else if (pos & Qt.AlignBottom) {
			control.anchors.bottom = _control.bottom;
			_bottom_control = control;
		}
		if (pos & Qt.AlignLeft) {
			control.anchors.left = _control.left;
			_left_control = control;
		} else if (pos & Qt.AlignRight) {
			control.anchors.right = _control.right;
			_right_control = control;
		} else if (pos & Qt.AlignHCenter) {
			control.anchors.horizontalCenter = _control.horizontalCenter;
		}
	}

	function anchorControls_part2(first_pos: int, first_control: Item, second_pos: int, second_control: Item): void {
		let h_pos1, h_pos2 = 0;
		let v_pos1, v_pos2 = 0;
		let second_control_anchored = false;
		if (first_pos & Qt.AlignVCenter) {
			h_pos1 = first_pos ^ Qt.AlignVCenter;
			h_pos2 = second_pos ^ Qt.AlignVCenter;
			if (h_pos1 === h_pos2) {
				if (label.contentWidth >= _control.width - first_control.width - second_control.width) {
					first_control.anchors.verticalCenterOffset = -first_control.height/2;
					second_control.anchors.top = first_control.bottom;
					second_control.anchors.horizontalCenter = first_control.horizontalCenter;
					second_control_anchored = true;
				}
			} else {
				anchorOneControl(second_pos, second_control);
				second_control_anchored = true;
			}
		} else if (first_pos & Qt.AlignTop) {
			h_pos1 = first_pos ^ Qt.AlignVTop;
			h_pos2 = second_pos ^ Qt.AlignVTop;
			if (h_pos1 === h_pos2) {
				if (label.contentWidth >= _control.width - first_control.width - second_control.width) {
					item.anchors.top = _control.top;
					second_control.anchors.top = first_control.bottom;
					second_control.anchors.horizontalCenter = first_control.horizontalCenter;
					second_control_anchored = true;
				}
			} else {
				anchorOneControl(second_pos, second_control);
				second_control_anchored = true;
			}
		} else if (first_pos & Qt.AlignBottom) {
			h_pos1 = first_pos ^ Qt.AlignBottom;
			h_pos2 = second_pos ^ Qt.AlignBottom;
			if (h_pos1 === h_pos2) {
				if (label.contentWidth >= _control.width - first_control.width - second_control.width) {
					second_control.anchors.bottom = parent.bottom;
					first_control.anchors.bottom = second_control.top;
					second_control.anchors.horizontalCenter = first_control.horizontalCenter;
					second_control_anchored = true;
				}
			} else {
				anchorOneControl(second_pos, second_control);
				second_control_anchored = true;
			}
		}
		if (!second_control_anchored) {
			anchorOneControl(second_pos, second_control);
			if (first_pos & Qt.AlignLeft) {
				v_pos1 = first_pos ^ Qt.AlignLeft;
				v_pos2 = second_pos ^ Qt.AlignLeft;
				if (v_pos1 === v_pos2) {
					first_control.anchors.left = _control.left;
					second_control.anchors.left = first_control.right;
				}
			}
			else if (first_pos & Qt.AlignRight) {
				v_pos1 = first_pos ^ Qt.AlignRight;
				v_pos2 = second_pos ^ Qt.AlignRight;
				if (v_pos1 === v_pos2) {
					second_control.anchors.right = _control.right;
					first_control.anchors.right = second_control.left;
				}
			}
			else if (first_pos & Qt.AlignHCenter) {
				v_pos1 = first_pos ^ Qt.AlignHCenter;
				v_pos2 = second_pos ^ Qt.AlignHCenter;
				if (v_pos1 === v_pos2) {
					first_control.anchors.horizontalCenterOffset = -first_control.width;
					second_control.anchors.left = first_control.right;
				}
			}
		}
	}

	function anchorLabel(): void {
		label.anchors.top = _top_control ? _top_control.bottom : top;
		label.anchors.bottom = _bottom_control ? _bottom_control.top : bottom;
		label.anchors.left = _left_control ? _left_control.right : left;
		label.anchors.right = _right_control ? _right_control.left : right;
		height = Math.max(label.preferredHeight()) + (_top_control ? _top_control.height : 0) +
											(_bottom_control ? _bottom_control.height : 0) + 10
	}

	function mouseClicked(): void {
		if (_control.boxType === TPRadioButtonOrCheckBox.TP_CHECKBOX)
			_control.checked = !_control.checked;
		else if (_control.boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX) {
			if (_control.checked)
				return;
			if (!_control.buttonGroup)
				_control.checked = true;
			else
				_control.buttonGroup.setChecked(_control, true);
		}
		_control.clicked();
	}
}
