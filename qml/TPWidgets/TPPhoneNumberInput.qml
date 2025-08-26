import QtQuick
import QtQuick.Controls

TPTextInput {
    inputMethodHints: Qt.ImhDigitsOnly
	validator: RegularExpressionValidator {
					regularExpression: countryPrefix ? /\+?[0-9]{0,3}/ : /\+?(\([0-9]{0,2}\))? ?[0-9]{0,5}-?[0-9]{0,4}/
				}
	placeholderText: "(XX) XXXXX-XXXX"
	maximumLength: countryPrefix ? 5 : 15
    ToolTip.text: qsTr("Invalid phone number")

    property bool phoneNumberOK: false
	property bool countryPrefix: false

    onTextEdited: {
		if (!textRemovedKeyPressed)
			text = formatPhoneNumber(text);
		let ok = true;
		ok = !countryPrefix && text.length === 15;
		ok |= countryPrefix && text.length >= 3;
		ToolTip.visible = !ok;
		phoneNumberOK = ok;
    }

    function formatPhoneNumber(digits: string) : string {
        // Remove all non-digits
        digits = digits.replace(/\D/g, '');

        // Format based on length
        let formatted = "";
        if (digits.length === 0)
            return ""; // Empty input shows nothing
		if (!countryPrefix) {
			if (digits.length <= 2)
				formatted = "(" + digits.substring(0, 2) + ")";
			else if (digits.length <= 6)
				formatted = "(" + digits.substring(0, 2) + ") " + digits.substring(2, 7);
			else
				formatted = "(" + digits.substring(0, 2) + ") " + digits.substring(2, 7) + "-" + digits.substring(7);
		}
		else
			formatted = "+" + digits.substring(0);
        return formatted;
    }
}
