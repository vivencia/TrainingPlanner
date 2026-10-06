import QtQuick

import TpQml

TPBackRec {
	id: _control
	backColor: "transparent"
	height: preferredHeight
	width: preferredWidth

//public:
	property alias image: img.source
	property alias text: label.text
	property alias elideMode: label.elide
	property alias helpString: label.helpString
	property alias font: label.font //When overriding any font property, all the font properties are reset; so other properties must be reapplied
	property int imageHeight: AppSettings.itemSmallHeight
	property int imageWidth: image.length > 0 ? imageHeight : 0
	property int indicatorPos: Qt.AlignLeft|Qt.AlignVCenter
	property int imagePos: Qt.AlignLeft|Qt.AlignVCenter
	property bool imageAfterIndicator: true
	property bool isChecked: false
	property bool multiLine: false
	property bool actionable: enabled
	property bool useDropShadow: false
	property bool enableCheckOutsideIndicator: true
	property bool enableClicks: false
	property int boxType: TPRadioButtonOrCheckBox.TP_RADIOBOX
	property TPButtonGroup buttonGroup: null
	readonly property int preferredWidth: label.preferredWidth + (_left_control ? _left_control.width : 0)
																		+ (_right_control ? _right_control.width : 0) + 10
	readonly property int preferredHeight: label.preferredHeight()

	enum BoxType { TP_RADIOBOX, TP_CHECKBOX, TP_NONEBOX }
	//only if enableClicks is true. If enableCheckOutsideIndicator is true both clicked() and checked() will be emitted
	signal clicked()
	signal checked(check: bool)

//private:
	enum BoxControls { INDICATOR_ONLY, INDICATOR_PLUS_IMAGE, IMAGE_ONLY, IMAGE_PLUS_INDICATOR, TEXT_ONLY }
	property Item _top_control: null
	property Item _bottom_control: null
	property Item _left_control: null
	property Item _right_control: null
	property bool _component_completed: false

	property int _control_subtype: getControlSubType();

	onBoxTypeChanged: {
		if (_component_completed) {
			_control_subtype = getControlSubType();
			setupLayout();
		}
	}

	onWidthChanged: {
		if (_component_completed && width >= AppSettings.itemSmallHeight)
			setupLayout();
	}

	TPLabel {
		id: label
		singleLine: !_control.multiLine
		enabled: parent.enabled
		padding: 5

		MouseArea {
			enabled: _control.enabled && _control.enableCheckOutsideIndicator || _control.enableClicks
			anchors.fill: parent
			onClicked: _control.enableCheckOutsideIndicator ? _control.checkFunction() : _control.clicked();
		}
	}

	Rectangle {
		id: indicator
		width: _control.boxType !== TPRadioButtonOrCheckBox.TP_NONEBOX ? AppSettings.itemSmallHeight : 0
		height: width
		radius: _control.boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX ? width / 2 : 4
		color: "transparent"
		border.color: _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
		visible: _control.boxType !== TPRadioButtonOrCheckBox.TP_NONEBOX
		anchors.margins: 2

		Rectangle {
			id: recChecked
			width: indicator.height * 0.5
			height: width
			radius: _control.boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX ? width / 2 : indicator.radius / 2
			x: (indicator.width - width) / 2
			y: x
			border.color: _control.enabled ? AppSettings.fontColor : AppSettings.disabledFontColor
			visible: _control.isChecked
		}

		MouseArea {
			enabled: _control.enabled
			anchors.fill: parent
			onClicked: _control.checkFunction();
		}
	}

	TPImage {
		id: img
		height: _control.imageHeight
		width: _control.imageWidth
		dropShadow: _control.useDropShadow
		visible: source.length > 0
		anchors.margins: 2
		MouseArea {
			enabled: _control.enabled && _control.enableCheckOutsideIndicator || _control.enableClicks
			anchors.fill: parent
			onClicked: _control.enableCheckOutsideIndicator ? _control.checkFunction() : _control.clicked();
		}
	}

	Component.onCompleted: {
		if (boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX && buttonGroup) {
			buttonGroup.addRadio(this);
			if (isChecked)
				buttonGroup.setChecked(this, true);
		}
		_component_completed = true;
		if (width >= AppSettings.itemSmallHeight)
			setupLayout();
	}

	Component.onDestruction: {
		if (boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX && buttonGroup)
			buttonGroup.removeRadio(this);
	}

	function getControlSubType(): int
	{
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
		if (_control.text.length > 0)
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
			h_pos1 = first_pos ^ Qt.AlignTop;
			h_pos2 = second_pos ^ Qt.AlignTop;
			if (h_pos1 === h_pos2) {
				if (label.contentWidth >= _control.width - first_control.width - second_control.width) {
					first_control.anchors.top = _control.top;
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
		if (_right_control)
			label.questionMarkOffset = _control.width - label.width;
	}

	function checkFunction(): void {
		if (_control.boxType === TPRadioButtonOrCheckBox.TP_CHECKBOX) {
			_control.isChecked = !_control.isChecked;
			_control.checked(_control.isChecked);
		} else if (_control.boxType === TPRadioButtonOrCheckBox.TP_RADIOBOX) {
			if (!_control.isChecked) {
				if (!_control.buttonGroup) {
					_control.isChecked = true;
					_control.checked(true);
				} else {
					_control.buttonGroup.setChecked(_control, true);
				}
			}
		}
	}
}
