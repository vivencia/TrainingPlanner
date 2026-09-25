import QtQuick

QtObject {

//public:
	property int selectedOption: -1

//private:
	property list<TPRadioButtonOrCheckBox> _radios: []
	property int _nradios: 0

	function addRadio(button: TPRadioButtonOrCheckBox): int {
		_nradios++;
		_radios.push(button);
		return _nradios;
	}

	function removeRadio(button: TPRadioButtonOrCheckBox): void {
		let new_radios = [];
		let found = false;
		for (let i = 0; i < _radios.length; ++i) {
			if (_radios[i] !== button)
				new_radios.push(button);
			else
				found = true;
		}
		if (found) {
			_radios = 0;
			_radios = new_radios;
			_nradios--;
		}
	}

	function setChecked(button: TPRadioButtonOrCheckBox, checked: bool) : void {
		for (let i = 0; i < _radios.length; ++i) {
			if (_radios[i] === button) {
				_radios[i].checked(true);
				_radios[i].isChecked = true;
				selectedOption = Math.abs(_radios.length - i - 1);
			} else {
				_radios[i].checked(false);
				_radios[i].isChecked = false;
			}
		}	
	}
}
