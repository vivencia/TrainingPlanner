import QtQuick

QtObject {
	property list<Item> buttons: []
	property int n_buttons: 0
	signal buttonChecked(btn_idx: int, checked: bool);

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
		let btn_idx = -1;
		for (let i = 0; i < buttons.length; ++i) {
			if (buttons[i] === button) {
				btn_idx = i;
				buttons[i].checked = true;
			}
			else
				buttons[i].checked = !checked;
		}
		if (btn_idx >= 0)
			buttonChecked(btn_idx, checked);
	}

	function anyButtonChecked(): bool {
		for (let i = 0; i < buttons.length; ++i) {
			if (buttons[i].checked)
				return true;
		}
		return false;
	}
}
