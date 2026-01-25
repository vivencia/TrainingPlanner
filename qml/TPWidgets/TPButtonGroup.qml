import QtQuick

QtObject {
	property int selectedOption: -1

	property list<Item> _buttons: []
	property int _nbuttons: 0

	function addButton(button: TPRadioButtonOrCheckBox): int {
		_nbuttons++;
		_buttons.push(button);
		return _nbuttons;
	}

	function removeButton(button: TPRadioButtonOrCheckBox): void {
		let new_buttons = [];
		let found = false;
		for (let i = 0; i < _buttons.length; ++i) {
			if (_buttons[i] !== button)
				new_buttons.push(button);
			else
				found = true;
		}
		if (found) {
			_buttons = 0;
			_buttons = new_buttons;
			_nbuttons--;
		}
	}

	function setChecked(button: TPRadioButtonOrCheckBox, checked: bool) : void {
		for (let i = 0; i < _buttons.length; ++i) {
			if (_buttons[i] === button) {
				_buttons[i].checked = true;
				selectedOption = Math.abs(_buttons.length - i - 1);
			}
			else
				_buttons[i].checked = false;
		}	
	}
}
