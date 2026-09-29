import QtQuick

import TpQml
import TpQml.Widgets

Item {
	id: _control

//private:
	property int _row: 0
	property list<int> _row_width: [0]

//public:
	signal execAction(action_index: int, data: variant)

	function createItem(type: int, label: string, index: int): Item {
		let _component;
		let _item;
		switch (type) {
		case TPMessage.AT_BUTTON:
			_component = Qt.createComponent("TpQml.Widgets", TPButton);
			_item = _component.createObject(_control, { text: label });
			_item.buttonClicked.connect(function(btn_id) { execAction(index, btn_id); });
			break;
		case TPMessage.AT_CHECKBOX:
			_component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox);
			_item = _component.createObject(_control, { text: label, boxType: TPRadioButtonOrCheckBox.TP_CHECKBOX});
			_item.checked.connect(function(check) { execAction(index, check); });
			break;
		case TPMessage.AT_RADIO:
			_component = Qt.createComponent("TpQml.Widgets", TPRadioButtonOrCheckBox);
			_item = _component.createObject(_control, { text: label, boxType: TPRadioButtonOrCheckBox.TP_RADIOBOX});
			_item.checked.connect(function(check) { execAction(index, check); });
			break;
		case TPMessage.AT_NONE:
			_item = null;
			break;
		}
		return _item;
	}

	function placeItem(item: Item, prev_item: Item, index: int, total_items: int): void {
		if (index === total_items - 1 && _row_width[_row] === 0) {
			item.anchors.horizontalCenter = horizontalCenter;
			if (index === 0)
				item.anchors.verticalCenter = verticalCenter;
			else
				item.anchors.top = prev_item.bottom;
			height += item.height;
			return;
		}
		if (item.width >= width * 0.8) { //too big to shrink
			if (index > 0)
				item.anchors.top = prev_item.bottom;
			else
				item.anchors.top = top;
			item.anchors.horizontalCenter = horizontalCenter;
			_row_width[_row] = item.width;
		} else {
			if (item.width + _row_width[_row] <= width * 0.95) { //this item fits on the current row
				if (_row_width[_row] === 0) { //current row is empty
					item.anchors.left = left;
					if (index > 0)
						item.anchors.top = prev_item.bottom;
					else
						item.anchors.top = top;
				} else { //current row has some item(s)
					item.anchors.left = prev_item.right;
					item.anchors.verticalCenter = prev_item.verticalCenter;
				}
				_row_width[_row] += item.width;
			} else { //resize one or more items until they fit on row. There are, at least, two items now
				let row_width = 0;
				const prev_widget_width = prev_item.width;
				let shrink_prev = false;
				do { //resize either of the items at a time
					row_width = _row_width[_row];
					if (!shrink_prev) {
						if (item.width >= width * 0.5) //big, but shrinkable
							item.width *= 0.9; //shrink 10%
						shrink_prev = true;
					} else { //shrink previous item
						row_width -= prev_widget_width;
						prev_item.width *= 0.9; //shrink 10%
						row_width += prev_item.width;
						shrink_prev = false;
					}
					row_width += item.width;
				} while (row_width > width * 0.95);
				_row_width[_row] = Math.ceil(row_width);
				item.anchors.left = prev_item.right;
				item.anchors.verticalCenter = prev_item.verticalCenter;
			}
		}
		if (_row_width[_row] >= width * 0.8) {
			_row_width.push(0);
			++_row;
			if (index < total_items)
				height += item.height;
		}
		item.anchors.margins = 5;
	} //function addItem

	function reLayoutLastRow(): void {
		//TODO
	}
} //TPLayout
