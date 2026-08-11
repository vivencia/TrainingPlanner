import QtQuick

import TpQml.Dialogs

Item {
	id: _control

//public:
	required property TPPopup parentPopup

//protected:
	property list<Item> items
	property int preferredHeight: 0

//private:
	property int _row: 0
	property list<int> _row_width: [0]

	Connections {
		target: _control.parentPopup
		function onPopupSizeChanged(w_ratio: real, h_ratio: real): void {
			for (let i = 0; i < _control.items.length; ++i)
				_control.items[i].width *= w_ratio;
		}
	}

//public:
	function addItem(item: Item, index: int, total_items: int): void {
		items.push(item);
		if (index === total_items - 1 && _row_width[_row] === 0) {
			item.anchors.horizontalCenter = horizontalCenter;
			if (index === 0)
				item.anchors.verticalCenter = verticalCenter;
			else
				item.anchors.top = items[index-1].bottom;
			preferredHeight += item.height;
			return;
		}
		if (item.width >= width * 0.8) { //too big to shrink
			if (index > 0)
				item.anchors.top = items[index-1].bottom;
			else
				item.anchors.top = top;
			item.anchors.horizontalCenter = horizontalCenter;
			_row_width[_row] = item.width;
		} else {
			if (item.width + _row_width[_row] <= width * 0.95) { //this item fits on the current row
				if (_row_width[_row] === 0) { //current row is empty
					item.anchors.left = left;
					if (index > 0)
						item.anchors.top = items[index-1].bottom;
					else
						item.anchors.top = top;
				} else { //current row has some item(s)
					item.anchors.left = items[index-1].right;
					item.anchors.verticalCenter = items[index-1].verticalCenter;
				}
				_row_width[_row] += item.width;
			} else { //resize one or more items until they fit on row. There are, at least, two items now
				let row_width = 0;
				const prev_widget_width = items[index-1].width;
				let shrink_prev = false;
				do { //resize either of the items at a time
					row_width = _row_width[_row];
					if (!shrink_prev) {
						if (item.width >= width * 0.5) //big, but shrinkable
							item.width *= 0.9; //shrink 10%
						shrink_prev = true;
					} else { //shrink previous item
						row_width -= prev_widget_width;
						items[index-1].width *= 0.9; //shrink 10%
						row_width += items[index-1].width;
						shrink_prev = false;
					}
					row_width += item.width;
				} while (row_width > width * 0.95);
				_row_width[_row] = Math.ceil(row_width);
				item.anchors.left = items[index-1].right;
				item.anchors.verticalCenter = items[index-1].verticalCenter;
			}
		}
		if (_row_width[_row] >= width * 0.8) {
			_row_width.push(0);
			++_row;
			if (index < total_items)
				preferredHeight += item.height;
		}
		item.anchors.margins = 5;
	} //function addItem

	function clearItems(): void {
		for (let i = items.length - 1; i >= 0; --i) {
			items[i].destroy();
			items.pop();
		}
		items = 0;
		preferredHeight = 0;
		_row_width = 0;
		_row = 0;
	}
} //TPLayout
