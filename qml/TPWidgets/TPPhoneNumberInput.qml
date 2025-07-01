import QtQuick
import QtQuick.Controls

TPTextInput {
    inputMethodHints: Qt.ImhDigitsOnly
    validator: RegularExpressionValidator { regularExpression: /\+?[0-9]{0,2} ?(\([0-9]{0,2}\))? ?[0-9]{0,5}-?[0-9]{0,4}/ }
    placeholderText: "+55 (XX) XXXXX-XXXX"
    maximumLength: 19
    ToolTip.text: qsTr("Invalid phone number")

    property bool phoneNumberOK: false

    onTextEdited: {
        text = formatPhoneNumber(text);
        if (text.length === 19) {
            ToolTip.visible = false;
            phoneNumberOK = true;
        }
        else {
            ToolTip.visible = true;
            phoneNumberOK = false;
        }
    }

    function formatPhoneNumber(digits: string) : string {
        // Remove all non-digits
        digits = digits.replace(/\D/g, '');

        // Format based on length
        let formatted = "";
        if (digits.length === 0)
            return ""; // Empty input shows nothing
        else if (digits.length <= 2)
            formatted = "+" + digits;
        else if (digits.length <= 4)
            formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2) + ")";
        else if (digits.length <= 9)
            formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2, 4) + ") " + digits.substring(4);
        else
            formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2, 4) + ") " + digits.substring(4, 9) + "-" + digits.substring(9);
        return formatted;
    }
}
