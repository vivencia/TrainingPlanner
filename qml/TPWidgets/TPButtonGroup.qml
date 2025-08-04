import QtQuick

QtObject {
	property list<Item> buttons: []
	property int n_buttons: 0

	function addButton(button: TPRadioButtonOrCheckBox): int {
		n_buttons++;
		buttons.push(button);
		return n_buttons;
	}

	function removeButton(button: TPRadioButtonOrCheckBox): void {
		let new_buttons = [];
		let found = false;
		for (let i = 0; i < buttons.length; ++i) {
			if (buttons[i] !== button)
				new_buttons.push(button);
			else
				found = true;
		}
		if (found) {
			buttons = 0;
			buttons = new_buttons;
			n_buttons--;
		}
	}

	function setChecked(button: TPRadioButtonOrCheckBox, checked: bool) : void {
		for (let i = 0; i < buttons.length; ++i)
			buttons[i].checked = buttons[i] === button ? checked : !checked;
	}
}
